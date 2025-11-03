#include "HERO_Character.h"

#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

AHERO_Character::AHERO_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	/* -------------------------------------------------
	 * 컴포넌트 구성
	 * ------------------------------------------------- */

	 // 캡슐 크기 (언리얼 기본 ThirdPerson 캐릭터 비슷한 값)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// 3인칭 시점용 카메라 붐(SpringArm)
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(GetCapsuleComponent());
	SpringArmComp->TargetArmLength = 300.0f;           // 카메라 거리
	SpringArmComp->bUsePawnControlRotation = true;     // 컨트롤러 Yaw/Pitch에 따라 카메라 붐이 회전

	// 실제 카메라
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;       // 카메라는 붐을 따라가므로 직접 회전 안 함

	// 회전 정책:
	// - 캐릭터 몸은 이동 방향을 따라 자동 회전
	// - 카메라는 컨트롤러 회전으로 움직임
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true; // 이동 방향으로 몸 회전
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
	}

	/* -------------------------------------------------
	 * 기본 스탯 초기화
	 * (초기 레벨 1 기준 값)
	 * ------------------------------------------------- */

	 // 이동/가속 관련
	CURRENT_V = 10.0f;
	MAX_V = 10000.0f;
	PLUS_V = 500.0f;
	MIUS_V = 500.0f;
	bIsAccelerating = false;

	// 레벨 / 성장 관련
	Level = 1;
	BASE_MAX_V = 10000.0f;
	BASE_PLUS_V = 500.0f;
	INC_MAX_V_PER_LVL = 1.0f;
	INC_PLUS_V_PER_LVL = 1.0f;

	// 일반 스탯 (추후 사용 예정)
	Weight = 10.0f;

	// HP 관련
	MaxHP = 100.0f;
	HP = MaxHP;

	// 스킬(대쉬) 관련
	CurrentSkillState = ESkillState::Normal;
	DashDuration = 10.0f;
	DashTimer = 0.0f;
	DashCooldown = 10.0f;
	DashCooldownRemaining = 0.0f;
}

void AHERO_Character::BeginPlay()
{
	Super::BeginPlay();

	// 레벨 기반으로 이동 스탯 갱신
	ApplyLevelStats();

	// CURRENT_V가 MAX_V보다 크지 않게 보정
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// CharacterMovement의 이동속도(MaxWalkSpeed)에 우리 CURRENT_V 반영
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CURRENT_V;
	}

	// Enhanced Input: 매핑 컨텍스트 등록
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (IMC_HERO)
				{
					Subsystem->AddMappingContext(IMC_HERO, /*Priority*/0);
				}
			}
		}
	}
}

void AHERO_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 시간 경과 처리 (쿨다운, 대쉬 타이머 등)
	HandleCooldowns(DeltaSeconds);

	// 이동 처리: 상태에 따라 다르게
	if (CurrentSkillState == ESkillState::Dashing)
	{
		HandleDash(DeltaSeconds);
	}
	else
	{
		HandleMovement(DeltaSeconds);
	}

	// 최신 CURRENT_V를 CharacterMovement에 반영
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CURRENT_V;
	}
}

void AHERO_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input으로 바인딩
	if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 시야 회전
		if (IA_HERO_Look)
		{
			EI->BindAction(IA_HERO_Look, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Look);
		}

		// 가속/감속
		if (IA_HERO_Accelerate)
		{
			// 누르는 동안 Triggered (true 유지)
			EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Accelerate);
			// 뗄 때 Completed (false 전달)
			EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Completed, this, &AHERO_Character::Input_Accelerate);
		}

		// 대쉬 스킬
		if (IA_HERO_DashSkill)
		{
			EI->BindAction(IA_HERO_DashSkill, ETriggerEvent::Started, this, &AHERO_Character::Input_DashSkill);
		}
	}
}


/* -------------------------------------------------
 * 내부 처리: 시간 경과 (쿨다운, 대쉬 타이머 등)
 * ------------------------------------------------- */
void AHERO_Character::HandleCooldowns(float DeltaSeconds)
{
	// 대쉬 쿨다운 남은 시간 줄이기
	if (DashCooldownRemaining > 0.0f)
	{
		DashCooldownRemaining -= DeltaSeconds;
		if (DashCooldownRemaining < 0.0f)
		{
			DashCooldownRemaining = 0.0f;
		}
	}

	// Dashing 상태에서의 남은 지속시간 줄이기
	if (CurrentSkillState == ESkillState::Dashing)
	{
		DashTimer -= DeltaSeconds;
		if (DashTimer <= 0.0f)
		{
			// 대쉬 끝 → Normal로 복귀
			CurrentSkillState = ESkillState::Normal;
			DashTimer = 0.0f;
		}
	}
}


/* -------------------------------------------------
 * 내부 처리: Dashing 이동
 * ------------------------------------------------- */
void AHERO_Character::HandleDash(float /*DeltaSeconds*/)
{
	// 돌진 중에는 무조건 최고속도 유지
	CURRENT_V = MAX_V;

	// 바라보는 방향으로 계속 이동
	const FVector ForwardDir = GetActorForwardVector();
	AddMovementInput(ForwardDir, 1.0f);
}


/* -------------------------------------------------
 * 내부 처리: 일반 이동(가속/감속)
 * ------------------------------------------------- */
