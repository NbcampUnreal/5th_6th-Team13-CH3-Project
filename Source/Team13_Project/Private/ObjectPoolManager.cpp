#include "ObjectPoolManager.h"
#include "BaseMonsterCharacter.h"
#include "AiTestMonster.h"
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
//void UObjectPoolManager::ReturnToPool(AActor* Poolable)
//{
//    if (!Poolable) return;
//
//    UClass* ActorClass = Poolable->GetClass();
//
//    if (ActorClass->ImplementsInterface(UPoolable::StaticClass()))
//    {
//        IPoolable::Execute_OnReturnToPool(Poolable);
//        Poolable->SetActorHiddenInGame(true);
//        Poolable->SetActorEnableCollision(false);
//        FPoolArray& ObjectPool = ObjectPools.FindOrAdd(ActorClass);
//        ObjectPool.Add(Poolable);
//    }
//    else
//    {
//        Poolable->Destroy();
//    }
//}
//
//void UObjectPoolManager::InitializePool(TSubclassOf<AActor> PoolClass, int32 MaxSize)
//{
//    if (!PoolClass) return;
//    UClass* KeyClass = GetPoolKey(PoolClass.Get());
//
//    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(KeyClass);
//
//    if (ObjectPool.Num() >= MaxSize)
//        return;
//
//    FActorSpawnParameters SpawnParams;
//    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
//    for (int32 i = 0; i < MaxSize; ++i)
//    {
//        AActor* NewActor = GetWorld()->SpawnActor<AActor>(PoolClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
//        if (!NewActor) continue;
//
//        if (NewActor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
//        {
//            IPoolable::Execute_OnReturnToPool(NewActor);
//            NewActor->SetActorHiddenInGame(true);
//            NewActor->SetActorEnableCollision(false);
//        }
//
//        ObjectPool.Add(NewActor);
//    }
//
//    UE_LOG(LogTemp, Log, TEXT("[ObjectPoolManager] Initialized %s pool (%d actors)"),
//        *KeyClass->GetName(), ObjectPool.Num());
//}
//
//AActor* UObjectPoolManager::GetActorFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation)
//{
//    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(PoolClass);
//    if (!ObjectPool.IsEmpty())
//    {
//        AActor* Actor = ObjectPool.Pop();
//        if (Actor) {
//            Actor->SetActorLocationAndRotation(Location, Rotation);
//            Actor->SetActorHiddenInGame(false);
//            IPoolable::Execute_OnSpawnFromPool(Actor);
//            return Actor;
//        }
//    }
//
//    UE_LOG(LogTemp, Warning, TEXT("[ObjectPoolManager] Pool for %s is empty!"), *PoolClass->GetName());
//    return nullptr;
//}

void UObjectPoolManager::InitializePool(TSubclassOf<AActor> PoolClass, int32 MaxSize)
{
    if (!PoolClass) return;
    UClass* KeyClass = GetPoolKey(PoolClass.Get());

    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(KeyClass);

    if (ObjectPool.Num() >= MaxSize)
        return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < MaxSize; ++i)
    {
        AActor* NewActor = GetWorld()->SpawnActor<AActor>(PoolClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (!NewActor) continue;

        if (NewActor->GetClass()->ImplementsInterface(UPoolable::StaticClass()))
        {
            IPoolable::Execute_OnReturnToPool(NewActor);
            NewActor->SetActorHiddenInGame(true);
            NewActor->SetActorEnableCollision(false);
            NewActor->RegisterAllComponents();
        }

        ObjectPool.Add(NewActor);
    }

    UE_LOG(LogTemp, Log, TEXT("[ObjectPoolManager] Initialized %s pool (%d actors)"),
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
