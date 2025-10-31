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

	/* ===========================
	 *  ������Ʈ ����
	 *  (ACharacter�� �̹� CapsuleComponent/CharacterMovement/���̸޽�(=GetMesh())�� ����)
	 * =========================== */

	 // ĸ�� ũ�� ���� (�𸮾� �⺻ ThirdPerson ����ϰ�)
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// ī�޶� �� (3��Ī ��)
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(GetCapsuleComponent());
	SpringArmComp->TargetArmLength = 300.0f;            // ī�޶� �Ÿ�
	SpringArmComp->bUsePawnControlRotation = true;      // ��Ʈ�ѷ� ȸ���� ���� ī�޶� ���� ȸ��

	// ���� ī�޶�
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;        // ī�޶� ��ü�� ���� �����Ƿ� pawn ȸ���� ���� ���� X

	// ĳ���� ȸ�� ����:
	// - ��Ʈ�ѷ��� Yaw�� ĳ���� ���� ���� ���� (3��Ī��)
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false; // �¿� �̵� �Ⱦ�
		MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
		// �߷�, ���� �ȱ� ���� CharacterMovement�� �⺻ ����
	}

	/* ===========================
	 *  �⺻ ���� �ʱⰪ
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

	// HP / ������ ����
	MaxHP = 3.0f;
	HP = MaxHP;

	// ��ų(�뽬) ����
	CurrentSkillState = ESkillState::Normal;
	DashDuration = 10.0f;
	DashTimer = 0.0f;
	DashCooldown = 10.0f;
	DashCooldownRemaining = 0.0f;
}

void AHERO_Character::BeginPlay()
{
	Super::BeginPlay();

	// ���� �������(=Level�� �������) �ӵ� ���� �ٽ� ���
	ApplyLevelStats();

	// CURRENT_V�� MAX_V�� ���� �ʵ��� ����
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// CharacterMovement �� MaxWalkSpeed�� �츮 CURRENT_V �ݿ�
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CURRENT_V;
	}

	// Enhanced Input: IMC_HERO (���� ���ؽ�Ʈ ���)
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
}

void AHERO_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ��ٿ�/Ÿ�̸� �� ��ų ���� �ð� ó��
	HandleCooldowns(DeltaSeconds);

	// ���¿� ���� �̵� ó��
	if (CurrentSkillState == ESkillState::Dashing)
	{
		HandleDash(DeltaSeconds);
	}
	else
	{
		HandleMovement(DeltaSeconds);
	}

	// �ֽ� CURRENT_V�� �̵� ������Ʈ�� �ݿ�
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = CURRENT_V;
	}
}

void AHERO_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input �������� ���ε�
	if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// ī�޶� ȸ�� / ���� �̵�
		if (IA_HERO_Look)
		{
			EI->BindAction(IA_HERO_Look, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Look);
		}

		// ���� ��ư (������ ����, ���� ����)
		if (IA_HERO_Accelerate)
		{
			EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Accelerate);
			EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Completed, this, &AHERO_Character::Input_Accelerate);
		}

		// �뽬 ��ų (��۽�: Normal -> AimingDash -> Dashing)
		if (IA_HERO_DashSkill)
		{
			EI->BindAction(IA_HERO_DashSkill, ETriggerEvent::Started, this, &AHERO_Character::Input_DashSkill);
		}
	}
}

/* ===========================
 *  ���� ó��: ��ٿ� / Ÿ�̸�
 * =========================== */
void AHERO_Character::HandleCooldowns(float DeltaSeconds)
{
	// �뽬 ��ٿ� ���
	if (DashCooldownRemaining > 0.0f)
	{
		DashCooldownRemaining -= DeltaSeconds;
		if (DashCooldownRemaining < 0.0f)
		{
			DashCooldownRemaining = 0.0f;
		}
	}

	// ���� ���� �ð� ����
	if (CurrentSkillState == ESkillState::Dashing)
	{
		DashTimer -= DeltaSeconds;
		if (DashTimer <= 0.0f)
		{
			// ���� ���� �� Normal ����
			CurrentSkillState = ESkillState::Normal;
			DashTimer = 0.0f;
		}
	}
}

