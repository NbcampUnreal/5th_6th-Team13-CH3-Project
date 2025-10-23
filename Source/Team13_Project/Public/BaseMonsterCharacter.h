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

	//HP ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float CurrentHealth = 100.f;
	//���� Ȯ��
	UFUNCTION(BlueprintPure, Category = "Monster\Health")
	bool IsDead() const { return CurrentHealth <= 0; }

	//�浹 �̺�Ʈ
	UFUNCTION()       //   �浹�� ���� ������Ʈ,      �浹 ���,   �浹 ��� ������Ʈ,                ��� ����,                �浹������ ����� ���� ����    
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector  NormalImpulse, const FHitResult& Hit);



	//ũ�⿡ ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float SizeScale = 1.0f;
	//�ӵ��� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float Speed = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Knockback")
	float KnockbackScalar = 400.f;

	// ��� �� ���׵��� ���� �� �޽� ���޽� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|Knockback")
	float RagdollImpulseScalar = 45000.f;

	// ��Ʈ���� ��� ���� �� ���� �ð�(��) - ���� �ð� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feedback|HitStop")
	bool bEnableHitStop = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableHitStop"), Category = "Feedback|HitStop")
	float HitStopDuration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableHitStop"), Category = "Feedback|HitStop")
	float GlobalTimeDilationDuringHitStop = 0.05f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Feedback")
	void BP_OnHitFeedback(float Impact, FVector KnockDirection, FVector HitLocation);

	// ��� �� ��� �������� ���׵� ó��(�ܺο��� HP 0 ó�� �� ȣ��)
	UFUNCTION(BlueprintCallable, Category = "Monster|Feedback")
	void PlayDeathRagdollThrow(const FVector& KnockDirection, float Impact, const FVector& HitLocation);
protected:
	virtual void BeginPlay() override;

	//���� ũ�� ��ȯ �Լ�
	void SyncSizeToScale();

	// �ǵ�� ���/����
	void ApplyCollisionFeedback(AActor* Other, const FHitResult& Hit);

	// �˹�(����ִ� ����)
	void ApplyAliveKnockback(const FVector& KnockDir, float Impact);

	// ���׵� ��ȯ �� ������(��� ����)
	void EnterRagdoll();
	void ThrowRagdoll(const FVector& KnockDir, float Impact);
	void DisableCapsuleForRagdoll();

	// ��Ʈ����
	void DoGlobalHitStop() const;

	// ��� �ӵ��� ũ�⿡�� ��ݷ� ���
	static FVector GetActorVelocitySafe(const AActor* Actor);
	static float   GetActorSizeScaleSafe(const AActor* Actor);

};
