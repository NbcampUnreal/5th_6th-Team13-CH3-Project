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
	//�������Ʈ ��� �� �ڵ�             UFUNCTION�� �Ա� ���
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseMonsterCharacter::OnCapsuleHit);
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

void ABaseMonsterCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	//ĳ���Ͱ� ��� �ӽ�
	UE_LOG(LogTemp, Warning, TEXT("Monster Hit With %s"), *GetNameSafe(Other));
}



