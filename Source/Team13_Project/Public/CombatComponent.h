// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class IHitDamageable;
class USoundBase;
class UNiagaraSystem;

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

	UPROPERTY(EditAnywhere, Category = "Blink")
	FName BlinkScalarParam = TEXT("HitBlink");

	// ====== [추가] 무적/블링크 ======
	// 충돌 후 부여할 무적 시간(초)
	UPROPERTY(EditAnywhere, Category = "Invincibility")
	float InvincibleDuration = 1.f;

	// 무적 중 깜빡임 간격(초)
	UPROPERTY(EditAnywhere, Category = "Invincibility")
	float BlinkInterval = 0.2f;

	// 깜빡임을 CustomDepth로 표시
	UPROPERTY(EditAnywhere, Category = "Invincibility")
	bool bBlinkUsingCustomDepth = true;

	// ===== SFX =====
	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	USoundBase* HitSFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	float HitSFXVolumeBase = 0.9f;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	float HitSFXPitchBase = 1.0f;

	// 임팩트에 비례해 추가되는 값
	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	float HitSFXVolumeByImpact = 0.6f;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	float HitSFXPitchByImpact = 0.2f;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	bool bAttachSFXToDefender = false;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	FName SFXAttachSocket = NAME_None;

	// ===== VFX (Niagara) =====
	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	FVector VFXOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	float VFXScaleBase = 1.0f;

	// 임팩트에 비례해 추가되는 스케일
	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	float VFXScaleByImpact = 0.5f;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	bool bAttachVFXToDefender = false;

	UPROPERTY(EditAnywhere, Category = "SFX/VFX")
	FName VFXAttachSocket = NAME_None;

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

	// 역할 판정: 레벨 우선 → 같으면 속도 → 크기 → 주소
	FRoleDecision DecideRoles(const TScriptInterface<IHitDamageable>& A,const TScriptInterface<IHitDamageable>& B,bool bIgnoreLevel = false) const;

	// 충돌 데미지(사망 시 레그돌+임펄스)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyImpactDamage(const TScriptInterface<IHitDamageable>& Attacker,const TScriptInterface<IHitDamageable>& Defender,const FVector& ImpactDirection) const;

	// 고정 데미지(돌진/투척)
	UFUNCTION(BlueprintCallable, Category = "Combat")

	void ApplyFixedDamage(const TScriptInterface<IHitDamageable>& Target,float Damage,const FVector& HitImpulseDir);

	

	// 즉사(특수 원거리, 트랩 등)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyInstantKill(const TScriptInterface<IHitDamageable>& Target,const FVector& HitImpulseDir) const;

	// 피격 연출(Defender 전용) : 넉백, 히트스톱, BP 이벤트 브로드캐스트
	UFUNCTION(BlueprintCallable, Category = "Feedback")
	void ApplyCollisionFeedbackForDefender(const TScriptInterface<IHitDamageable>& Defender,AActor* Attacker,const FHitResult& Hit);

	UPROPERTY(BlueprintAssignable, Category = "Feedback")
    FOnFeedbackPlayed OnFeedbackPlayed;

	UFUNCTION(BlueprintCallable, Category = "Combat|Setup")
	void InitializeComponent();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyFixedDamage_BP(AActor* TargetActor, float Damage, const FVector& HitImpulseDir);


private:
	float ComputeImpactDamage(const TScriptInterface<IHitDamageable>& Attacker,const TScriptInterface<IHitDamageable>& Defender) const;

	static FVector MakeBounceDirFromAttacker(const AActor* Attacker, float UpRatio);


	mutable TMap<TWeakObjectPtr<AActor>, bool> InvincibleMap;
	mutable TMap<TWeakObjectPtr<AActor>, FTimerHandle> BlinkTimerMap;

	mutable TMap<TWeakObjectPtr<AActor>, bool> BlinkStateMap;

	void StartInvincibility(AActor* Target) const;
	void StopInvincibility(AActor* Target) const;

	void ToggleBlink(AActor* Target) const;              // on/off 토글
	void SetBlink(AActor* Target, bool bOn) const;       // 명시 on/off

	 //
	static void ForEachMesh(AActor* Target, TFunctionRef<void(UMeshComponent*)> Fn);

	 //
	void EnsureMidAndSetScalar(UMeshComponent* Mesh, FName Param, float Value) const;

	bool IsInvincible(AActor* Target) const;
	

	
	static void ForEachPrimitive(AActor* Target, TFunctionRef<void(UPrimitiveComponent*)> Fn);

	float NormalizeImpact(float Impact) const;
	void PlayHitEffects(AActor* DefenderActor, float Impact, const FVector& ImpactPoint, const FVector& KnockDir) const;

	bool bFeedbackInitialized = false;
};
