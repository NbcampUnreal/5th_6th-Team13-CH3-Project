#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HERO_Character.generated.h"

/**
 * ��ų ����(�̵� ���)
 * - Normal: �׳� �ȴ�/���� ����
 * - AimingDash: �뽬 �غ� ���� (��ư 1�� ������ �� ��)
 * - Dashing: ���� ���� ��
 */
UENUM(BlueprintType)
enum class ESkillState : uint8
{
	Normal      UMETA(DisplayName = "Normal"),
	AimingDash  UMETA(DisplayName = "AimingDash"),
	Dashing     UMETA(DisplayName = "Dashing")
};

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCharacterMovementComponent;

/**
 * AHERO_Character
 *  - ACharacter ���: ĸ�� �浹, �߷�, ���� �ȱ�, ȸ�� ó�� �� �𸮾� �⺻ ����� �̵��� �ڵ����� ����
 *  - �츮�� �̵�/����/�뽬/���� �ý����� ����
 */
UCLASS()
class TEAM13_PROJECT_API AHERO_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AHERO_Character();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* ===========================
	 *  ���� ó�� �Լ�
	 * =========================== */
	void HandleCooldowns(float DeltaSeconds);
	void HandleDash(float DeltaSeconds);
	void HandleMovement(float DeltaSeconds);

	void ApplyLevelStats();   // ������ ���� ���� �����Լ�
	void LevelUpInternal();   // ���� ������ ó�� (HP/MaxHP ����)

	/* ===========================
	 *  �Է� ó�� �ݹ�
	 * =========================== */
	void Input_Accelerate(const FInputActionValue& Value); 
	void Input_Look(const FInputActionValue& Value);       
	void Input_DashSkill(const FInputActionValue& Value); 

public:
	/* ===========================
	 *  ī�޶� ���� ������Ʈ
	 *  (ĸ��/�̵�������Ʈ�� ACharacter�� �̹� ����)
	 * =========================== */

	 // 3��Ī ī�޶� ��(ĳ���� �ڿ� ����ٴ� ��������)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	// ���� ī�޶�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	/* ===========================
	 *  Enhanced Input ����
	 * =========================== */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC_HERO;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_HERO_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_HERO_Accelerate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_HERO_DashSkill;

	/* ===========================
	 *  �̵�/���� ���� ����
	 *  (���� Pawn �ڵ� �״�� �ű�)
	 * =========================== */

	 // ���� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float CURRENT_V;

	// �ְ� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float MAX_V;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float PLUS_V;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float MIUS_V;

	// ���� ��ư
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	bool bIsAccelerating;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	int32 Level;

	// �⺻ �ִ�ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float BASE_MAX_V;

	// �⺻ ���ӷ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float BASE_PLUS_V;

	// ������ MAX_V ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float INC_MAX_V_PER_LVL;

	// ������ PLUS_V ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float INC_PLUS_V_PER_LVL;

	// ���� (�ϴ� ������)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float Weight;

	/* ===========================
	 *  HP / ������ ����
	 * =========================== */

	 // ���� HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float HP;

	// �ִ� HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float MaxHP;

	// ������ MaxHP ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float HPPerLevelGain;

	/* ===========================
	 *  ��ų(�뽬) ����
	 * =========================== */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	ESkillState CurrentSkillState;

	// ���� ��ų �����Ⱓ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashDuration;

	// ���� ���� ��ų ���� �ð�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashTimer;

	// ���� ��ų ��ٿ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashCooldown;

	// ���� ���� ��ٿ� �ð�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashCooldownRemaining;

public:
	/* ===========================
	 *  �׽�Ʈ��
	 * =========================== */

	 // ���� HP
	UFUNCTION(BlueprintCallable, Category = "HP")
	float GetHP() const { return HP; }

	// �ִ� HP
	UFUNCTION(BlueprintCallable, Category = "HP")
	float GetMaxHP() const { return MaxHP; }

	// ���� ĳ���� ����
	UFUNCTION(BlueprintCallable, Category = "HP")
	int32 GetHeroLevel() const { return Level; }

	// ���� �Ա�
	UFUNCTION(BlueprintCallable, Category = "HP")
	void ApplyDamage(float DamageAmount);

	// ȸ��
	UFUNCTION(BlueprintCallable, Category = "HP")
	void Heal(float HealAmount);

	// ������ ������ (�׽�Ʈ��)
	UFUNCTION(BlueprintCallable, Category = "HP")
	void ForceLevelUp();
};