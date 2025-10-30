// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "HitDamageable.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"


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

void UCombatComponent::ApplyImpactDamage(const TScriptInterface<IHitDamageable>& Attacker,const TScriptInterface<IHitDamageable>& Defender, const FVector& ImpactDirection) const
{
    if (!Attacker || !Defender || Defender->IsDead()) return;

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

void UCombatComponent::ApplyFixedDamage(const TScriptInterface<IHitDamageable>& Target,  float Damage,const FVector& HitImpulseDir) const
{
    if (!Target || Target->IsDead())
        return;
    const float NewHP = FMath::Max(Target->GetCurrentHealth() - FMath::Max(Damage, 0.f), 0.f);

    Target->SetCurrentHealth(NewHP);

    if (NewHP <= 0.f)
    {
        Target->OnDead();
        Target->EnableRagdollAndImpulse(HitImpulseDir.GetSafeNormal() * Settings.RagdollImpulseScale);
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

        OnFeedbackPlayed.Broadcast(Impact, Dir, Hit.ImpactPoint);
    }
}
