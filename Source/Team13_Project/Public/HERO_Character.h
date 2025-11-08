#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "HitDamageable.h" // 데미지 인터페이스
#include "HERO_Character.generated.h"

// 스킬1 상태
UENUM(BlueprintType)
enum class ESkillState : uint8
{
    Normal,
    AimingDash,
    Dashing
};

// 알림 델리게이트
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
class UCombatComponent;
class AFixedDamageProjectile;
class AMeteorAOE;

UCLASS()
class TEAM13_PROJECT_API AHERO_Character : public ACharacter, public IHitDamageable
{
    GENERATED_BODY()

public:
    AHERO_Character();

    // 스킬1 상태 Getter (AnimBP에서 사용)
    UFUNCTION(BlueprintPure, Category = "Skill")
    ESkillState GetSkillState() const { return CurrentSkillState; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Landed(const FHitResult& Hit) override;

    // 내부 처리
    void HandleCooldowns(float DeltaSeconds);
    void HandleDash(float DeltaSeconds);
    void HandleMovement(float DeltaSeconds);

    void ApplyLevelStats();
    void LevelUpInternal();

    // 입력 콜백
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_HERO_Throw;

    // 이동 스탯
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
    float CURRENT_V;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
    float MAX_V;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
    float PLUS_V;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
    float MIUS_V;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Stats")
    bool bIsAccelerating;

    // 레벨/성장
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    float BASE_MAX_V;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    float BASE_PLUS_V;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    float INC_MAX_V_PER_LVL;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level/Stats")
    float INC_PLUS_V_PER_LVL;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
    float MaxHP;

    // 스킬1(대쉬)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    ESkillState CurrentSkillState = ESkillState::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float DashDuration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
    float DashTimer;

    

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
    TSubclassOf<AMeteorAOE> MeteorAOEClass;

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

    UFUNCTION(BlueprintCallable, Category = "HP")
    float GetMaxHP() const { return MaxHP; }

    UFUNCTION(BlueprintCallable, Category = "HP")
    int32 GetHeroLevel() const { return Level; }

    UFUNCTION(BlueprintCallable, Category = "HP")
    void ApplyDamage(float DamageAmount);

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Projectile")
    TSubclassOf<AFixedDamageProjectile> ProjectileClass_Player;

    UFUNCTION(BlueprintCallable, Category = "Combat|Projectile")
    void FireProjectile();


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombatComponent* CombatComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Fire")
    float FireCooldown = 0.3f; // 0.3초마다 발사 가능 (초당 약 3.3발)

    // 내부 타이머
    FTimerHandle FireCooldownTimer;

    // 쿨타임 중인지 여부
    bool bCanFire = true;

    // 쿨타임 리셋
    void ResetFire();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Cooldown")
    bool bCanDash = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Cooldown")
    float DashCooldown = 10.0f;

    FTimerHandle DashCooldownTimer;
    void ResetDash(); // 대시 쿨타임 해제

    // 메테오도 동일 패턴
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|Cooldown")
    bool bCanMeteor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Cooldown")
    float MeteorCooldown = 8.0f;

    FTimerHandle MeteorCooldownTimer;
    void ResetMeteor(); // 메테오 쿨타임 해제

};