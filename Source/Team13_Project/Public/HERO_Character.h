#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HERO_Character.generated.h"

/**
 * 스킬 상태(이동 모드)
 * - Normal: 그냥 걷는/가속 상태
 * - AimingDash: 대쉬 준비 상태 (버튼 1번 눌렀을 때 등)
 * - Dashing: 실제 돌진 중
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
 *  - ACharacter 기반: 캡슐 충돌, 중력, 지면 걷기, 회전 처리 등 언리얼 기본 사람형 이동을 자동으로 가짐
 *  - 우리의 이동/가속/대쉬/스탯 시스템을 통합
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
	 *  내부 처리 함수
	 * =========================== */
	void HandleCooldowns(float DeltaSeconds);
	void HandleDash(float DeltaSeconds);
	void HandleMovement(float DeltaSeconds);

	void ApplyLevelStats();   // 레벨에 따른 스텟 적용함수
	void LevelUpInternal();   // 실제 레벨업 처리 (HP/MaxHP 포함)

	/* ===========================
	 *  입력 처리 콜백
	 * =========================== */
	void Input_Accelerate(const FInputActionValue& Value); 
	void Input_Look(const FInputActionValue& Value);       
	void Input_DashSkill(const FInputActionValue& Value); 

public:
	/* ===========================
	 *  카메라 관련 컴포넌트
	 *  (캡슐/이동컴포넌트는 ACharacter가 이미 가짐)
	 * =========================== */

	 // 3인칭 카메라 붐(캐릭터 뒤에 따라붙는 스프링암)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	// 실제 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	/* ===========================
	 *  Enhanced Input 설정
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
	 *  이동/가속 관련 스탯
	 *  (기존 Pawn 코드 그대로 옮김)
	 * =========================== */

	 // 현재 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float CURRENT_V;

	// 최고 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float MAX_V;

	// 가속
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float PLUS_V;

	// 감속
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float MIUS_V;

	// 가속 버튼
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	bool bIsAccelerating;

	// 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	int32 Level;

	// 기본 최대속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float BASE_MAX_V;

	// 기본 가속량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float BASE_PLUS_V;

	// 레벨당 MAX_V 증가량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float INC_MAX_V_PER_LVL;

	// 레벨당 PLUS_V 증가량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float INC_PLUS_V_PER_LVL;

	// 무게 (일단 변수만)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float Weight;

	/* ===========================
	 *  HP / 레벨업 관련
	 * =========================== */

	 // 현재 HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float HP;

	// 최대 HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float MaxHP;

	// 레벨당 MaxHP 증가량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float HPPerLevelGain;

	/* ===========================
	 *  스킬(대쉬) 관련
	 * =========================== */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	ESkillState CurrentSkillState;

	// 돌진 스킬 유지기간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashDuration;

	// 현재 돌진 스킬 남은 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashTimer;

	// 돌진 스킬 쿨다운
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashCooldown;

	// 현재 남은 쿨다운 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	float DashCooldownRemaining;

public:
	/* ===========================
	 *  테스트용
	 * =========================== */

	 // 현재 HP
	UFUNCTION(BlueprintCallable, Category = "HP")
	float GetHP() const { return HP; }

	// 최대 HP
	UFUNCTION(BlueprintCallable, Category = "HP")
	float GetMaxHP() const { return MaxHP; }

	// 현재 캐릭터 레벨
	UFUNCTION(BlueprintCallable, Category = "HP")
	int32 GetHeroLevel() const { return Level; }

	// 피해 입기
	UFUNCTION(BlueprintCallable, Category = "HP")
	void ApplyDamage(float DamageAmount);

	// 회복
	UFUNCTION(BlueprintCallable, Category = "HP")
	void Heal(float HealAmount);

	// 강제로 레벨업 (테스트용)
	UFUNCTION(BlueprintCallable, Category = "HP")
	void ForceLevelUp();
};