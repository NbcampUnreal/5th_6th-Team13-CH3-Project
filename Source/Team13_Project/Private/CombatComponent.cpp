// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "HitDamageable.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"
#include "Sound/SoundBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "CombatFeedbackSettings.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "HERO_Character.h"
#include "Engine/World.h"



static void OrderPair(FRoleDecision& Out,
	const TScriptInterface<IHitDamageable>& Attacker,
	const TScriptInterface<IHitDamageable>& Defender,
	const TCHAR* Why)
{
	Out.Attacker = Attacker; Out.Defender = Defender; Out.bValid = (Attacker && Defender);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	Out.Reason = Why;
#endif
}
FRoleDecision UCombatComponent::DecideRoles(const TScriptInterface<IHitDamageable>& A, const TScriptInterface<IHitDamageable>& B, bool bIgnoreLevel) const
{
    FRoleDecision R;
    if (!A || !B || A->IsDead() || B->IsDead()) return R;

    const int32 LA = A->GetLevel(), LB = B->GetLevel();
    if (LA != LB)
    {
        // 레벨 우선
        (LA > LB) ? OrderPair(R, A, B, TEXT("Level priority"))
            : OrderPair(R, B, A, TEXT("Level priority"));
        return R;
    }

    // 동레벨: 닫히는 속도
    const AActor* AA = Cast<AActor>(A.GetObject());
    const AActor* BB = Cast<AActor>(B.GetObject());
    if (!AA || !BB) { OrderPair(R, A, B, TEXT("Fallback")); return R; }

    const FVector PA = AA->GetActorLocation();
    const FVector PB = BB->GetActorLocation();

    FVector n = (PB - PA).GetSafeNormal();
    if (n.IsNearlyZero())
    {
        const FVector Vrel = BB->GetVelocity() - AA->GetVelocity();
        n = Vrel.IsNearlyZero() ? FVector::ForwardVector : Vrel.GetSafeNormal();
    }

    const float closingA = FVector::DotProduct(AA->GetVelocity(), n);
    const float closingB = FVector::DotProduct(BB->GetVelocity(), -n);

    if (!FMath::IsNearlyEqual(closingA, closingB, 1e-2f))
    {
        (closingA > closingB) ? OrderPair(R, A, B, TEXT("ClosingVelocity"))
            : OrderPair(R, B, A, TEXT("ClosingVelocity"));
        return R;
    }

    // 타이브레이커 1: 현재속도
    const float spA = A->GetCurrentSpeed();
    const float spB = B->GetCurrentSpeed();
    if (!FMath::IsNearlyEqual(spA, spB, 1e-2f))
    {
        (spA >= spB) ? OrderPair(R, A, B, TEXT("Speed tiebreak"))
            : OrderPair(R, B, A, TEXT("Speed tiebreak"));
        return R;
    }

    // 타이브레이커 2: 포인터 주소(결정적 순서 보장)
    (A.GetObject() >= B.GetObject()) ? OrderPair(R, A, B, TEXT("Ptr tiebreak"))
        : OrderPair(R, B, A, TEXT("Ptr tiebreak"));
    return R;
}
float UCombatComponent::ComputeImpactDamage(const TScriptInterface<IHitDamageable>& Attacker, const TScriptInterface<IHitDamageable>& Defender) const
{
    const float H = FMath::Max(Defender->GetMaxHealth(), 1.f);

    const float MaxS = FMath::Max(Attacker->GetMaxSpeed(), 1.f);

    const float ImpactFactor = FMath::Clamp(Attacker->GetCurrentSpeed() / MaxS, 0.f, 1.f);

    const float MassRatio = FMath::Max(Attacker->GetSizeScale(), 0.0001f) / FMath::Max(Defender->GetSizeScale(), 0.0001f);

    const float MassTerm = FMath::Max(MassRatio - 1.f, 0.f);

    return H * (Settings.ImpactWeight * ImpactFactor + Settings.MassWeight * MassTerm);

}

void UCombatComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (bFeedbackInitialized)
        return;

    const UCombatFeedbackSettings* GlobalSettings = GetDefault<UCombatFeedbackSettings>();
    if (!GlobalSettings)
        return;

    // 전역 기본 사운드 / 이펙트 자동 주입
    if (Feedback.HitSFX == nullptr && GlobalSettings->DefaultHitSFX.ToSoftObjectPath().IsValid())
    {
        Feedback.HitSFX = GlobalSettings->DefaultHitSFX.LoadSynchronous();
    }

    if (Feedback.HitVFX == nullptr && GlobalSettings->DefaultHitVFX.ToSoftObjectPath().IsValid())
    {
        Feedback.HitVFX = GlobalSettings->DefaultHitVFX.LoadSynchronous();
    }

    // 튜닝 값 (비어 있을 때만)
    if (Feedback.HitSFXVolumeBase <= 0.f)
        Feedback.HitSFXVolumeBase = GlobalSettings->DefaultSFXVolumeBase;

    if (Feedback.HitSFXPitchBase <= 0.f)
        Feedback.HitSFXPitchBase = GlobalSettings->DefaultSFXPitchBase;

    bFeedbackInitialized = true;
}

void UCombatComponent::ApplyImpactDamage(const TScriptInterface<IHitDamageable>& Attacker, const TScriptInterface<IHitDamageable>& Defender, const FVector& ImpactDirection) const
{
    if (!Attacker || !Defender || Defender->IsDead())
        return;

    AActor* DefenderActor = Cast<AActor>(Defender.GetObject());
    if (IsInvincible(DefenderActor))
        return;

    if (Attacker->GetLevel() == Defender->GetLevel())
    {
        constexpr float Eps = 1e-2f; // 동속 오차 허용치
        const float SpA = Attacker->GetCurrentSpeed();
        const float SpD = Defender->GetCurrentSpeed();

        
        if (SpD > SpA + Eps)
        {
            return; 
        }
    }
    StartInvincibility(DefenderActor);

    float Dmg = FMath::Max(ComputeImpactDamage(Attacker, Defender), 0.f);
    const bool bDefenderIsHero = (DefenderActor && DefenderActor->IsA(AHERO_Character::StaticClass()));
    if (bDefenderIsHero && Attacker->GetLevel() == Defender->GetLevel())
    {
        Dmg *= 0.2f;
    }

    const float NewHP = FMath::Max(Defender->GetCurrentHealth() - Dmg, 0.f);
    Defender->SetCurrentHealth(NewHP);

    if (NewHP <= 0.f)
    {
        Defender->OnDead();
        const FVector Impulse = ImpactDirection.GetSafeNormal() * Settings.RagdollImpulseScale * FMath::Max(1.f, Attacker->GetSizeScale());
        Defender->EnableRagdollAndImpulse(Impulse);
    }
}


void UCombatComponent::ApplyFixedDamage(const TScriptInterface<IHitDamageable>& Target,  float Damage,const FVector& HitImpulseDir) 

{
    if (!Target || Target->IsDead())
        return;

    AActor* TargetActor = Cast<AActor>(Target.GetObject());
    if (IsInvincible(TargetActor))
        return;

    
    StartInvincibility(TargetActor);

    const float NewHP = FMath::Max(Target->GetCurrentHealth() - FMath::Max(Damage, 0.f), 0.f);
    Target->SetCurrentHealth(NewHP);

    if (NewHP <= 0.f)
    {
        Target->OnDead();
        Target->EnableRagdollAndImpulse(HitImpulseDir.GetSafeNormal() * Settings.RagdollImpulseScale);
    }
}
void UCombatComponent::ApplyFixedDamage_BP(AActor* TargetActor, float Damage, const FVector& HitImpulseDir)
{
    if (!TargetActor) return;
    if (TargetActor->GetClass()->ImplementsInterface(UHitDamageable::StaticClass()))
    {
        TScriptInterface<IHitDamageable> TargetIntf;
        TargetIntf.SetObject(TargetActor);
        TargetIntf.SetInterface(Cast<IHitDamageable>(TargetActor));
        ApplyFixedDamage(TargetIntf, Damage, HitImpulseDir);
    }
}
void UCombatComponent::ApplyInstantKill(const TScriptInterface<IHitDamageable>& Target,const FVector& HitImpulseDir) const
{
    if (!Target || Target->IsDead()) return;
    Target->SetCurrentHealth(0.f);
    Target->OnDead();
    Target->EnableRagdollAndImpulse(HitImpulseDir.GetSafeNormal() * Settings.RagdollImpulseScale * 1.5f);
}

