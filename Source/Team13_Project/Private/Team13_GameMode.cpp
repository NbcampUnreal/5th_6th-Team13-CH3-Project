// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameMode.h"
#include "Team13_GameState.h"
#include "Team13_PlayerController.h"
#include "ObjectPoolManager.h"
#include "Team13_GameInstance.h"
#include "AiTestMonster.h"

ATeam13_GameMode::ATeam13_GameMode()
{
	GameStateClass = ATeam13_GameState::StaticClass();
	PlayerControllerClass = ATeam13_PlayerController::StaticClass();
}

void ATeam13_GameMode::BeginPlay()
{
	Super::BeginPlay();

	// 월드가 완전히 준비된 시점에서 Subsystem 접근 가능
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
		if (Team13_GameInstance->CurrentStageIndex > 0 && Team13_GameInstance->CurrentStageIndex < 3)
		{
			UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();
			if (PoolManager)
			{
				PoolManager->InitializePool(AAiTestMonster::StaticClass(), 10);
				UE_LOG(LogTemp, Warning, TEXT("Object Pool initialized from GameMode"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to get ObjectPoolManager subsystem!"));
			}
		}
	}
}