void AHERO_Character::HandleMovement(float DeltaSeconds)
{
	if (bIsAccelerating)
	{
		// 가속 중이라면 속도 증가
		CURRENT_V += PLUS_V * DeltaSeconds;
	}
	else
	{
		// 가속 중이 아니면 속도 감소
		CURRENT_V -= MIUS_V * DeltaSeconds;
	}

	// 속도를 [0, MAX_V] 범위로 제한
	if (CURRENT_V < 0.0f)
	{
		CURRENT_V = 0.0f;
	}
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// 속도가 양수면 앞으로 계속 이동
	if (CURRENT_V > 0.0f)
	{
		const FVector ForwardDir = GetActorForwardVector();
		AddMovementInput(ForwardDir, 1.0f);
	}
}


/* -------------------------------------------------
 * 레벨 기반 스탯 재계산
 * ------------------------------------------------- */
void AHERO_Character::ApplyLevelStats()
{
	// MAX_V = BASE_MAX_V + (Level - 1) * INC_MAX_V_PER_LVL
	MAX_V = BASE_MAX_V + INC_MAX_V_PER_LVL * (static_cast<float>(Level) - 1.0f);

	// PLUS_V = BASE_PLUS_V + (Level - 1) * INC_PLUS_V_PER_LVL
	PLUS_V = BASE_PLUS_V + INC_PLUS_V_PER_LVL * (static_cast<float>(Level) - 1.0f);

	// MIUS_V와 Weight 등은 현재 버전에서는 레벨 비례 변화 없음
}


/* -------------------------------------------------
 * 내부 레벨업 처리
 * ------------------------------------------------- */
void AHERO_Character::LevelUpInternal()
{
	// (알림용) 이전의 레벨/HP 백업
	const int32 OldLevel = Level;
	const float OldHP = HP;

	// 레벨 증가
	Level += 1;

	// 이동/가속 스탯 다시 계산
	ApplyLevelStats();

	// HP 최대치 증가 규칙이 있다면 여기서 MaxHP 갱신 후 풀회복하세요.
	// 현재 버전에선 MaxHP 고정 → HP만 MaxHP로 회복
	HP = MaxHP;

	// 현재 속도가 새 MAX_V를 넘지 않게
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// 레벨업 알림 (틱 없음)
	OnHeroLevelUp.Broadcast(OldLevel, Level);

	// 레벨업 과정에서 HP가 실제로 변했으면 HP 변경 알림
	if (!FMath::IsNearlyEqual(OldHP, HP))
	{
		OnHPChanged.Broadcast(OldHP, HP, HP - OldHP);
	}
}


/* -------------------------------------------------
 * HP 관련 공개 함수
 * ------------------------------------------------- */
void AHERO_Character::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f) return;

	const float OldHP = HP;
	HP = FMath::Clamp(HP - DamageAmount, 0.f, MaxHP);

	// HP가 실제로 바뀐 경우에만 알림
	if (!FMath::IsNearlyEqual(OldHP, HP))
	{
		OnHPChanged.Broadcast(OldHP, HP, HP - OldHP);

		// 사망(OldHP>0 → HP==0) 시 알림
		if (OldHP > 0.f && HP <= 0.f)
		{
			OnHeroDeath.Broadcast();
			// 사망 처리)를 여기에서 추가
		}
	}
}

void AHERO_Character::Heal(float HealAmount)
{
	if (HealAmount <= 0.f) return;

	const float OldHP = HP;
	HP = FMath::Clamp(HP + HealAmount, 0.f, MaxHP);

	//  HP가 실제로 바뀐 경우에만 알림
	if (!FMath::IsNearlyEqual(OldHP, HP))
	{
		OnHPChanged.Broadcast(OldHP, HP, HP - OldHP);
	}
}


/* -------------------------------------------------
 * 레벨업 공개 함수 (블루프린트 등에서 호출)
 * ------------------------------------------------- */
void AHERO_Character::ForceLevelUp()
{
	LevelUpInternal();
}


/* -------------------------------------------------
 * 입력 콜백
 * ------------------------------------------------- */

 // 가속/감속 토글 (IA_HERO_Accelerate → bool)
void AHERO_Character::Input_Accelerate(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	bIsAccelerating = bPressed;
}

// 시야 회전 (IA_HERO_Look → Vector2D)
void AHERO_Character::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	// 카메라 회전 = 컨트롤러 회전
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

// 대쉬 스킬 (IA_HERO_DashSkill)
void AHERO_Character::Input_DashSkill(const FInputActionValue& /*Value*/)
{
	// Normal 상태에서 쿨다운이 0이면 조준 상태로 진입
	if (CurrentSkillState == ESkillState::Normal && DashCooldownRemaining <= 0.0f)
	{
		CurrentSkillState = ESkillState::AimingDash;
		return;
	}

	// 조준 상태에서 다시 누르면 실제 돌진 시작
	if (CurrentSkillState == ESkillState::AimingDash)
	{
		CurrentSkillState = ESkillState::Dashing;
		DashTimer = DashDuration;
		DashCooldownRemaining = DashCooldown;

		// 돌진 시작 즉시 최고속도까지 끌어올림
		CURRENT_V = MAX_V;
		return;
	}

	// 이미 Dashing이면 무시
}