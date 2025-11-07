// Fill out your copyright notice in the Description page of Project Settings.


#include "AiTestMonster.h"
#include "TestAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// ===========================
// 아웃라인 관련 헤더 추가
// ===========================
#include "HERO_Character.h" // 플레이어 헤더
#include "Kismet/GameplayStatics.h" // GetPlayerCharacter용
#include "Components/SkeletalMeshComponent.h" // GetMesh()용

AAiTestMonster::AAiTestMonster()
{
	AIControllerClass = ATestAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	PrimaryActorTick.bCanEverTick = true;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void AAiTestMonster::SetMovementSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NewSpeed;
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Speed changed: %.1f"), NewSpeed);
	}
}

void AAiTestMonster::BeginPlay()
{
	Super::BeginPlay();
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = WalkSpeed;
	UE_LOG(LogTemp, Warning, TEXT("AI Test Spawned"));


	AICon = Cast<AAIController>(GetController());
	if (!AICon)
	{	UE_LOG(LogTemp, Warning, TEXT("AI Controller missing"));
		return;
	}
	BB = AICon->GetBlackboardComponent();
	if (!BB)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI BlackBoard missing"));
		return;
	}

	// ===========================
	// 아웃라인 관련 로직 (BeginPlay에 추가)
	// ===========================

	// 월드에서 플레이어 캐릭터를 찾습니다.
	PlayerCharacter = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (PlayerCharacter)
	{
		// 1. 즉시 아웃라인 상태를 업데이트합니다.
		UpdateOutlineByPlayerLevel(0,PlayerCharacter->GetLevel());

		// 2. 플레이어의 레벨업 이벤트에 우리 함수를 바인딩합니다.
		PlayerCharacter->OnHeroLevelUp.AddDynamic(this, &AAiTestMonster::UpdateOutlineByPlayerLevel);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("AiTestMonster: HERO_Character를 찾을 수 없습니다!"));
	}
}

void AAiTestMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//UE_LOG(LogTemp, Warning, TEXT("AI Control/BlackBoard allright"));
	bool bIsRunning = BB->GetValueAsBool(TEXT("IsRunning"));

	float TargetSpeed = bIsRunning ? 600.f : 300.f;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp && MoveComp->MaxWalkSpeed != TargetSpeed)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AI Speed Set"));
		MoveComp->MaxWalkSpeed = TargetSpeed;
	}

	if (PlayerCharacter && GetMesh() && DesiredStencilValue >= 0)
	{
		APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (!CameraManager)
		{
			GetMesh()->SetRenderCustomDepth(false); 
			return;
		}

		const FVector CameraLocation = CameraManager->GetCameraLocation();
		const FVector MonsterLocation = GetActorLocation(); 

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this); 
		Params.AddIgnoredActor(PlayerCharacter); 

		// 카메라에서 몬스터로 라인 트레이스 (Visibility 채널 사용)
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLocation,
			MonsterLocation,
			ECC_Visibility,
			Params
		);

		if (bHit)
		{
			// 장애물 있음
			GetMesh()->SetRenderCustomDepth(false);
		}
		else
		{
			// 장애물 없음
			GetMesh()->SetRenderCustomDepth(true);
			GetMesh()->SetCustomDepthStencilValue(DesiredStencilValue);
		}
	}
	else if (GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(false);
	}
}



void AAiTestMonster::UpdateOutlineByPlayerLevel(int oldlevel, int newlevel)
{
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateOutline: PlayerCharacter가 유효하지 않습니다."));
		return;
	}
	
	if (!GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateOutline: 몬스터의 Mesh가 유효하지 않습니다."));
		return;
	}

	const int32 PlayerLevel = PlayerCharacter->GetHeroLevel();
	const int32 MonsterLevel = GetLevel(); 

	const int32 STENCIL_RED_OUTLINE = 255;   // 높을때 (빨강)
	const int32 STENCIL_GREEN_OUTLINE = 128; // 같을때 (초록)
	const int32 STENCIL_BLUE_OUTLINE = 0;  // 낮을때 (파랑)
	
	if (MonsterLevel > PlayerLevel)
	{
		// 몬스터 레벨이 더 높음 -> 빨간색 아웃라인
		GetMesh()->SetRenderCustomDepth(true);
		GetMesh()->SetCustomDepthStencilValue(STENCIL_RED_OUTLINE);
		BB->SetValueAsBool(TEXT("IsUpperLevel"), true);
		DesiredStencilValue = STENCIL_RED_OUTLINE;
		UE_LOG(LogTemp, Log, TEXT("Outline: RED (Monster: %d > Player: %d)"), MonsterLevel, PlayerLevel);
	}
	else if (MonsterLevel == PlayerLevel)
	{
		// 몬스터 레벨이 같거나 낮음 -> 초록색 아웃라인
		GetMesh()->SetRenderCustomDepth(true);
		GetMesh()->SetCustomDepthStencilValue(STENCIL_GREEN_OUTLINE);
		BB->SetValueAsBool(TEXT("IsUpperLevel"), false);
		DesiredStencilValue = STENCIL_GREEN_OUTLINE;
		UE_LOG(LogTemp, Log, TEXT("Outline: Green (Monster: %d <= Player: %d)"), MonsterLevel, PlayerLevel);
	}else
	{
		// 몬스터가 허접임 -> 파란색 아웃라인
		GetMesh()->SetRenderCustomDepth(true);
		GetMesh()->SetCustomDepthStencilValue(STENCIL_BLUE_OUTLINE);
		BB->SetValueAsBool(TEXT("IsUpperLevel"), false);
		DesiredStencilValue = STENCIL_BLUE_OUTLINE;
		UE_LOG(LogTemp, Log, TEXT("Outline: BLUE (Monster: %d <= Player: %d)"), MonsterLevel, PlayerLevel);
	}
}