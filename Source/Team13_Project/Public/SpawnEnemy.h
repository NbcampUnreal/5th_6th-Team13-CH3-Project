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

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AActor* SpawnRandomEnemy();
protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	USceneComponent* Scene;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	//UBoxComponent* SpawningBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	USphereComponent* SpawningSphere;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* EnemyDataTable;

	FEnemySpawnRow* GetRandomEnemy() const;
	AActor* SpawnEnemy(TSubclassOf<AActor> EnemyClass);
	//FVector GetRandomPointInVolume() const;
	FVector GetRandomPointInVolume_Sphere() const;


public:
	// Called every frame
	//virtual void Tick(float DeltaTime) override;



};