/* ===========================
 *  ���� ó��: Dash �̵�
 * =========================== */
void AHERO_Character::HandleDash(float /*DeltaSeconds*/)
{
	// ���� ��ų ���� �����ְ�ӵ� ����
	CURRENT_V = MAX_V;

	// �ٶ󺸴� �������� ��� ����
	const FVector ForwardDir = GetActorForwardVector();
	AddMovementInput(ForwardDir, 1.0f);
}

/* ===========================
 *  ���� ó��: �Ϲ� �̵�
 * =========================== */
void AHERO_Character::HandleMovement(float DeltaSeconds)
{
	if (bIsAccelerating)
	{
		// ���� ���̸� �ӵ� ����
		CURRENT_V += PLUS_V * DeltaSeconds;
	}
	else
	{
		// ���� �� �ϰ� ������ ����
		CURRENT_V -= MIUS_V * DeltaSeconds;
	}

	// �ӵ� ���� ����
	if (CURRENT_V < 0.0f)
	{
		CURRENT_V = 0.0f;
	}
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}

	// �ӵ��� 0 �̻��̸� �������� ��� �̵�
	if (CURRENT_V > 0.0f)
	{
		const FVector ForwardDir = GetActorForwardVector();
		AddMovementInput(ForwardDir, 1.0f);
	}
}

/* ===========================
 *  ���� ���� ó��
 * =========================== */
void AHERO_Character::ApplyLevelStats()
{
	// �ӵ� ���� ����
	MAX_V = BASE_MAX_V + INC_MAX_V_PER_LVL * (static_cast<float>(Level) - 1.0f);
	PLUS_V = BASE_PLUS_V + INC_PLUS_V_PER_LVL * (static_cast<float>(Level) - 1.0f);

	// MIUS_V, Weight ��, ������ ��ȭ ������ ������� �Ѵٸ� �̰�����
}

void AHERO_Character::LevelUpInternal()
{
	Level += 1;

	// �������� ���� ���� ����
	ApplyLevelStats();

	// �������� ���� ü�� ȸ��
	HP = MaxHP;

	// �ӵ� ���� ����)
	if (CURRENT_V > MAX_V)
	{
		CURRENT_V = MAX_V;
	}
}

/* ===========================
 *  HP / ������ ���� �Լ�
 * =========================== */
void AHERO_Character::ApplyDamage(float DamageAmount)
{
	HP -= DamageAmount;
	if (HP < 0.0f)
	{
		HP = 0.0f;
		// ��� �� ó���� ���⼭ ���� (�ı�, ���ư� ���)
	}
}

void AHERO_Character::Heal(float HealAmount)
{
	HP += HealAmount;
	if (HP > MaxHP)
	{
		HP = MaxHP;
		// �� ���, Ȥ�� ���� �߰���
	}
}

void AHERO_Character::ForceLevelUp()
{
	LevelUpInternal();
}

/* ===========================
 *  �Է� �ݹ�
 * =========================== */

 // ����(IA_HERO_Accelerate -> bool)
void AHERO_Character::Input_Accelerate(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	bIsAccelerating = bPressed;
}

// �þ� ȸ��(IA_HERO_Look -> Vector2D)
void AHERO_Character::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	// ��Ʈ�ѷ� ȸ������ ���� �� ī�޶� ���� ����
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(LookAxis.Y);
}

// �뽬 ��ų(IA_HERO_DashSkill -> bool)
void AHERO_Character::Input_DashSkill(const FInputActionValue& /*Value*/)
{
	// Normal ���¿��� ��ٿ��� 0�̸� �� AimingDash ����
	if (CurrentSkillState == ESkillState::Normal && DashCooldownRemaining <= 0.0f)
	{
		CurrentSkillState = ESkillState::AimingDash;
		return;
	}

	// �غ� ���¿��� �ٽ� ������ ���� ���� ����
	if (CurrentSkillState == ESkillState::AimingDash)
	{
		CurrentSkillState = ESkillState::Dashing;
		DashTimer = DashDuration;
		DashCooldownRemaining = DashCooldown;

		// ���� ���� ���� �ӵ��� �ִ�� ����ø�
		CURRENT_V = MAX_V;
		return;
	}

	// Dashing �߿��� ����
}