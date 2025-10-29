#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Team13_GameState.generated.h"

UCLASS()
class TEAM13_PROJECT_API ATeam13_GameState : public AGameState
{
	GENERATED_BODY()
public:
	ATeam13_GameState();

	virtual void BeginPlay() override;

	//int32 EnemyToSpawn;

	FTimerHandle HUDUpdateTimerHandle;

	void StartLevel();
	void UpdateHUD();
};
