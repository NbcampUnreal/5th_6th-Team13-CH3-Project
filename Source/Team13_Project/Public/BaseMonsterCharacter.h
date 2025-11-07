#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HitDamageable.h"
#include "BaseMonsterCharacter.generated.h"

class UCombatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterDeath);

UCLASS()
class TEAM13_PROJECT_API ABaseMonsterCharacter : public ACharacter, public IHitDamageable
{
    GENERATED_BODY()

public:
    ABaseMonsterCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster") 
    int32 Level = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
    float SizeScale = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster") 
    float MaxHealth = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster") 
    float CurrentHealth = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster") 
    float Speed = 50.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster") 
    float exp = 1.f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombatComponent* CombatComp;
    
    
    UFUNCTION()
    void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    virtual float GetCurrentHealth() const override { return CurrentHealth; }
    virtual float GetMaxHealth() const override { return MaxHealth; }
    virtual void  SetCurrentHealth(float V) override { CurrentHealth = V; }

    virtual int32 GetLevel() const override { return Level; }
    virtual float GetSizeScale() const override { return SizeScale; }
    virtual float GetMaxSpeed() const override;
    virtual float GetCurrentSpeed() const override { return GetVelocity().Size(); }


    virtual bool  IsDead() const override { return CurrentHealth <= 0.f; }
    virtual void  OnDead() override;
    virtual void  EnableRagdollAndImpulse(const FVector& Impulse) override;

    void SyncSizeToScale();


protected:
    virtual void BeginPlay() override; 
};