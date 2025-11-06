#include "ObjectPoolManager.h"
#include "Poolable.h"
// Fill out your copyright notice in the Description page of Project Settings.

/*
#include "SpawnEnemy.h"
#include "AiTestMonster.h"
#include "ObjectPoolManager.h"

void UObjectPoolManager::InitializePool(TSubclassOf<AActor> ObjectClass, int32 PoolSize)
{
    if (!GetWorld() || EnemyPool.Num() > 0) return;  // 이미 풀 초기화됨

    SpawnableEnemyClass = ObjectClass;

    for (int32 i = 0; i < PoolSize; ++i)
    {
        AAiTestMonster* NewEnemy = GetWorld()->SpawnActor<AAiTestMonster>(ObjectClass);
        if (NewEnemy)
        {
            NewEnemy->SetActorHiddenInGame(true);
            NewEnemy->SetActorEnableCollision(false);
            EnemyPool.Add(NewEnemy);
        }
    }
}

AAiTestMonster* UObjectPoolManager::GetEnemyFromPool()
{
    for (AAiTestMonster* Enemy : EnemyPool)
    {
        if (Enemy && Enemy->IsHidden())
        {
            Enemy->SetActorHiddenInGame(false);
            Enemy->SetActorEnableCollision(true);
            return Enemy;
        }
    }
    return nullptr; // 풀에 없으면 nullptr 반환
}


void UObjectPoolManager::ReturnEnemyToPool(AAiTestMonster* Enemy)
{
    if (Enemy)
    {
        Enemy->SetActorHiddenInGame(true);
        Enemy->SetActorEnableCollision(false);
    }
}

*/

void UObjectPoolManager::SpawnFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation, AActor*& SpawnedActor)
{
    SpawnedActor = GetActorFromPool(PoolClass, Location, Rotation);
}

void UObjectPoolManager::ReturnToPool(AActor* Poolable)
{
    if (!Poolable) return;

    UClass* ActorClass = Poolable->GetClass();

    if (ActorClass->ImplementsInterface(UPoolable::StaticClass()))
    {
        IPoolable::Execute_OnReturnToPool(Poolable);
        Poolable->SetActorHiddenInGame(true);
        Poolable->SetActorEnableCollision(false);
        FPoolArray& ObjectPool = ObjectPools.FindOrAdd(ActorClass);
        ObjectPool.Add(Poolable);
    }
    else
    {
        Poolable->Destroy();
    }
}

void UObjectPoolManager::InitializePool(TSubclassOf<AActor> PoolClass, int32 MaxSize)
{
    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(PoolClass);
    for (int32 i = 0; i < MaxSize; ++i) {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* NewActor = GetWorld()->SpawnActor<AActor>(PoolClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (NewActor && PoolClass.Get()->ImplementsInterface(UPoolable::StaticClass())) {
            IPoolable::Execute_OnReturnToPool(NewActor);
            NewActor->SetActorHiddenInGame(true);
            NewActor->SetActorEnableCollision(false);
            ObjectPool.Add(NewActor);
        }
    }
}

AActor* UObjectPoolManager::GetActorFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation)
{
    FPoolArray& ObjectPool = ObjectPools.FindOrAdd(PoolClass);
    if (!ObjectPool.IsEmpty())
    {
        AActor* Actor = ObjectPool.Pop();
        if (Actor) {
            Actor->SetActorLocationAndRotation(Location, Rotation);
            Actor->SetActorHiddenInGame(false);
            IPoolable::Execute_OnSpawnFromPool(Actor);
            return Actor;
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* NewActor = GetWorld()->SpawnActor<AActor>(PoolClass, Location, Rotation, SpawnParams);
    if (NewActor && PoolClass.Get()->ImplementsInterface(UPoolable::StaticClass())) {
        IPoolable::Execute_OnSpawnFromPool(NewActor);
    }
    return NewActor;
}
