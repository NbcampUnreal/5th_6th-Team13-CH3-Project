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

// 스킬2(메테오) 상태
UENUM(BlueprintType)
enum class EMeteorState : uint8
{
    None,
    Ascending,
    Aiming,
    Descending
};

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCharacterMovementComponent;
class UDecalComponent;
class UMaterialInterface;
class AFixedDamageProjectile;
class UCombatComponent;
class AFixedDamageProjectile;
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Projectile")
    TSubclassOf<AFixedDamageProjectile> ProjectileClass_Player;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_HERO_Throw;

    UFUNCTION(BlueprintCallable, Category = "Combat|Projectile")
    void FireProjectile();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Landed(const FHitResult& Hit) override;

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

    // 스킬2 입력 콜백
    void Input_MeteorStrike(const FInputActionValue& Value);

    // 스킬2 내부 처리
    void BeginMeteorAscend();
    void TickMeteor(float DeltaSeconds);
    void BeginMeteorAiming();
    void UpdateMeteorCursor();
    void CommitMeteorStrike();

    // 충돌 감지 함수
    UFUNCTION()
    void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp,
        FVector NormalImpulse, const FHitResult& Hit);

public:
    // 카메라
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArmComp;

	// 실제 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

    // 입력
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* IMC_HERO;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_HERO_Look;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_HERO_Accelerate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_HERO_DashSkill;

    // 스킬2 입력 액션 추가
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_HERO_MeteorStrike;

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

    // 경험치(고정 규칙: MAX_EXP는 100, 초과 시 레벨업하고 EXP=0)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    float EXP;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    float MAX_EXP;

    // HP
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
    float HP;

	// 최대 HP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float MaxHP;

    // 스킬1(대쉬)
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

    // 스킬2(메테오) 커서 데칼
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    UDecalComponent* MeteorCursorDecal;

    // 커서에 쓸 머티리얼(선택)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    UMaterialInterface* MeteorCursorMaterial;

    // 낙하 시 스폰할 구형 액터 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    TSubclassOf<AActor> MeteorAOESphereClass;

    // 메테오 파라미터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorTargetHeight;          // 상승 목표 Z 추가값

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorAscendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorFallSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorAimMaxDistance;        // 카메라 전방 트레이스 거리

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorAOESphereLifeSeconds;  // 스폰된 구형 액터 수명(0이면 무제한)

    // 스킬2 상태값
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    EMeteorState MeteorState = EMeteorState::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorStartZ = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorTargetZ = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    float MeteorSavedGravityScale = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    bool bMeteorAimValid = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Meteor")
    FVector MeteorAimLocation = FVector::ZeroVector;

public:
    // 이벤트 바인딩용 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHeroLevelUp OnHeroLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHeroDeath OnHeroDeath;

    // 공개 함수
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

    UFUNCTION(BlueprintCallable, Category = "HP")
    void ForceLevelUp();

    // 경험치 
    UFUNCTION(BlueprintCallable, Category = "Level/Stats")
    void AddExp(float Amount);

    UFUNCTION(BlueprintPure, Category = "Level/Stats")
    float GetExpProgress01() const;

    // 크기 동기화 함수 - CSM
    void SyncSizeToScale();

    /* ===========================
     *  HitDamageable 인터페이스 구현부 (헤더 선언)
     * =========================== */
    virtual float GetCurrentHealth() const override { return HP; }
    virtual float GetMaxHealth() const override { return MaxHP; }
    virtual void  SetCurrentHealth(float NewValue) override;
    virtual int32 GetLevel() const override { return GetHeroLevel(); }
    virtual float GetSizeScale() const override { return GetActorScale3D().GetMax(); }
    virtual float GetMaxSpeed() const override { return MAX_V; }
    virtual float GetCurrentSpeed() const override { return GetVelocity().Size(); }
    virtual bool  IsDead() const override { return HP <= 0.f; }
    virtual void  OnDead() override { OnHeroDeath.Broadcast(); }
    virtual void  EnableRagdollAndImpulse(const FVector& Impulse) override;

<<<<<<< Updated upstream
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombatComponent* CombatComp;
=======
	}

	virtual void  EnableRagdollAndImpulse(const FVector& Impulse) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Projectile")
	TSubclassOf<AFixedDamageProjectile> ProjectileClass_Player;

	UFUNCTION(BlueprintCallable, Category = "Combat|Projectile")
	void FireProjectile();
>>>>>>> Stashed changes
};