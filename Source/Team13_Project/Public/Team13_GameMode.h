// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Team13_GameMode.generated.h"

/**
 * 
 */
class ABaseMonsterCharacter;
UCLASS()
class TEAM13_PROJECT_API ATeam13_GameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ATeam13_GameMode();
	void MonsterKilled(ABaseMonsterCharacter* KilledMonster);
};
