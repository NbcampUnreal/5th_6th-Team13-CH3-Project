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

	EnemyToSpawn = 5;

	EnemySpawned = 0;

	EnemyAlive = 0;

	bCanSpawn = true;
}

void ASpawnEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Overlap 이벤트 바인딩
	SpawningSphere->OnComponentBeginOverlap.AddDynamic(this, &ASpawnEnemy::OnOverlapBegin);
}

FEnemySpawnRow* ASpawnEnemy::GetRandomEnemy()
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

//Overlap할 경우 적 생성
void ASpawnEnemy::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bCanSpawn) return; // 10초 제한 중에는 작동하지 못하게 함

	bCanSpawn = false; // 한 번 스폰 후 비활성화 -> 이유 : Overlap이 계속 반복됨

	// 여기서 바로 적 생성
	for (int32 i = 0; i < EnemyToSpawn; i++)
	{
		AActor* SpawnedActor = SpawnRandomEnemy();
		if (SpawnedActor)
		{
			EnemySpawned++;
			EnemyAlive++;
		}
	}

	//10초 후 다시 활성화
	GetWorld()->GetTimerManager().SetTimer(
		SpawnCooldownTimerHandle,
		this,
		&ASpawnEnemy::ResetSpawnState,
		10.0f, 
		false  
	);
}

void ASpawnEnemy::ResetSpawnState()
{
	bCanSpawn = true;
}

//Object Pool을 사용한 Spawn
AActor* ASpawnEnemy::SpawnEnemy(TSubclassOf<AActor> EnemyClass)
{
	if (!EnemyClass) return nullptr;

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
