// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemySpawnRow.h"
#include "GameFramework/Actor.h"
#include "SpawnEnemy.generated.h"

class UBoxComponent;
class USphereComponent;

UCLASS()
class TEAM13_PROJECT_API ASpawnEnemy : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpawnEnemy();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AActor* SpawnRandomEnemy();
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	USceneComponent* Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	USphereComponent* SpawningSphere;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* EnemyDataTable;

	AActor* SpawnEnemy(TSubclassOf<AActor> EnemyClass);
	FVector GetRandomPointInVolume_Sphere() const;

public:
	// Called every frame
	UPROPERTY(BlueprintReadOnly)
	int32 EnemyToSpawn; // 총 스폰 예정 수

	UPROPERTY(BlueprintReadOnly)
	int32 EnemySpawned; // 지금까지 스폰된 적 수

	UPROPERTY(BlueprintReadOnly)
	int32 EnemyAlive; // 현재 살아있는 적 수

	UPROPERTY()
	bool bCanSpawn; // 스폰 가능 여부 체크

	FTimerHandle SpawnCooldownTimerHandle;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	FEnemySpawnRow* GetRandomEnemy();
	//다시 reset해서 적 생성하는 함수
	void ResetSpawnState();

};