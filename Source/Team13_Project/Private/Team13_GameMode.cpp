// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameMode.h"
#include "Team13_GameState.h"
#include "Team13_GameInstance.h"
#include "HERO_Character.h"
#include "Team13_PlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeam13_GameMode::ATeam13_GameMode()
{
	GameStateClass = ATeam13_GameState::StaticClass();
	PlayerControllerClass = ATeam13_PlayerController::StaticClass();
}

void ATeam13_GameMode::MonsterKilled(ABaseMonsterCharacter* KilledMonster, int32 exp)
{
	if (KilledMonster)
	{
		UTeam13_GameInstance* GameInstance = GetGameInstance<UTeam13_GameInstance>();
		if (GameInstance)
		{
			GameInstance->AddToKill();
		}
		AHERO_Character* player = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (player)
		{
			//플레이어 경험치 증가 함수
			//player->addexp(exp);
		}
	}
}
