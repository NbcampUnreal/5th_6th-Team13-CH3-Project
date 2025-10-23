#include "BaseMonsterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
// �÷��̾�(������) ������� + ���� ������ ���� �밢�� ���� ���ͷ� �����.
static FORCEINLINE FVector MakeBalloonBounceDir(const AActor* Attacker, float UpRatio /*0~1*/)
{
	// 1) �켱����: (1) �ӵ� ����, (2) ���� Forward, (3) ���� X ����
	FVector fwd = FVector::ZeroVector;

	// �ӵ��� ������ �� ���� �켱
	if (const APawn* P = Cast<APawn>(Attacker))
	{
		if (const UMovementComponent* Move = P->GetMovementComponent())
			fwd = Move->Velocity.GetSafeNormal();
	}
	if (fwd.IsNearlyZero() && Attacker)
		fwd = Attacker->GetActorForwardVector();

	if (fwd.IsNearlyZero())
		fwd = FVector::ForwardVector;

	// ���� ���и� ���� �� ����ȭ
	FVector horiz = FVector(fwd.X, fwd.Y, 0.f);
	if (horiz.IsNearlyZero())
		horiz = FVector::ForwardVector;

	horiz.Normalize();

	// ���� ������ ���� �밢�� ����
	FVector dir = (horiz + FVector(0, 0, UpRatio)).GetSafeNormal();
	return dir;
}


ABaseMonsterCharacter::ABaseMonsterCharacter()
{

	PrimaryActorTick.bCanEverTick = false;

}

void ABaseMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = FMath::Clamp(MaxHealth, 1.f, MaxHealth);
	SyncSizeToScale();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetNotifyRigidBodyCollision(true);
		Capsule->OnComponentHit.AddDynamic(this, &ABaseMonsterCharacter::OnCapsuleHit);
	}
}

void ABaseMonsterCharacter::SyncSizeToScale()
{
	//������ ũ�� ����
	SetActorScale3D(FVector(1.0f));

	//ĸ���� ũ��
	const float BaseRadius = 42.f;
	const float BaseHalf = 96.f;


	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		return;
	}
	//ĸ�� ũ�� ����
	Capsule->SetCapsuleSize(BaseRadius * SizeScale, BaseHalf * SizeScale, true);


	if (USkeletalMeshComponent* mesh = GetMesh())
	{
		//�޽� ũ�� ����
		mesh->SetRelativeScale3D(FVector(SizeScale));

		//�� ���� ����
		const float NewHalf = BaseHalf * SizeScale;
		mesh->SetRelativeLocation(FVector(0, 0, -NewHalf));

	}


}

//2������ Ŀ���� �̻�����
//void ABaseMonsterCharacter::SyncSizeToScale()
//{
//	SetActorScale3D(FVector(SizeScale));
//
//	if (UCapsuleComponent* capsule = GetCapsuleComponent())
//	{
//		const float BaseRadius = 42.f;
//		const float BaseHalf = 96.f;
//
//		capsule->SetCapsuleSize(BaseRadius * SizeScale, BaseHalf * SizeScale);
//
//	}
//}

void ABaseMonsterCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// ĳ���Ͱ� ��� �ӽ� �� ���� �÷��̾�/�׽�Ʈ ��/������Ʈ �� ����
	ApplyCollisionFeedback(Other, Hit);
}

