#include "HERO_Character.h"

#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

#include "DrawDebugHelpers.h"

// ---------------------------------------------
// 생성자
// ---------------------------------------------
AHERO_Character::AHERO_Character()
{
    PrimaryActorTick.bCanEverTick = true;

    // 컴포넌트 구성
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(GetCapsuleComponent());
    SpringArmComp->TargetArmLength = 300.0f;
    SpringArmComp->bUsePawnControlRotation = true;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    CameraComp->bUsePawnControlRotation = false;

    // 마우스 회전 기반
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // 마우스 회전이 몸 회전을 주도하므로 이동방향 자동회전은 끈다
        MoveComp->bOrientRotationToMovement = false;
        MoveComp->RotationRate = FRotator(0.f, 500.f, 0.f);
        // 중력, 지상 이동 등 기본값은 CharacterMovement가 처리
    }

    // 기본 스탯 초기화
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

    MaxHP = 100.0f;
    HP = MaxHP;

    // 스킬1(대쉬)
    CurrentSkillState = ESkillState::Normal;
    DashDuration = 10.0f;
    DashTimer = 0.0f;
    DashCooldown = 10.0f;
    DashCooldownRemaining = 0.0f;

    // 스킬2(메테오) 기본값
    MeteorState = EMeteorState::None;
    MeteorTargetHeight = 1200.f;
    MeteorAscendSpeed = 1800.f;
    MeteorFallSpeed = 3000.f;
    MeteorAimMaxDistance = 10000.f;
    MeteorAOESphereLifeSeconds = 2.0f;
    MeteorSavedGravityScale = 1.0f;
    bMeteorAimValid = false;
    MeteorAimLocation = FVector::ZeroVector;

    // 커서 데칼 컴포넌트
    MeteorCursorDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("MeteorCursorDecal"));
    MeteorCursorDecal->SetupAttachment(GetRootComponent());
    MeteorCursorDecal->SetHiddenInGame(true);             // 조준 상태에서만 보이도록
    MeteorCursorDecal->DecalSize = FVector(256.f, 128.f, 128.f); // X=깊이
}

// ---------------------------------------------
// BeginPlay
// ---------------------------------------------
void AHERO_Character::BeginPlay()
{
    Super::BeginPlay();

    // 레벨 기반 이동 스탯 갱신
    ApplyLevelStats();

    if (CURRENT_V > MAX_V)
    {
        CURRENT_V = MAX_V;
    }

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
                    Subsystem->AddMappingContext(IMC_HERO, 0);
                }
            }
        }
    }
}

// ---------------------------------------------
// Tick
// ---------------------------------------------
void AHERO_Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    HandleCooldowns(DeltaSeconds);

    // 스킬2 상태 우선 처리
    if (MeteorState == EMeteorState::Ascending || MeteorState == EMeteorState::Aiming)
    {
        TickMeteor(DeltaSeconds);
    }
    else if (CurrentSkillState == ESkillState::Dashing)
    {
        HandleDash(DeltaSeconds);
    }
    else
    {
        HandleMovement(DeltaSeconds);
    }

    // 이동 속도 반영
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = CURRENT_V;
    }
}

// ---------------------------------------------
// 입력 바인딩
// ---------------------------------------------
void AHERO_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_HERO_Look)
        {
            EI->BindAction(IA_HERO_Look, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Look);
        }

        if (IA_HERO_Accelerate)
        {
            EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Triggered, this, &AHERO_Character::Input_Accelerate);
            EI->BindAction(IA_HERO_Accelerate, ETriggerEvent::Completed, this, &AHERO_Character::Input_Accelerate);
        }

        if (IA_HERO_DashSkill)
        {
            EI->BindAction(IA_HERO_DashSkill, ETriggerEvent::Started, this, &AHERO_Character::Input_DashSkill);
        }

        if (IA_HERO_MeteorStrike)
        {
            EI->BindAction(IA_HERO_MeteorStrike, ETriggerEvent::Started, this, &AHERO_Character::Input_MeteorStrike);
        }
    }
}

// ---------------------------------------------
// 시간 경과 처리(쿨다운, 대쉬 타이머)
// ---------------------------------------------
void AHERO_Character::HandleCooldowns(float DeltaSeconds)
{
    if (DashCooldownRemaining > 0.0f)
    {
        DashCooldownRemaining -= DeltaSeconds;
        if (DashCooldownRemaining < 0.0f)
        {
            DashCooldownRemaining = 0.0f;
        }
    }

    if (CurrentSkillState == ESkillState::Dashing)
    {
        DashTimer -= DeltaSeconds;
        if (DashTimer <= 0.0f)
        {
            CurrentSkillState = ESkillState::Normal;
            DashTimer = 0.0f;
        }
    }
}

