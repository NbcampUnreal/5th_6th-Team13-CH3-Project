// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

//ASpawnEnemy -> 
ASpawnEnemy::ASpawnEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	//SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	//SpawningBox->SetupAttachment(Scene);

	//Circle Spawning Range
	SpawningSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SpawningSphere"));
	SpawningSphere->SetupAttachment(Scene);

	EnemyDataTable = nullptr;
}

//// Called when the game starts or when spawned
//void ASpawnEnemy::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}

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

AActor* ASpawnEnemy::SpawnEnemy(TSubclassOf<AActor> EnemyClass)
{
	if (!EnemyClass) return nullptr;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		EnemyClass,
		GetRandomPointInVolume_Sphere(),
		FRotator::ZeroRotator
	);
	if (SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned: %s"), *SpawnedActor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn FAILED for %s"), *EnemyClass->GetName());
	}
	return SpawnedActor;
}
/*
FVector ASpawnEnemy::GetRandomPointInVolume() const
{
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	//중심좌표
	FVector BoxOrigin = SpawningBox->GetComponentLocation();

	//랜덤한 좌표값을 들고옴
	return BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
	);
}
*/

FVector ASpawnEnemy::GetRandomPointInVolume_Sphere() const
{
	FVector Origin = SpawningSphere->GetComponentLocation();
	float Radius = SpawningSphere->GetScaledSphereRadius();

	// XY 평면의 원 내부에서 랜덤 점 생성
	FVector2D RandomPoint = FMath::RandPointInCircle(Radius);

	// Z는 고정 (바닥면)
	return FVector(Origin.X + RandomPoint.X, Origin.Y + RandomPoint.Y, Origin.Z);
}


AActor* ASpawnEnemy::SpawnRandomEnemy()
{
	if (FEnemySpawnRow* SelectedRow = GetRandomEnemy())
	{
		//TSubclass
		if (UClass* ActualClass = SelectedRow->EnemyClass.Get())
		{
			return SpawnEnemy(ActualClass);
		}
	}
	return nullptr;
}