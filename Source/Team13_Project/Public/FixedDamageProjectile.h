
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HitDamageable.h"
#include "FixedDamageProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UCombatComponent;

UENUM(BlueprintType)
enum class ETeam : uint8
{
    Player = 0,
    Monster = 1
};

UCLASS()
class TEAM13_PROJECT_API AFixedDamageProjectile : public AActor
{
    GENERATED_BODY()

public:
    AFixedDamageProjectile();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    // 공통
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Damage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    ETeam Team = ETeam::Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float LifeSeconds = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    bool bDestroyOnHit = true;

    // 이동 모드 스위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Mode")
    bool bUsePhysics = false; // false: ProjectileMovement, true: Physics

    // 모드 A 설정(ProjectileMovement)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Mode")
    float InitialSpeed = 2000.f;

    // 모드 B 설정(Physics)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Mode", meta = (EditCondition = "bUsePhysics"))
    float PhysicsImpulse = 3000.f; // 전방 임펄스

    // 발사자가 가진 CombatComponent 주입
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SetSourceCombat(UCombatComponent* InCombat) { SourceCombat = InCombat; }

    // 런타임 토글(선택): BeginPlay 전에 호출 권장
    UFUNCTION(BlueprintCallable, Category = "Projectile|Mode")
    void SetUsePhysics(bool bInUsePhysics) { bUsePhysics = bInUsePhysics; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void ApplyTravelVelocity();  // 모드별 초기 속도/임펄스
    void TryApplyDamageTo(AActor* OtherActor, const FHitResult& Hit);

    bool IsFriendlyToMe(AActor* Other) const;
    UCombatComponent* ResolveCombatComponent() const;

private:
    UPROPERTY()
    UCombatComponent* SourceCombat;
};