// ---------------------------------------------
// 대쉬 이동(스킬1)
// ---------------------------------------------
void AHERO_Character::HandleDash(float /*DeltaSeconds*/)
{
    CURRENT_V = MAX_V;
    const FVector ForwardDir = GetActorForwardVector();
    AddMovementInput(ForwardDir, 1.0f);
}

// ---------------------------------------------
// 일반 이동(가속/감속)
// ---------------------------------------------
void AHERO_Character::HandleMovement(float DeltaSeconds)
{
    if (bIsAccelerating)
    {
        CURRENT_V += PLUS_V * DeltaSeconds;
    }
    else
    {
        CURRENT_V -= MIUS_V * DeltaSeconds;
    }

    CURRENT_V = FMath::Clamp(CURRENT_V, 0.0f, MAX_V);

    if (CURRENT_V > 0.0f)
    {
        const FVector ForwardDir = GetActorForwardVector();
        AddMovementInput(ForwardDir, 1.0f);
    }
}

// ---------------------------------------------
// 레벨 스탯 재계산
// ---------------------------------------------
void AHERO_Character::ApplyLevelStats()
{
    MAX_V = BASE_MAX_V + INC_MAX_V_PER_LVL * (static_cast<float>(Level) - 1.0f);
    PLUS_V = BASE_PLUS_V + INC_PLUS_V_PER_LVL * (static_cast<float>(Level) - 1.0f);
}

// ---------------------------------------------
// 내부 레벨업 처리
// ---------------------------------------------
void AHERO_Character::LevelUpInternal()
{
    const int32 OldLevel = Level;
    const float OldHP = HP;

    Level += 1;

    ApplyLevelStats();

    HP = MaxHP;

    if (CURRENT_V > MAX_V)
    {
        CURRENT_V = MAX_V;
    }

    OnHeroLevelUp.Broadcast(OldLevel, Level);

    if (!FMath::IsNearlyEqual(OldHP, HP))
    {
        OnHPChanged.Broadcast(OldHP, HP, HP - OldHP);
    }
}

// ---------------------------------------------
// HP 공개 함수
// ---------------------------------------------
void AHERO_Character::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.f) return;

    const float OldHPLocal = HP;
    HP = FMath::Clamp(HP - DamageAmount, 0.f, MaxHP);

    if (!FMath::IsNearlyEqual(OldHPLocal, HP))
    {
        OnHPChanged.Broadcast(OldHPLocal, HP, HP - OldHPLocal);

        if (OldHPLocal > 0.f && HP <= 0.f)
        {
            OnHeroDeath.Broadcast();
        }
    }
}

void AHERO_Character::Heal(float HealAmount)
{
    if (HealAmount <= 0.f) return;

    const float OldHPLocal = HP;
    HP = FMath::Clamp(HP + HealAmount, 0.f, MaxHP);

    if (!FMath::IsNearlyEqual(OldHPLocal, HP))
    {
        OnHPChanged.Broadcast(OldHPLocal, HP, HP - OldHPLocal);
    }
}

// ---------------------------------------------
// 레벨업 공개 함수
// ---------------------------------------------
void AHERO_Character::ForceLevelUp()
{
    LevelUpInternal();
}

// ---------------------------------------------
// 입력 콜백
// ---------------------------------------------
void AHERO_Character::Input_Accelerate(const FInputActionValue& Value)
{
    const bool bPressed = Value.Get<bool>();
    bIsAccelerating = bPressed;
}

void AHERO_Character::Input_Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

// 스킬1: 대쉬 상태 토글
void AHERO_Character::Input_DashSkill(const FInputActionValue& /*Value*/)
{
    if (MeteorState != EMeteorState::None)
    {
        // 메테오 중에는 스킬1 입력 무시
        return;
    }

    if (CurrentSkillState == ESkillState::Normal && DashCooldownRemaining <= 0.0f)
    {
        CurrentSkillState = ESkillState::AimingDash;
        return;
    }

    if (CurrentSkillState == ESkillState::AimingDash)
    {
        CurrentSkillState = ESkillState::Dashing;
        DashTimer = DashDuration;
        DashCooldownRemaining = DashCooldown;
        CURRENT_V = MAX_V;
        return;
    }
}

// 스킬2: 메테오
void AHERO_Character::Input_MeteorStrike(const FInputActionValue& /*Value*/)
{
    if (MeteorState == EMeteorState::Descending)
        return;

    if (MeteorState == EMeteorState::None)
    {
        BeginMeteorAscend();
        return;
    }

    if (MeteorState == EMeteorState::Aiming)
    {
        CommitMeteorStrike();
        return;
    }
}

