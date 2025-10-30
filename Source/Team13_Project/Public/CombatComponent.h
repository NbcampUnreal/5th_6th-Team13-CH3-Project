// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class IHitDamageable;

USTRUCT()
struct FImpactDamageSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	float ImpactWeight = 1.0f; 

	UPROPERTY(EditAnywhere)
	float MassWeight = 0.25f;  

	UPROPERTY(EditAnywhere) 
	float RagdollImpulseScale = 30000.0f;
};

USTRUCT()
struct FHitFeedbackSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere) 
	float KnockbackScalarXY = 1.5f;
	UPROPERTY(EditAnywhere) 
	float KnockbackScalarZ = 0.35f;
	UPROPERTY(EditAnywhere) 
	float UpRatioForDir = 0.22f;

	UPROPERTY(EditAnywhere)
	float TempGroundFriction = 0.6f;
	UPROPERTY(EditAnywhere) 
	float TempBrakingDecel = 120.f;
	UPROPERTY(EditAnywhere) 
	float TempResetDelay = 0.18f;

	UPROPERTY(EditAnywhere) 
	bool  bEnableHitStop = true;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableHitStop")) 
	float HitStopDuration = 0.05f;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableHitStop")) 
	float GlobalTimeDilationDuringHitStop = 0.05f;

	UPROPERTY(EditAnywhere) 
	float ImpactMin = 250.f;
	UPROPERTY(EditAnywhere) 
	float ImpactMax = 1200.f;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFeedbackPlayed, float, Impact, FVector, KnockDir, FVector, HitLocation);


USTRUCT()
struct FRoleDecision
{
	GENERATED_BODY()
	TScriptInterface<IHitDamageable> Attacker;

	TScriptInterface<IHitDamageable> Defender;

	bool bValid = false;

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	FString Reason;
#endif
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAM13_PROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Combat")   FImpactDamageSettings Settings;
	UPROPERTY(EditAnywhere, Category = "Feedback") FHitFeedbackSettings  Feedback;

	// ���� ����: ���� �켱 �� ������ �ӵ� �� ũ�� �� �ּ�
	FRoleDecision DecideRoles(const TScriptInterface<IHitDamageable>& A,const TScriptInterface<IHitDamageable>& B,bool bIgnoreLevel = false) const;

	// �浹 ������(��� �� ���׵�+���޽�)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyImpactDamage(const TScriptInterface<IHitDamageable>& Attacker,const TScriptInterface<IHitDamageable>& Defender,const FVector& ImpactDirection) const;

	// ���� ������(����/��ô)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyFixedDamage(const TScriptInterface<IHitDamageable>& Target,float Damage,const FVector& HitImpulseDir) const;

	// ���(Ư�� ���Ÿ�, Ʈ�� ��)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyInstantKill(const TScriptInterface<IHitDamageable>& Target,const FVector& HitImpulseDir) const;

	// �ǰ� ����(Defender ����) : �˹�, ��Ʈ����, BP �̺�Ʈ ��ε�ĳ��Ʈ
	UFUNCTION(BlueprintCallable, Category = "Feedback")
	void ApplyCollisionFeedbackForDefender(const TScriptInterface<IHitDamageable>& Defender,AActor* Attacker,const FHitResult& Hit);

	UPROPERTY(BlueprintAssignable, Category = "Feedback")
    FOnFeedbackPlayed OnFeedbackPlayed;

private:
	float ComputeImpactDamage(const TScriptInterface<IHitDamageable>& Attacker,const TScriptInterface<IHitDamageable>& Defender) const;

	static FVector MakeBounceDirFromAttacker(const AActor* Attacker, float UpRatio);
		
};
