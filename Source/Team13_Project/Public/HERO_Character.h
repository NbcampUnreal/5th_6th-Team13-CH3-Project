#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
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

UCLASS()
class TEAM13_PROJECT_API AHERO_Character : public ACharacter
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
    float DashCooldown;

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
};