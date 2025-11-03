#include "HERO_Character.h"

#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

#include "CombatComponent.h"//충돌 데미지 함수

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

AHERO_Character::AHERO_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	/* ===========================
	 *  컴포넌트 구성
	 *  (ACharacter는 이미 CapsuleComponent/CharacterMovement/스켈메시(=GetMesh())를 가짐)
	 * =========================== */

	 // 캡슐 크기 세팅 (언리얼 기본 ThirdPerson 비슷하게)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// 카메라 붐 (3인칭 뷰)
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(GetCapsuleComponent());
	SpringArmComp->TargetArmLength = 300.0f;            // 카메라 거리
	SpringArmComp->bUsePawnControlRotation = true;      // 컨트롤러 회전에 따라 카메라 붐이 회전

	// 실제 카메라
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;        // 카메라 자체는 붐을 따르므로 pawn 회전에 직접 종속 X

	// 캐릭터 회전 설정:
	// - 컨트롤러의 Yaw로 캐릭터 몸을 직접 돌림 (3인칭식)
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false; // 좌우 이동 안씀
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
		// 중력, 지면 걷기 등은 CharacterMovement가 기본 제공
	}

	/* ===========================
	 *  기본 스탯 초기값
	 * =========================== */

	CURRENT_V = 10.0f;
	MAX_V = 10000.0f;
	PLUS_V = 500.0f;
	MIUS_V = 500.0f;
	bIsAccelerating = false;

	Level = 1;
	BASE_MAX_V = 10000.0f;
	BASE_PLUS_V = 500.0f;
	INC_MAX_V_PER_LVL = 1.0f;
	INC_PLUS_V_PER_LVL = 1.0f;
	Weight = 10.0f;

	// HP / 레벨업 관련
	MaxHP = 3.0f;
	HP = MaxHP;

	// 스킬(대쉬) 관련
	CurrentSkillState = ESkillState::Normal;
	DashDuration = 10.0f;
	DashTimer = 0.0f;
	DashCooldown = 10.0f;
	DashCooldownRemaining = 0.0f;

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
}

void AHERO_Character::BeginPlay()
{
	Super::BeginPlay();

	// 레벨 기반으로(=Level값 기반으로) 속도 스탯 다시 계산
	ApplyLevelStats();

	// CURRENT_V가 MAX_V를 넘지 않도록 보정
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// CharacterMovement 의 MaxWalkSpeed에 우리 CURRENT_V 반영
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CURRENT_V;
	}

	// Enhanced Input: IMC_HERO (매핑 컨텍스트 등록)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (IMC_HERO)
				{
					Subsystem->AddMappingContext(IMC_HERO, 0);
				}
			}
		}
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetNotifyRigidBodyCollision(true);
		Capsule->OnComponentHit.AddDynamic(this, &AHERO_Character::OnCapsuleHit);
	}
}

void AHERO_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 쿨다운/타이머 등 스킬 관련 시간 처리
	HandleCooldowns(DeltaSeconds);

	// 상태에 따라 이동 처리
	if (CurrentSkillState == ESkillState::Dashing)
	{
		HandleDash(DeltaSeconds);
	}
	else
	{
		HandleMovement(DeltaSeconds);
	}

	// 최신 CURRENT_V를 이동 컴포넌트에 반영
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CURRENT_V;
	}
}

void AHERO_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input 전용으로 바인딩
	if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 카메라 회전 / 시점 이동
		if (IA_HERO_Look)
		{
			EI->BindAction(IA_HERO_Look, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Look);
		}

		// 가속 버튼 (누르면 가속, 떼면 감속)
		if (IA_HERO_Accelerate)
		{
			EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Accelerate);
			EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Completed, this, &AHERO_Character::Input_Accelerate);
		}

		// 대쉬 스킬 (토글식: Normal -> AimingDash -> Dashing)
		if (IA_HERO_DashSkill)
		{
			EI->BindAction(IA_HERO_DashSkill, ETriggerEvent::Started, this, &AHERO_Character::Input_DashSkill);
		}
	}
}

/* ===========================
 *  내부 처리: 쿨다운 / 타이머
 * =========================== */
void AHERO_Character::HandleCooldowns(float DeltaSeconds)
{
	// 대쉬 쿨다운 경과
	if (DashCooldownRemaining > 0.0f)
	{
		DashCooldownRemaining -= DeltaSeconds;
		if (DashCooldownRemaining < 0.0f)
		{
			DashCooldownRemaining = 0.0f;
		}
	}

	// 돌진 지속 시간 관리
	if (CurrentSkillState == ESkillState::Dashing)
	{
		DashTimer -= DeltaSeconds;
		if (DashTimer <= 0.0f)
		{
			// 돌진 종료 → Normal 복귀
			CurrentSkillState = ESkillState::Normal;
			DashTimer = 0.0f;
		}
	}
}

/* ===========================
 *  내부 처리: Dash 이동
 * =========================== */
void AHERO_Character::HandleDash(float /*DeltaSeconds*/)
{
	// 돌진 스킬 사용시 강제최고속도 유지
	CURRENT_V = MAX_V;

	// 바라보는 방향으로 계속 진행
	const FVector ForwardDir = GetActorForwardVector();
	AddMovementInput(ForwardDir, 1.0f);
}