void ABaseMonsterCharacter::ApplyCollisionFeedback(AActor* Other, const FHitResult& Hit)
{
	if (!Other) return;

	if (!Other->ActorHasTag("Player")) return;

	constexpr float UpRatio = 0.22f; // �밢�� ���� ���� 
	const FVector KnockDir = MakeBalloonBounceDir(Other, UpRatio);

	// 1) ��� �ӵ��� ũ��
	const FVector SelfVel = GetVelocity();
	const FVector OtherVel = GetActorVelocitySafe(Other);
	//const FVector RelVel = OtherVel - SelfVel;
	const float   ApproachSpeed = (OtherVel - SelfVel).Size(); // ������ Dot ���� ��ü�� �ӵ�����
	const float   OtherSize = GetActorSizeScaleSafe(Other);
	const float   RawImpact = ApproachSpeed * OtherSize;

	const float ImpactClamped = FMath::Clamp(RawImpact, 250.f, 1200.f);

	//// 2) ��� ����(��뿡�� �־����� ���� + ��Ʈ ��� �ݴ� ���)
	//const FVector AwayDir = (GetActorLocation() - Other->GetActorLocation()).GetSafeNormal();
	//const FVector HitAwayDir = (-Hit.ImpactNormal).GetSafeNormal();
	//const FVector KnockDir = (AwayDir + HitAwayDir).GetSafeNormal();

	//// 3) ���� �ӵ� ����(������ 0)
	//const float ApproachSpeed = FMath::Max(0.f, FVector::DotProduct(RelVel, KnockDir));

	//// 4) ��ݷ� = ��� �ӵ� �� ��� ũ��
	//const float OtherSize = GetActorSizeScaleSafe(Other);
	//const float Impact = ApproachSpeed * OtherSize;


	if (bEnableHitStop)
	{
		DoGlobalHitStop();
	}
	if (!IsDead())
	{
		// ���� ũ�� + ���� ���� �� ǳ��ó�� �������ϰ� ��
		const float XY = ImpactClamped * 1.5f; // ���� ���ϰ�
		const float Z = ImpactClamped * 0.35f; // Z�� ������ �ʰ�

		const FVector v = FVector(KnockDir.X, KnockDir.Y, 0).GetSafeNormal() * XY + FVector(0, 0, Z);
		LaunchCharacter(v, /*bXYOverride=*/true, /*bZOverride=*/true);

		// (����) ��� �̲������� �ؼ� ����/�и� �� �츮��
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			const float OldFric = Move->GroundFriction, OldBrake = Move->BrakingDecelerationWalking;
			Move->GroundFriction = 0.6f; Move->BrakingDecelerationWalking = 120.f;
			FTimerHandle Th;
			GetWorldTimerManager().SetTimer(Th, [this, OldFric, OldBrake]()
				{
					if (UCharacterMovementComponent* M = GetCharacterMovement())
					{
						M->GroundFriction = OldFric; M->BrakingDecelerationWalking = OldBrake;
					}
				}, 0.18f, false);
		}
	}
	else
	{
		// ���׵� ������: ���� ��������, ���� ���� ������ �ʰ� ĸ
		if (!GetMesh() || !GetMesh()->IsSimulatingPhysics()) EnterRagdoll();

		const FVector Dir = (FVector(KnockDir.X, KnockDir.Y, 0).GetSafeNormal() + FVector(0, 0, 0.18f)).GetSafeNormal();
		const float   P = FMath::Clamp(ImpactClamped, 300.f, 1200.f) * (RagdollImpulseScalar / 50000.f);

		if (USkeletalMeshComponent* MeshComp = GetMesh())
			MeshComp->AddImpulse(Dir * P, NAME_None, true);
	}

	BP_OnHitFeedback(ImpactClamped, KnockDir, Hit.ImpactPoint);
	//// 5) ��������� �˹�, �׾����� ���׵��� ������
	//if (!IsDead())
	//{
	//	ApplyAliveKnockback(KnockDir, Impact);
	//}
	//else
	//{
	//	PlayDeathRagdollThrow(KnockDir, Impact, Hit.ImpactPoint);
	//}

	//// 6) BP ��
	//BP_OnHitFeedback(Impact, KnockDir, Hit.ImpactPoint);
}
void ABaseMonsterCharacter::ApplyAliveKnockback(const FVector& KnockDir, float Impact)
{
	// ������� ���� LaunchCharacter�� ���� �˹�
	LaunchCharacter(KnockDir * Impact * KnockbackScalar, true, true);
}

void ABaseMonsterCharacter::PlayDeathRagdollThrow(const FVector& KnockDir, float Impact, const FVector& HitLocation)
{
	// �̹� �׾��ִ� ���¶�� ���׵��� ��ȯ�Ǿ� �־�� ��. �ƴϸ� ��ȯ �� ������.
	if (!GetMesh() || GetMesh()->IsSimulatingPhysics() == false)
	{
		EnterRagdoll();
	}
	ThrowRagdoll(KnockDir, Impact);
}

void ABaseMonsterCharacter::EnterRagdoll()
{
	// �̵� ���� �� ��Ʈ�ѷ� �и�
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	DetachFromControllerPendingDestroy();

	// �޽� ���� �ѱ�
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->WakeAllRigidBodies();
	}

	// ĸ�� ����
	DisableCapsuleForRagdoll();
}

void ABaseMonsterCharacter::ThrowRagdoll(const FVector& KnockDir, float Impact)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->AddImpulse(KnockDir * Impact * RagdollImpulseScalar, NAME_None, true);
	}
}

void ABaseMonsterCharacter::DisableCapsuleForRagdoll()
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABaseMonsterCharacter::DoGlobalHitStop() const
{
	// ���� ���� �ð� ���� ��Ʈ����
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, GlobalTimeDilationDuringHitStop);

		FTimerHandle Th;
		World->GetTimerManager().SetTimer(Th,
			[World]()
			{
				UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
			},
			HitStopDuration, false);
	}
}

FVector ABaseMonsterCharacter::GetActorVelocitySafe(const AActor* Actor)
{
	if (!Actor) return FVector::ZeroVector;

	if (const APawn* P = Cast<APawn>(Actor))
	{
		if (const UMovementComponent* Move = P->GetMovementComponent())
		{
			return Move->Velocity;
		}
	}
	if (const UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
	{
		return Prim->GetComponentVelocity();
	}
	return FVector::ZeroVector;
}

float ABaseMonsterCharacter::GetActorSizeScaleSafe(const AActor* Actor)
{
	return Actor ? Actor->GetActorScale3D().GetMax() : 1.0f;
}