// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonsterCharacter.generated.h"

UCLASS()
class TEAM13_PROJECT_API ABaseMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	ABaseMonsterCharacter();

	//HP 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float CurrentHealth = 100.f;
	//죽음 확인
	UFUNCTION(BlueprintPure, Category = "Monster\Health")
	bool IsDead() const { return CurrentHealth <= 0; }

	//충돌 이벤트
	UFUNCTION()       //   충돌한 본인 컴포넌트,      충돌 대상,   충돌 대상 컴포넌트,                충격 백터,                충돌지점과 방향등 세부 정보    
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector  NormalImpulse, const FHitResult& Hit);



	//크기에 대한 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float SizeScale = 1.0f;
	//속도에 대한 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float Speed = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Knockback")
	float KnockbackScalar = 400.f;

	// 사망 시 레그돌로 날릴 때 메시 임펄스 배율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Knockback")
	float RagdollImpulseScalar = 45000.f;

	// 히트스톱 사용 여부 및 지속 시간(초) - 전역 시간 감속
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|HitStop")
	bool bEnableHitStop = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableHitStop"), Category = "Feedback|HitStop")
	float HitStopDuration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableHitStop"), Category = "Feedback|HitStop")
	float GlobalTimeDilationDuringHitStop = 0.05f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Feedback")
	void BP_OnHitFeedback(float Impact, FVector KnockDirection, FVector HitLocation);

	// 사망 시 충격 방향으로 레그돌 처리(외부에서 HP 0 처리 후 호출)
	UFUNCTION(BlueprintCallable, Category = "Monster|Feedback")
	void PlayDeathRagdollThrow(const FVector& KnockDirection, float Impact, const FVector& HitLocation);
protected:
	virtual void BeginPlay() override;

	//실재 크기 변환 함수
	void SyncSizeToScale();

	// 피드백 계산/적용
	void ApplyCollisionFeedback(AActor* Other, const FHitResult& Hit);

	// 넉백(살아있는 상태)
	void ApplyAliveKnockback(const FVector& KnockDir, float Impact);

	// 레그돌 전환 및 던지기(사망 상태)
	void EnterRagdoll();
	void ThrowRagdoll(const FVector& KnockDir, float Impact);
	void DisableCapsuleForRagdoll();

	// 히트스톱
	void DoGlobalHitStop() const;

	// 상대 속도와 크기에서 충격량 계산
	static FVector GetActorVelocitySafe(const AActor* Actor);
	static float   GetActorSizeScaleSafe(const AActor* Actor);

};
