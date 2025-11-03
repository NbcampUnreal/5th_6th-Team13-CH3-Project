#include "SpawnEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "ObjectPoolManager.h"
#include "Kismet/GameplayStatics.h"

ASpawnEnemy::ASpawnEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SpawningSphere"));
	SpawningSphere->SetupAttachment(Scene);

	EnemyDataTable = nullptr;
}

FEnemySpawnRow* ASpawnEnemy::GetRandomEnemy() const
{
	if (!EnemyDataTable) return nullptr;

	TArray<FEnemySpawnRow*> AllRows;
	static const FString ContextString(TEXT("ItemSpawnContext"));
	EnemyDataTable->GetAllRows(ContextString, AllRows);

	if (AllRows.IsEmpty()) return nullptr;

	float TotalChance = 0.0f;
	for (const FEnemySpawnRow* Row : AllRows)
	{
		if (Row)
		{
			TotalChance += Row->Spawnchance;
		}
	}

	const float RandValue = FMath::FRandRange(0.0f, TotalChance);
	float AccumulateChance = 0.0f;

	for (FEnemySpawnRow* Row : AllRows)
	{
		AccumulateChance += Row->Spawnchance;
		if (RandValue <= AccumulateChance)
		{
			return Row;
		}
	}

	return nullptr;
}

FVector ASpawnEnemy::GetRandomPointInVolume_Sphere() const
{
	FVector Origin = SpawningSphere->GetComponentLocation();
	float Radius = SpawningSphere->GetScaledSphereRadius();

	FVector2D RandomPoint = FMath::RandPointInCircle(Radius);
	return FVector(Origin.X + RandomPoint.X, Origin.Y + RandomPoint.Y, Origin.Z);
}

// Object Pool을 사용한 Spawn
AActor* ASpawnEnemy::SpawnEnemy(TSubclassOf<AActor> EnemyClass)
{
	if (!EnemyClass) return nullptr;

	// World Subsystem에서 PoolManager 가져오기
	UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();
	if (!PoolManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolManager not found!"));
		return nullptr;
	}

	FVector SpawnLocation = GetRandomPointInVolume_Sphere();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	AActor* SpawnedActor = nullptr;
	PoolManager->SpawnFromPool(EnemyClass, SpawnLocation, SpawnRotation, SpawnedActor);

	if (SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned from pool: %s"), *SpawnedActor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn FAILED for %s"), *EnemyClass->GetName());
	}

	return SpawnedActor;
}

// 랜덤 적 풀에서 Spawn
AActor* ASpawnEnemy::SpawnRandomEnemy()
{
	if (FEnemySpawnRow* SelectedRow = GetRandomEnemy())
	{
		if (TSubclassOf<AActor> EnemyClass = SelectedRow->EnemyClass)
		{
			return SpawnEnemy(EnemyClass);
		}
	}
	return nullptr;
}
