// Fill out your copyright notice in the Description page of Project Settings.

#pragma once



#include "CoreMinimal.h"
#include "BaseMonsterCharacter.h"
#include "AiTestMonster.generated.h"


/**
 * 
 */

class AHERO_Character; // 플레이어 클래스 전방 선언
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

	// ===========================
	// 아웃라인 관련 추가
	// ===========================

	/** 감지된 플레이어 캐릭터의 참조 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AHERO_Character> PlayerCharacter;

	/** 플레이어 레벨 변경 시 호출될 함수 */
	UFUNCTION()
	void UpdateOutlineByPlayerLevel();
};
