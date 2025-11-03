// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameState.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "AiTestMonster.h"
#include "ObjectPoolManager.h"

ATeam13_GameState::ATeam13_GameState()
{
    EnemyToSpawn = 5;

    EnemySpawned = 0;

    EnemyAlive = 0;
}

void ATeam13_GameState::BeginPlay()
{
	Super::BeginPlay();

    EnemySpawned = 0;
    EnemyAlive = 0;
    UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();
    if (PoolManager)
    {
        PoolManager->InitializePool(AAiTestMonster::StaticClass(), 50);
    }

    TArray<AActor*> FoundVolumes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnEnemy::StaticClass(), FoundVolumes);

    if (FoundVolumes.Num() > 0)
    {
        ASpawnEnemy* SpawnVolume = Cast<ASpawnEnemy>(FoundVolumes[0]);
        if (SpawnVolume)
        {
            for (int32 i = 0; i < EnemyToSpawn; i++)
            {
                AActor* SpawnedActor = SpawnVolume->SpawnRandomEnemy();
                if (SpawnedActor)
                {
                    EnemySpawned++;
                    EnemyAlive++;
                }
            }
        }
    }
}