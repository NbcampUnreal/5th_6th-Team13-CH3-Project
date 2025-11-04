#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HitDamageable.h" // 데미지 인터페이스
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
	Normal,
	AimingDash,
	Dashing
};

/* ===========================
 *  알림(이벤트) 델리게이트 타입 선언
 *  레벨업/HP변경/사망 알림
 * =========================== */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeroLevelUp, int32, OldLevel, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHPChanged, float, OldHP, float, NewHP, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHeroDeath);

class USpringArmComponent;
class UCameraComponent;
//class UInputMappingContext;
//class UInputAction;
class UCharacterMovementComponent;
class UCombatComponent;

/**
 * AHERO_Character
 *  - ACharacter 기반: 캡슐 충돌, 중력, 지면 걷기, 회전 처리 등 언리얼 기본 사람형 이동을 자동으로 가짐
 *  - 이동/가속/대쉬/스탯 시스템을 통합
 */
UCLASS()
class TEAM13_PROJECT_API AHERO_Character : public ACharacter, public IHitDamageable
{
	GENERATED_BODY()

public:
	AHERO_Character();

	// AnimBP에서 읽기용(스킬 상태 Getter)
	UFUNCTION(BlueprintPure, Category = "Skill")
	ESkillState GetSkillState() const { return CurrentSkillState; }

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

	//충돌 감지 함수
	UFUNCTION()
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

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

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputMappingContext* IMC_HERO;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* IA_HERO_Look;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* IA_HERO_Accelerate;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* IA_HERO_DashSkill;

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

	// 크기 계수 - CSM
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
	float SizeScale = 1.0f;

	/* ===========================
	 *  HP / 레벨업 관련
	 * =========================== */

	 // 현재 HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float HP;

	// 최대 HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float MaxHP;

	/* ===========================
	 *  스킬(대쉬) 관련
	 * =========================== */

	 // 현재 스킬 상태(AnimBP에서 읽음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	ESkillState CurrentSkillState = ESkillState::Normal;

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

	/* ===========================
	 *  알림 이벤트 (BP에서 Bind 가능, 틱 사용 없음)
	 * =========================== */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHeroLevelUp OnHeroLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHeroDeath OnHeroDeath;

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
	/*UFUNCTION(BlueprintCallable, Category = "HP")
	void ApplyDamage(float DamageAmount);*/

	// 회복
	UFUNCTION(BlueprintCallable, Category = "HP")
	void Heal(float HealAmount);

	// 강제로 레벨업 (테스트용)
	UFUNCTION(BlueprintCallable, Category = "HP")
	void ForceLevelUp();

	// 크기증가 함수 - CSM
	void SyncSizeToScale();
	//데미지 관련 함수들
	virtual float GetCurrentHealth() const override { return HP; }
	virtual float GetMaxHealth() const override { return MaxHP; }
	virtual void  SetCurrentHealth(float NewValue) override;

	virtual int32 GetLevel() const override { return GetHeroLevel(); }
	virtual float GetSizeScale() const override { return GetActorScale3D().GetMax(); }

	virtual float GetMaxSpeed() const override { return MAX_V; }
	virtual float GetCurrentSpeed() const override { return GetVelocity().Size(); }

	virtual bool  IsDead() const override { return HP <= 0.f; }
	virtual void  OnDead() override { /* 필요시 추가 처리 */ }

	virtual void  EnableRagdollAndImpulse(const FVector& Impulse) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComp;
};