#pragma once

#include "CoreMinimal.h"
#include "HERO_Character.h"
#include "GameFramework/GameState.h"
#include "Team13_GameState.generated.h"

UCLASS()
class TEAM13_PROJECT_API ATeam13_GameState : public AGameState
{
	GENERATED_BODY()
public:
	ATeam13_GameState();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	//int32 EnemyToSpawn;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 CurrentStageIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage")
	int32 MaxStageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage")
	TArray<FName> StageMapNames;

	FTimerHandle StageTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;

	AHERO_Character* HERO_Character;

	void StartStage();
	void EndStage();
<<<<<<< HEAD

	//dynamic delegate에 바인딩된 함수는 언리얼엔진이 참조해서 호출하기때문에 ufunction이 필수
=======
>>>>>>> ec5d385250ca373f69e12dafde2bc8f5c0cdf6f5
	UFUNCTION()
	void OnGameOver();
	void UpdateHUD();
	void OnStageTimeUp();

	float StageDuration;
};
