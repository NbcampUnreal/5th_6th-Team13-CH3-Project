#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Team13_GameInstance.generated.h"

UCLASS()
class TEAM13_PROJECT_API UTeam13_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UTeam13_GameInstance();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 CurrentExp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 MaxExp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 CurrentLevel;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 CurrentKill;

	/*UFUNCTION(BlueprintCallable, Category = "GameData")
	void AddToEXP(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "GameData")
	void AddToKill();*/
};