FVector UCombatComponent::MakeBounceDirFromAttacker(const AActor* Attacker, float UpRatio)
{
    FVector Fwd = FVector::ZeroVector;

    if (const ACharacter* C = Cast<ACharacter>(Attacker))
    {
        if (const UCharacterMovementComponent* M = C->GetCharacterMovement())
            Fwd = M->Velocity.GetSafeNormal();
    }

    if (Fwd.IsNearlyZero() && Attacker)
        Fwd = Attacker->GetActorForwardVector();

    if (Fwd.IsNearlyZero())
        Fwd = FVector::ForwardVector;

    FVector H(Fwd.X, Fwd.Y, 0); 

    if (H.IsNearlyZero())
        H = FVector::ForwardVector;

    H.Normalize();

    return (H + FVector(0, 0, UpRatio)).GetSafeNormal();
}

void UCombatComponent::ApplyCollisionFeedbackForDefender(const TScriptInterface<IHitDamageable>& Defender, AActor* Attacker, const FHitResult& Hit)
{


    if (!Defender || !Attacker)
        return;

   

    AActor* Def = Cast<AActor>(Defender.GetObject()); 
    
    
    if (!Def)
        return;

    auto VelOf = [](const AActor* A)->FVector
        {
            if (!A)
                return FVector::ZeroVector;

            if (const ACharacter* C = Cast<ACharacter>(A))
            {
                if (const UCharacterMovementComponent* M = C->GetCharacterMovement())
                    return M->Velocity;
            }

            if (const UPrimitiveComponent* P = Cast<UPrimitiveComponent>(A->GetRootComponent()))
                return P->GetComponentVelocity();

            return FVector::ZeroVector;
        };

    auto SizeOf = [](const AActor* A)->float 
        { 
            return A ? A->GetActorScale3D().GetMax() : 1.f; 
        };

    const float Impact = FMath::Clamp((VelOf(Attacker) - VelOf(Def)).Size() * SizeOf(Attacker),Feedback.ImpactMin, Feedback.ImpactMax);

    if (Feedback.bEnableHitStop)
    {
        if (UWorld* W = Def->GetWorld())
        {
            UGameplayStatics::SetGlobalTimeDilation(W, Feedback.GlobalTimeDilationDuringHitStop);
            FTimerHandle Th; W->GetTimerManager().SetTimer(Th, [W]() { UGameplayStatics::SetGlobalTimeDilation(W, 1.f); }, Feedback.HitStopDuration, false);
        }
    }
    const bool bDefenderDead = Defender->IsDead();
    const FVector Dir = MakeBounceDirFromAttacker(Attacker, Feedback.UpRatioForDir);

   
    if (!bDefenderDead)
    {
        if (ACharacter* DC = Cast<ACharacter>(Def))
        {
            const FVector Launch =
                FVector(Dir.X, Dir.Y, 0).GetSafeNormal() * (Impact * Feedback.KnockbackScalarXY) +
                FVector(0, 0, Impact * Feedback.KnockbackScalarZ);

            DC->LaunchCharacter(Launch, /*bXYOverride=*/true, /*bZOverride=*/true);

            if (UCharacterMovementComponent* M = DC->GetCharacterMovement())
            {
                const float OldF = M->GroundFriction;
                const float OldB = M->BrakingDecelerationWalking;

                M->GroundFriction = Feedback.TempGroundFriction;
                M->BrakingDecelerationWalking = Feedback.TempBrakingDecel;

                FTimerHandle Reset;
                DC->GetWorldTimerManager().SetTimer(
                    Reset,
                    [DC, OldF, OldB]()
                    {
                        if (UCharacterMovementComponent* MM = DC->GetCharacterMovement())
                        {
                            MM->GroundFriction = OldF;
                            MM->BrakingDecelerationWalking = OldB;
                        }
                    },
                    Feedback.TempResetDelay,
                    false
                );
            }
        }
    }

    
    PlayHitEffects(Def, Impact, Hit.ImpactPoint, Dir);

   
    OnFeedbackPlayed.Broadcast(Impact, Dir, Hit.ImpactPoint);
}

