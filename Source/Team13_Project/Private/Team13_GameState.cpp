// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameState.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"

ATeam13_GameState::ATeam13_GameState()
{

}

void ATeam13_GameState::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnEnemy::StaticClass(), FoundVolumes);

	int32 BaseSpawnCount = 40;
	float SpawnIncreaseRate = 0.2f;

	EnemyToSpawn = BaseSpawnCount + (BaseSpawnCount * SpawnIncreaseRate);

	for (int32 i = 0; i < BaseSpawnCount; i++)
	{
		if (FoundVolumes.Num() > 0)
		{
			ASpawnEnemy* SpawnEnemy = Cast<ASpawnEnemy>(FoundVolumes[0]);
			if (SpawnEnemy)
			{
				AActor* SpawnedActor = SpawnEnemy->SpawnRandomEnemy();
			}
		}
	}
}
