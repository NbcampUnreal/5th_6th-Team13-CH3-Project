// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameMode.h"

#include "AiTestMonster.h"
#include "Team13_GameState.h"
#include "Team13_GameInstance.h"
#include "HERO_Character.h"
#include "ObjectPoolManager.h"
#include "Team13_PlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeam13_GameMode::ATeam13_GameMode()
{
	GameStateClass = ATeam13_GameState::StaticClass();
	PlayerControllerClass = ATeam13_PlayerController::StaticClass();

	poolinitalized_v = 50;
	
}

void ATeam13_GameMode::MonsterKilled(ABaseMonsterCharacter* KilledMonster, int32 exp)
{
	if (KilledMonster)
	{
		UTeam13_GameInstance* GameInstance = GetGameInstance<UTeam13_GameInstance>();
		if (GameInstance)
		{
			GameInstance->AddToKill();
			GameInstance->AddScore(exp);
		}
		AHERO_Character* player = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (player)
		{
			//플레이어 경험치 증가 함수
			player->AddExp(exp);
			UE_LOG(LogTemp, Display, TEXT("[Game Mode] Player exp: %f"),player->EXP);
		}
	}
	
}

void ATeam13_GameMode::PlayerLevelUp()
{
	AHERO_Character* player = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!player)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode]PlayerLevelUp: Player nullptr"));
		return;
	}
	UTeam13_GameInstance* GameInstance = GetGameInstance<UTeam13_GameInstance>();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode]PlayerLevelUp: GameInstance nullptr"));
		return;
	}

	//game complete check
	if (player->GetLevel() >= GameInstance->MaxLevels[GameInstance->CurrentStageIndex])
	{
		ATeam13_GameState* gamestate = GetGameState<ATeam13_GameState>();
		if (gamestate)
		{
			if (IsCompleteGame())
			{
				gamestate->OnGameOver();
			}
			else
			{
				gamestate->EndStage();
			}

		}
	}
}

bool ATeam13_GameMode::IsCompleteGame()
{
	AHERO_Character* player = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!player)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode]PlayerLevelUp: Player nullptr"));
		return false;
	}
	UTeam13_GameInstance* GameInstance = GetGameInstance<UTeam13_GameInstance>();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode]PlayerLevelUp: GameInstance nullptr"));
		return false;
	}

	//clear conditions
	/*if (player->GetLevel() >= GameInstance->MaxLevels[GameInstance->CurrentStageIndex]
	 *&& GameInstance->CurrentStageIndex >= GameInstance->MaxStageIndex)
	{
		return true;
	}*/
	return false;
}

void ATeam13_GameMode::CompleteGame()
{
	if (IsCompleteGame())
	{
		//clear call, ending scene
	}
}

void ATeam13_GameMode::StartGameStage()
{
	UE_LOG(LogTemp, Display, TEXT("[Game Mode] Starting Game Stage"));
	
	UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();
	FString CurrentMapName = GetWorld()->GetMapName();
	if (!CurrentMapName.Contains("StartMenu"))
	{
		if (PoolManager)
		{
			UE_LOG(LogTemp, Display, TEXT("[Game Mode] Starting initialize pool"));
			PoolManager->InitializePool(AAiTestMonster::StaticClass(), poolinitalized_v);
		}
	}
}




