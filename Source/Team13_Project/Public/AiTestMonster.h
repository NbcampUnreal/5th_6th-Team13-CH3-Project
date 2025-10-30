// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMonsterCharacter.h"
#include "AiTestMonster.generated.h"

/**
 * 
 */
UCLASS()
class TEAM13_PROJECT_API AAiTestMonster : public ABaseMonsterCharacter
{
	GENERATED_BODY()

public:
	AAiTestMonster();
	void SetMovementSpeed(float NewSpeed);

	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float RunSpeed = 600.0f;
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
};
