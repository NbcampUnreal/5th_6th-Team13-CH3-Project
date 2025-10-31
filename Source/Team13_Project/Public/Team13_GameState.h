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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 CurrentStageIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 MaxStageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TArray<FName> StageMapNames;

	FTimerHandle StageTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;

	void StartStage();
	void EndStage();
	void OnGameOver();
	void UpdateHUD();
	
	//юс╫ц
	void OnStageTimeUp();

	float StageDuration;
};