void UCombatComponent::ForEachPrimitive(AActor* Target, TFunctionRef<void(UPrimitiveComponent*)> Fn)
{
    if (!Target) return;

    TArray<UActorComponent*> Components;
    Target->GetComponents(Components);

    for (UActorComponent* Comp : Components)
    {
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
        {
            Fn(Prim);
        }
    }
}

bool UCombatComponent::IsInvincible(AActor* Target) const
{
    if (!Target) return false;
    if (const bool* bInv = InvincibleMap.Find(Target)) return *bInv;
    return false;
}


float UCombatComponent::NormalizeImpact(float Impact) const
{
    const float MinI = Feedback.ImpactMin;
    const float MaxI = Feedback.ImpactMax;
    if (MaxI <= MinI) return 1.0f;
    return FMath::Clamp((Impact - MinI) / (MaxI - MinI), 0.0f, 1.0f);
}

void UCombatComponent::PlayHitEffects(AActor* DefenderActor, float Impact, const FVector& ImpactPoint, const FVector& KnockDir) const
{
    if (!DefenderActor) return;

    const float T = NormalizeImpact(Impact);
    const FVector Loc = ImpactPoint.IsNearlyZero() ? DefenderActor->GetActorLocation() : ImpactPoint;

    // SFX
    if (Feedback.HitSFX)
    {
        const float Volume = FMath::Max(Feedback.HitSFXVolumeBase + Feedback.HitSFXVolumeByImpact * T, 0.0f);
        const float Pitch = FMath::Max(Feedback.HitSFXPitchBase + Feedback.HitSFXPitchByImpact * T, 0.01f);

        if (Feedback.bAttachSFXToDefender)
        {
            UGameplayStatics::SpawnSoundAttached(
                Feedback.HitSFX,
                DefenderActor->GetRootComponent(),
                Feedback.SFXAttachSocket,
                FVector::ZeroVector,
                EAttachLocation::KeepRelativeOffset,
                true,
                Volume,
                Pitch);
        }
        else
        {
            UGameplayStatics::PlaySoundAtLocation(
                DefenderActor,
                Feedback.HitSFX,
                Loc,
                Volume,
                Pitch);
        }
    }

    // VFX (Niagara)
    if (Feedback.HitVFX)
    {
        const float Scale = FMath::Max(Feedback.VFXScaleBase + Feedback.VFXScaleByImpact * T, 0.01f);
        const FRotator Rot = KnockDir.IsNearlyZero() ? FRotator::ZeroRotator : KnockDir.Rotation();

        if (Feedback.bAttachVFXToDefender)
        {
            UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAttached(
                Feedback.HitVFX,
                DefenderActor->GetRootComponent(),
                Feedback.VFXAttachSocket,
                Feedback.VFXOffset,
                Rot,
                EAttachLocation::KeepRelativeOffset,
                true, /* bAutoDestroy */
                true  /* bAutoActivate */);

            if (NC)
            {
                NC->SetWorldScale3D(FVector(Scale));
                NC->SetFloatParameter(TEXT("ImpactStrength"), T);
            }
        }
        else
        {
            UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                DefenderActor->GetWorld(),
                Feedback.HitVFX,
                Loc + Feedback.VFXOffset,
                Rot,
                FVector(Scale),
                true /* bAutoDestroy */,
                true /* bAutoActivate */);

            if (NC)
            {
                NC->SetFloatParameter(TEXT("ImpactStrength"), T);
            }
        }
    }
}

