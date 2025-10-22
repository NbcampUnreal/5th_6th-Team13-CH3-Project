#include "BaseMonsterCharacter.h"
#include "Components/CapsuleComponent.h"


ABaseMonsterCharacter::ABaseMonsterCharacter()
{

	PrimaryActorTick.bCanEverTick = false;

}

void ABaseMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	SyncSizeToScale();
	//블루프린트 대신 쓴 코드             UFUNCTION과 함깨 사용
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseMonsterCharacter::OnCapsuleHit);
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

void ABaseMonsterCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	//캐릭터가 없어서 임시
	UE_LOG(LogTemp, Warning, TEXT("Monster Hit With %s"), *GetNameSafe(Other));
}



