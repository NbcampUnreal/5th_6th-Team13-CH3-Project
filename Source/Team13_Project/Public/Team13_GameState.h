// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Team13_GameState.generated.h"

/**
 * 
 */
UCLASS()
class TEAM13_PROJECT_API ATeam13_GameState : public AGameState
{
	GENERATED_BODY()
public : 
	ATeam13_GameState();

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	int32 EnemyToSpawn; // 총 스폰 예정 수

	UPROPERTY(BlueprintReadOnly)
	int32 EnemySpawned; // 지금까지 스폰된 적 수

	UPROPERTY(BlueprintReadOnly)
	int32 EnemyAlive; // 현재 살아있는 적 수
};