void UCombatComponent::ForEachMesh(AActor* Target, TFunctionRef<void(UMeshComponent*)> Fn)
{
    if (!Target) return;
    TInlineComponentArray<UMeshComponent*> Meshes;
    Target->GetComponents(Meshes);
    for (UMeshComponent* M : Meshes)
    {
        if (IsValid(M) && M->GetNumMaterials() > 0)
        {
            Fn(M);
        }
    }
}

void UCombatComponent::EnsureMidAndSetScalar(UMeshComponent* Mesh, FName Param, float Value) const
{
    const int32 Num = Mesh->GetNumMaterials();
    for (int32 i = 0; i < Num; ++i)
    {
        UMaterialInterface* Mat = Mesh->GetMaterial(i);
        UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Mat);
        if (!MID)
        {
            MID = Mesh->CreateAndSetMaterialInstanceDynamic(i);
        }
        if (MID)
        {
            MID->SetScalarParameterValue(Param, Value);
        }
    }

}

void UCombatComponent::SetBlink(AActor* Target, bool bOn) const
{
    if (!Target) return;

   
    if (const UWorld* W = Target->GetWorld())
    {
        if (W->IsNetMode(NM_DedicatedServer)) return;
    }

    BlinkStateMap.FindOrAdd(Target) = bOn;

    const FName ParamName = Feedback.BlinkScalarParam.IsNone()
        ? FName(TEXT("HitBlink"))
        : Feedback.BlinkScalarParam;

    ForEachMesh(Target, [this, ParamName, bOn](UMeshComponent* Mesh)
        {
            EnsureMidAndSetScalar(Mesh, ParamName, bOn ? 1.0f : 0.0f);
        });
}

void UCombatComponent::ToggleBlink(AActor* Target) const
{
    const bool* Cur = BlinkStateMap.Find(Target);
    const bool NewState = !(Cur && *Cur);
    SetBlink(Target, NewState);
}

void UCombatComponent::StartInvincibility(AActor* Target) const
{
    if (!Target) return;
    UWorld* W = Target->GetWorld();
    if (!W) return;

    InvincibleMap.FindOrAdd(Target) = true;

    // 첫 프레임에 켠다
    SetBlink(Target, true);

    // 깜빡임 타이머 (간격마다 on/off 토글)
    if (Feedback.BlinkInterval > 0.f)
    {
        FTimerHandle& BlinkTh = BlinkTimerMap.FindOrAdd(Target);
        W->GetTimerManager().SetTimer(
            BlinkTh,
            FTimerDelegate::CreateWeakLambda(this, [this, Target]()
                {
                    if (!IsValid(Target)) return;
                    ToggleBlink(Target);
                }),
            Feedback.BlinkInterval,
            true
        );
    }

    // 무적 종료 타이머
    FTimerHandle EndTh;
    W->GetTimerManager().SetTimer(
        EndTh,
        FTimerDelegate::CreateWeakLambda(this, [this, Target]()
            {
                StopInvincibility(Target);
            }),
        Feedback.InvincibleDuration,
        false
    );
}

void UCombatComponent::StopInvincibility(AActor* Target) const
{
    if (!Target) return;
    UWorld* W = Target->GetWorld();
    if (!W) return;

    InvincibleMap.Remove(Target);

    // 깜빡임 타이머 해제
    if (FTimerHandle* BlinkTh = BlinkTimerMap.Find(Target))
    {
        W->GetTimerManager().ClearTimer(*BlinkTh);
        BlinkTimerMap.Remove(Target);
    }

    // 확실하게 끄기
    SetBlink(Target, false);
    BlinkStateMap.Remove(Target);
}
