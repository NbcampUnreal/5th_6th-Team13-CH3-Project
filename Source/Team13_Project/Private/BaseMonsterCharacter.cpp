#include "BaseMonsterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
// 플레이어(가해자) 진행방향 + 위쪽 성분을 섞어 대각선 상향 벡터로 만든다.
static FORCEINLINE FVector MakeBalloonBounceDir(const AActor* Attacker, float UpRatio /*0~1*/)
{
	// 1) 우선순위: (1) 속도 방향, (2) 액터 Forward, (3) 월드 X 방향
	FVector fwd = FVector::ZeroVector;

	// 속도가 있으면 그 방향 우선
	if (const APawn* P = Cast<APawn>(Attacker))
	{
		if (const UMovementComponent* Move = P->GetMovementComponent())
			fwd = Move->Velocity.GetSafeNormal();
	}
	if (fwd.IsNearlyZero() && Attacker)
		fwd = Attacker->GetActorForwardVector();

	if (fwd.IsNearlyZero())
		fwd = FVector::ForwardVector;

	// 수평 성분만 추출 후 정규화
	FVector horiz = FVector(fwd.X, fwd.Y, 0.f);
	if (horiz.IsNearlyZero())
		horiz = FVector::ForwardVector;

	horiz.Normalize();

	// 위쪽 비율을 섞어 대각선 상향
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
	//스케일 크기 고정
	SetActorScale3D(FVector(1.0f));

	//캡슐의 크기
	const float BaseRadius = 42.f;
	const float BaseHalf = 96.f;


	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		return;
	}
	//캡슐 크기 증가
	Capsule->SetCapsuleSize(BaseRadius * SizeScale, BaseHalf * SizeScale, true);


	if (USkeletalMeshComponent* mesh = GetMesh())
	{
		//메시 크기 증가
		mesh->SetRelativeScale3D(FVector(SizeScale));

		//발 높이 보정
		const float NewHalf = BaseHalf * SizeScale;
		mesh->SetRelativeLocation(FVector(0, 0, -NewHalf));

	}


}

//2중으로 커져서 이상해짐
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
	// 캐릭터가 없어서 임시 → 실제 플레이어/테스트 폰/오브젝트 다 대응
	ApplyCollisionFeedback(Other, Hit);
}

void ABaseMonsterCharacter::ApplyCollisionFeedback(AActor* Other, const FHitResult& Hit)
{
	if (!Other) return;

	if (!Other->ActorHasTag("Player")) return;

	constexpr float UpRatio = 0.22f; // 대각선 상향 정도 
	const FVector KnockDir = MakeBalloonBounceDir(Other, UpRatio);

	// 1) 상대 속도와 크기
	const FVector SelfVel = GetVelocity();
	const FVector OtherVel = GetActorVelocitySafe(Other);
	//const FVector RelVel = OtherVel - SelfVel;
	const float   ApproachSpeed = (OtherVel - SelfVel).Size(); // 복잡한 Dot 빼고 ‘체감 속도’로
	const float   OtherSize = GetActorSizeScaleSafe(Other);
	const float   RawImpact = ApproachSpeed * OtherSize;

	const float ImpactClamped = FMath::Clamp(RawImpact, 250.f, 1200.f);

	//// 2) 충격 방향(상대에서 멀어지는 방향 + 히트 노멀 반대 평균)
	//const FVector AwayDir = (GetActorLocation() - Other->GetActorLocation()).GetSafeNormal();
	//const FVector HitAwayDir = (-Hit.ImpactNormal).GetSafeNormal();
	//const FVector KnockDir = (AwayDir + HitAwayDir).GetSafeNormal();

	//// 3) 접근 속도 성분(음수면 0)
	//const float ApproachSpeed = FMath::Max(0.f, FVector::DotProduct(RelVel, KnockDir));

	//// 4) 충격량 = 상대 속도 × 상대 크기
	//const float OtherSize = GetActorSizeScaleSafe(Other);
	//const float Impact = ApproachSpeed * OtherSize;


	if (bEnableHitStop)
	{
		DoGlobalHitStop();
	}
	if (!IsDead())
	{
		// 수평 크게 + 위쪽 적게 → 풍선처럼 ‘퉁’하고 뜀
		const float XY = ImpactClamped * 1.5f; // 수평 강하게
		const float Z = ImpactClamped * 0.35f; // Z는 과하지 않게

		const FVector v = FVector(KnockDir.X, KnockDir.Y, 0).GetSafeNormal() * XY + FVector(0, 0, Z);
		LaunchCharacter(v, /*bXYOverride=*/true, /*bZOverride=*/true);

		// (선택) 잠깐 미끄러지게 해서 착지/밀림 감 살리기
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
		// 레그돌 던지기: 같은 방향으로, 위쪽 성분 과하지 않게 캡
		if (!GetMesh() || !GetMesh()->IsSimulatingPhysics()) EnterRagdoll();

		const FVector Dir = (FVector(KnockDir.X, KnockDir.Y, 0).GetSafeNormal() + FVector(0, 0, 0.18f)).GetSafeNormal();
		const float   P = FMath::Clamp(ImpactClamped, 300.f, 1200.f) * (RagdollImpulseScalar / 50000.f);

		if (USkeletalMeshComponent* MeshComp = GetMesh())
			MeshComp->AddImpulse(Dir * P, NAME_None, true);
	}

	BP_OnHitFeedback(ImpactClamped, KnockDir, Hit.ImpactPoint);
	//// 5) 살아있으면 넉백, 죽었으면 레그돌로 던지기
	//if (!IsDead())
	//{
	//	ApplyAliveKnockback(KnockDir, Impact);
	//}
	//else
	//{
	//	PlayDeathRagdollThrow(KnockDir, Impact, Hit.ImpactPoint);
	//}

	//// 6) BP 훅
	//BP_OnHitFeedback(Impact, KnockDir, Hit.ImpactPoint);
}
void ABaseMonsterCharacter::ApplyAliveKnockback(const FVector& KnockDir, float Impact)
{
	// 살아있을 때는 LaunchCharacter로 간단 넉백
	LaunchCharacter(KnockDir * Impact * KnockbackScalar, true, true);
}

void ABaseMonsterCharacter::PlayDeathRagdollThrow(const FVector& KnockDir, float Impact, const FVector& HitLocation)
{
	// 이미 죽어있는 상태라면 레그돌로 전환되어 있어야 함. 아니면 전환 후 던지기.
	if (!GetMesh() || GetMesh()->IsSimulatingPhysics() == false)
	{
		EnterRagdoll();
	}
	ThrowRagdoll(KnockDir, Impact);
}

void ABaseMonsterCharacter::EnterRagdoll()
{
	// 이동 정지 및 컨트롤러 분리
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	DetachFromControllerPendingDestroy();

	// 메시 물리 켜기
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->WakeAllRigidBodies();
	}

	// 캡슐 끄기
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
	// 간단 전역 시간 감속 히트스톱
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