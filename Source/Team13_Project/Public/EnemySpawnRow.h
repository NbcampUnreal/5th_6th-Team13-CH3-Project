// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemySpawnRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)

struct FEnemySpawnRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EnemyLevel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> EnemyClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Spawnchance;
};
