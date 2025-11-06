#include "ObjectPoolManager.h"
#include "BaseMonsterCharacter.h"
#include "AiTestMonster.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Poolable.h"

void UObjectPoolManager::SpawnFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation, AActor*& SpawnedActor)
{
    SpawnedActor = GetActorFromPool(PoolClass, Location, Rotation);
}

UClass* UObjectPoolManager::GetPoolKey(UClass* InputClass)
{
    if (!InputClass) return nullptr;

    // ABaseMonsterCharacter를 기준으로 모든 하위 클래스 묶기
    if (InputClass->IsChildOf(AAiTestMonster::StaticClass()))
    {
        return AAiTestMonster::StaticClass();
    }

    // 그 외 클래스들은 자기 자신 기준
    return InputClass;
}

void UObjectPoolManager::InitializePool(TSubclassOf<AActor> DefaultPoolClass, int32 MaxSize)
{
    //풀 클래스 유효성 체크
    if (!DefaultPoolClass) return;

    UClass* KeyClass = GetPoolKey(DefaultPoolClass.Get());
    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(KeyClass);

    if (ObjectPool.Num() >= MaxSize)
        return;

    // 현재 월드에서 SpawnEnemy 액터 찾기 (랜덤 적 클래스 뽑기용)
    TArray<AActor*> FoundSpawners;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnEnemy::StaticClass(), FoundSpawners);

    if (FoundSpawners.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ObjectPoolManager] No ASpawnEnemy found! Using default class only."));
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // MaxSize만큼 반복해서 랜덤 적 생성
    for (int32 i = 0; i < MaxSize; ++i)
    {
        TSubclassOf<AActor> SpawnClass = DefaultPoolClass;

        // 스포너가 있다면 랜덤 적 클래스 가져오기
        if (FoundSpawners.Num() > 0)
        {
            ASpawnEnemy* RandomSpawner = Cast<ASpawnEnemy>(FoundSpawners[FMath::RandRange(0, FoundSpawners.Num() - 1)]);
            if (RandomSpawner)
            {
                if (FEnemySpawnRow* EnemyRow = RandomSpawner->GetRandomEnemy())
                {
                    if (EnemyRow->EnemyClass)
                    {
                        SpawnClass = EnemyRow->EnemyClass;
                    }
                }
            }
        }

        if (!SpawnClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ObjectPoolManager] No valid class for enemy at index %d"), i);
            continue;
        }

        AActor* NewActor = GetWorld()->SpawnActor<AActor>(SpawnClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (!NewActor) continue;

        if (NewActor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
        {
            IPoolable::Execute_OnReturnToPool(NewActor);
            NewActor->SetActorHiddenInGame(true);
            NewActor->SetActorEnableCollision(false);
        }

        ObjectPool.Add(NewActor);
        UE_LOG(LogTemp, Log, TEXT("[ObjectPoolManager] Added %s to pool."), *NewActor->GetName());
    }

    UE_LOG(LogTemp, Log, TEXT("[ObjectPoolManager] Initialized %s pool (%d actors, random enemies)"),
        *KeyClass->GetName(), ObjectPool.Num());
}

AActor* UObjectPoolManager::GetActorFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation)
{
    if (!PoolClass) return nullptr;

    UClass* KeyClass = GetPoolKey(PoolClass.Get());
    FPoolArray* ObjectPool = ObjectPools.Find(KeyClass);
    if (!ObjectPool || ObjectPool->IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[ObjectPoolManager] Pool empty for %s!"), *KeyClass->GetName());
        return nullptr;
    }
    if (!ObjectPool->IsEmpty())
    {
        AActor* Actor = ObjectPool->Pop();
        if (Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Spawn Location: %s"), *Location.ToString());

            Actor->SetActorHiddenInGame(false);
            Actor->SetActorLocationAndRotation(Location, Rotation);
            Actor->SetActorEnableCollision(true);
            Actor->RegisterAllComponents();
            Actor->ReregisterAllComponents();
            if (Actor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
            {
                IPoolable::Execute_OnSpawnFromPool(Actor);
            }

            return Actor;
        }
    }


    UE_LOG(LogTemp, Warning, TEXT("[ObjectPoolManager] Pool for %s (key %s) is empty!"),
        *PoolClass->GetName(), *KeyClass->GetName());
    return nullptr;
}

void UObjectPoolManager::ReturnToPool(AActor* Poolable)
{
    if (!Poolable) return;

    UClass* KeyClass = GetPoolKey(Poolable->GetClass());
    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(KeyClass);

    IPoolable::Execute_OnReturnToPool(Poolable);
    Poolable->SetActorHiddenInGame(true);
    Poolable->SetActorEnableCollision(false);

    ObjectPool.Add(Poolable);
}