// ---------------------------------------------
// 스킬2: 상승 시작
// ---------------------------------------------
void AHERO_Character::BeginMeteorAscend()
{
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        MeteorStartZ = GetActorLocation().Z;
        MeteorTargetZ = MeteorStartZ + MeteorTargetHeight;

        MeteorSavedGravityScale = Move->GravityScale;
        Move->GravityScale = 0.f;
        Move->StopMovementImmediately();
        Move->Velocity = FVector::ZeroVector;
    }

    MeteorState = EMeteorState::Ascending;

    if (MeteorCursorDecal)
        MeteorCursorDecal->SetHiddenInGame(true);
}

// ---------------------------------------------
// 스킬2: 틱 처리(상승/조준)
// ---------------------------------------------
void AHERO_Character::TickMeteor(float DeltaSeconds)
{
    if (MeteorState == EMeteorState::Ascending)
    {
        FVector P = GetActorLocation();
        P.Z += MeteorAscendSpeed * DeltaSeconds;

        if (P.Z >= MeteorTargetZ)
        {
            P.Z = MeteorTargetZ;
            SetActorLocation(P);
            BeginMeteorAiming();
        }
        else
        {
            SetActorLocation(P);
        }
        return;
    }

    if (MeteorState == EMeteorState::Aiming)
    {
        FVector P = GetActorLocation();
        P.Z = MeteorTargetZ;
        SetActorLocation(P);

        UpdateMeteorCursor();
    }
}

// ---------------------------------------------
// 스킬2: 조준 상태 진입
// ---------------------------------------------
void AHERO_Character::BeginMeteorAiming()
{
    MeteorState = EMeteorState::Aiming;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->GravityScale = 0.f;
        Move->StopMovementImmediately();
        Move->Velocity = FVector::ZeroVector;
    }

    if (MeteorCursorDecal)
        MeteorCursorDecal->SetHiddenInGame(false);
}

// ---------------------------------------------
// 스킬2: 커서 위치 갱신(카메라 전방 레이)
// ---------------------------------------------
void AHERO_Character::UpdateMeteorCursor()
{
    bMeteorAimValid = false;

    if (!CameraComp) return;

    const FVector Start = CameraComp->GetComponentLocation();
    const FVector Dir = CameraComp->GetForwardVector();
    const FVector End = Start + Dir * MeteorAimMaxDistance;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MeteorTrace), false, this);

    const ECollisionChannel Channel = ECC_Visibility;

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Channel, Params);

    // 필요시 주석 해제
    // DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.02f, 0, 1.f);

    if (bHit)
    {
        bMeteorAimValid = true;
        MeteorAimLocation = Hit.Location;

        if (MeteorCursorDecal)
        {
            const FRotator DecalRot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();
            const FVector  DecalPos = Hit.Location + Hit.Normal * 2.f;

            MeteorCursorDecal->SetWorldLocationAndRotation(DecalPos, DecalRot);
        }
    }
}

// ---------------------------------------------
// 스킬2: 커밋(목표 XY로 이동 후 낙하 시작)
// ---------------------------------------------
void AHERO_Character::CommitMeteorStrike()
{
    FVector Target = GetActorLocation();

    if (bMeteorAimValid)
    {
        Target.X = MeteorAimLocation.X;
        Target.Y = MeteorAimLocation.Y;
    }
    Target.Z = MeteorTargetZ;

    SetActorLocation(Target, false);

    if (MeteorCursorDecal)
        MeteorCursorDecal->SetHiddenInGame(true);

    MeteorState = EMeteorState::Descending;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->GravityScale = MeteorSavedGravityScale;
        Move->Velocity = FVector(0.f, 0.f, -MeteorFallSpeed);
    }
}

// ---------------------------------------------
// 착지 처리(메테오 낙하 종료 지점)
// ---------------------------------------------
void AHERO_Character::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    if (MeteorState == EMeteorState::Descending)
    {
        MeteorState = EMeteorState::None;

        if (MeteorCursorDecal)
            MeteorCursorDecal->SetHiddenInGame(true);

        if (MeteorAOESphereClass)
        {
            FActorSpawnParameters SP;
            AActor* AOE = GetWorld()->SpawnActor<AActor>(MeteorAOESphereClass, Hit.ImpactPoint, FRotator::ZeroRotator, SP);
            if (AOE && MeteorAOESphereLifeSeconds > 0.f)
            {
                AOE->SetLifeSpan(MeteorAOESphereLifeSeconds);
            }
        }
    }
}