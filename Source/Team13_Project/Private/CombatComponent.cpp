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
    FRoleDecision R; if (!A || !B || A->IsDead() || B->IsDead()) return R;

    auto SpeedThenSize = [&]() -> int
        {
            const float SA = A->GetCurrentSpeed(), SB = B->GetCurrentSpeed();

            if (!FMath::IsNearlyEqual(SA, SB)) 
                return (SA > SB) ? 0 : 1;


            const float SZA = A->GetSizeScale(), SZB = B->GetSizeScale();


            if (!FMath::IsNearlyEqual(SZA, SZB)) 
                return (SZA > SZB) ? 0 : 1;


            return (A.GetObject() < B.GetObject()) ? 0 : 1;

        };

    if (!bIgnoreLevel)
    {
        const int32 LA = A->GetLevel(), LB = B->GetLevel();
        if (LA != LB)
        {
            if (LA > LB)
            {
                OrderPair(R, A, B, TEXT("Level priority"));
            }
            else
            {
                OrderPair(R, B, A, TEXT("Level priority"));
            }
            return R;
        }
    }
    const int Pick = SpeedThenSize();

    if (Pick == 0)
        OrderPair(R, A, B, TEXT("Speed/Size tiebreak"));
    else           
        OrderPair(R, B, A, TEXT("Speed/Size tiebreak"));

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

    const float Dmg = FMath::Max(ComputeImpactDamage(Attacker, Defender), 0.f);
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
    const float NewHP = FMath::Max(Target->GetCurrentHealth() - FMath::Max(Damage, 0.f), 0.f);

    Target->SetCurrentHealth(NewHP);
    
    if (NewHP <= 0.f)
    {
        Target->OnDead();
        const FVector Impulse = HitImpulseDir.GetSafeNormal() * Settings.RagdollImpulseScale;
            
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
    if (!Defender || !Attacker || Defender->IsDead())
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

    if (ACharacter* DC = Cast<ACharacter>(Def))
    {
        const FVector Dir = MakeBounceDirFromAttacker(Attacker, Feedback.UpRatioForDir);

        const FVector Launch = FVector(Dir.X, Dir.Y, 0).GetSafeNormal() * (Impact * Feedback.KnockbackScalarXY) + FVector(0, 0, Impact * Feedback.KnockbackScalarZ);

        DC->LaunchCharacter(Launch, true, true);

        if (UCharacterMovementComponent* M = DC->GetCharacterMovement())
        {
            const float OldF = M->GroundFriction, OldB = M->BrakingDecelerationWalking;

            M->GroundFriction = Feedback.TempGroundFriction;

            M->BrakingDecelerationWalking = Feedback.TempBrakingDecel;

            FTimerHandle Reset; DC->GetWorldTimerManager().SetTimer(Reset, [DC, OldF, OldB]()
                {
                    if (UCharacterMovementComponent* MM = DC->GetCharacterMovement())
                    {
                        MM->GroundFriction = OldF; MM->BrakingDecelerationWalking = OldB;
                    }
                }, Feedback.TempResetDelay, false);
        }
        // 넉백 계산 및 LaunchCharacter 적용된 뒤:
        PlayHitEffects(Def, Impact, Hit.ImpactPoint, Dir);

        OnFeedbackPlayed.Broadcast(Impact, Dir, Hit.ImpactPoint);
        

       
    }

    

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

    // 전용 서버에서는 연출 건너뜀
    if (const UWorld* W = Target->GetWorld())
    {
        if (W->IsNetMode(NM_DedicatedServer)) return;
    }

    BlinkStateMap.FindOrAdd(Target) = bOn;

    const FName ParamName = Feedback.BlinkScalarParam.IsNone() ? FName(TEXT("HitBlink")) : Feedback.BlinkScalarParam;

    ForEachMesh(Target, [this, ParamName, bOn](UMeshComponent* Mesh)
        {
            EnsureMidAndSetScalar(Mesh, ParamName, bOn ? 1.0f : 0.0f);
        });
}