/* ===========================
 *  내부 처리: 일반 이동
 * =========================== */
void AHERO_Character::HandleMovement(float DeltaSeconds)
{
	if (bIsAccelerating)
	{
		// 가속 중이면 속도 증가
		CURRENT_V += PLUS_V * DeltaSeconds;
	}
	else
	{
		// 가속 안 하고 있으면 감속
		CURRENT_V -= MIUS_V * DeltaSeconds;
	}

	// 속도 범위 제한
	if (CURRENT_V < 0.0f)
	{
		CURRENT_V = 0.0f;
	}
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// 속도가 0 이상이면 전방으로 계속 이동
	if (CURRENT_V > 0.0f)
	{
		const FVector ForwardDir = GetActorForwardVector();
		AddMovementInput(ForwardDir, 1.0f);
	}
}

/* ===========================
 *  레벨 스탯 처리
 * =========================== */
void AHERO_Character::ApplyLevelStats()
{
	// 속도 성장 공식
	MAX_V = BASE_MAX_V + INC_MAX_V_PER_LVL * (static_cast<float>(Level) - 1.0f);
	PLUS_V = BASE_PLUS_V + INC_PLUS_V_PER_LVL * (static_cast<float>(Level) - 1.0f);

	// MIUS_V, Weight 등, 레벨별 변화 공식을 세우고자 한다면 이곳에서
}

void AHERO_Character::LevelUpInternal()
{
	Level += 1;

	// 레벨업에 따른 스텟 재계산
	ApplyLevelStats();

	// 레벨업에 따른 체력 회복
	HP = MaxHP;

	// 속도 오류 방지)
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}
}

/* ===========================
 *  HP / 레벨용 공개 함수
 * =========================== */
void AHERO_Character::ApplyDamage(float DamageAmount)
{
	HP -= DamageAmount;
	if (HP < 0.0f)
	{
		HP = 0.0f;
		// 사망 후 처리를 여기서 적용 (파괴, 날아감 등등)
	}
}

void AHERO_Character::Heal(float HealAmount)
{
	HP += HealAmount;
	if (HP > MaxHP)
	{
		HP = MaxHP;
		// 힐 기능, 혹시 몰라서 추가함
	}
}

void AHERO_Character::ForceLevelUp()
{
	LevelUpInternal();
}

/* ===========================
 *  입력 콜백
 * =========================== */

 // 가속(IA_HERO_Accelerate -> bool)
void AHERO_Character::Input_Accelerate(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	bIsAccelerating = bPressed;
}

// 시야 회전(IA_HERO_Look -> Vector2D)
void AHERO_Character::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	// 컨트롤러 회전값에 누적 → 카메라 붐이 따라돔
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

// 대쉬 스킬(IA_HERO_DashSkill -> bool)
void AHERO_Character::Input_DashSkill(const FInputActionValue& /*Value*/)
{
	// Normal 상태에서 쿨다운이 0이면 → AimingDash 진입
	if (CurrentSkillState == ESkillState::Normal && DashCooldownRemaining <= 0.0f)
	{
		CurrentSkillState = ESkillState::AimingDash;
		return;
	}

	// 준비 상태에서 다시 누르면 실제 돌진 시작
	if (CurrentSkillState == ESkillState::AimingDash)
	{
		CurrentSkillState = ESkillState::Dashing;
		DashTimer = DashDuration;
		DashCooldownRemaining = DashCooldown;

		// 돌진 시작 순간 속도를 최대로 끌어올림
		CURRENT_V = MAX_V;
		return;
	}

	// Dashing 중에는 무시
}

void AHERO_Character::SyncSizeToScale() // 크기 증가 함수 - CSM
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

void AHERO_Character::SetCurrentHealth(float NewValue)
{
	HP = NewValue;
	if (HP <= 0.f)
	{
		OnDead();
	}
}


void AHERO_Character::EnableRagdollAndImpulse(const FVector& Impulse)
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (!MeshComp->IsSimulatingPhysics())
		{
			MeshComp->SetSimulatePhysics(true);
			MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
			if (UCapsuleComponent* Cap = GetCapsuleComponent())
			{
				Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
		MeshComp->AddImpulse(Impulse, NAME_None, true);
	}
}

void AHERO_Character::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Other || !CombatComp) return;

	TScriptInterface<IHitDamageable> Me(this);
	TScriptInterface<IHitDamageable> Target(Other);
	if (!Me || !Target) return;

	const FRoleDecision R = CombatComp->DecideRoles(Me, Target, /*bIgnoreLevel=*/false);
	if (!R.bValid) return;

	AActor* AttackerActor = Cast<AActor>(R.Attacker.GetObject());
	AActor* DefenderActor = Cast<AActor>(R.Defender.GetObject());
	if (!AttackerActor || !DefenderActor) return;

	FVector Dir = DefenderActor->GetActorLocation() - AttackerActor->GetActorLocation();
	if (Dir.IsNearlyZero()) { Dir = AttackerActor->GetActorForwardVector(); }
	Dir = Dir.GetSafeNormal();

	CombatComp->ApplyImpactDamage(R.Attacker, R.Defender, Dir);

	if (DefenderActor == this)
	{
		CombatComp->ApplyCollisionFeedbackForDefender(Me, AttackerActor, Hit);
	}
}