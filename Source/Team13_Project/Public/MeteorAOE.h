#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeteorAOE.generated.h"

class USphereComponent;
class UCombatComponent;

UCLASS()
class TEAM13_PROJECT_API AMeteorAOE : public AActor
{
    GENERATED_BODY()

public:
    AMeteorAOE();

    
    void SetSourceCombat(UCombatComponent* InCombat) { SourceCombat = InCombat; }
    void SetOwnerActor(AActor* InOwner) { OwnerActor = InOwner; }

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
    float Radius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
    float LifeSeconds = 0.15f; 

    UFUNCTION(BlueprintCallable, Category = "AOE")
    void Trigger();
protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "AOE")
    USphereComponent* Sphere;

    UPROPERTY()
    UCombatComponent* SourceCombat = nullptr;

    UPROPERTY()
    AActor* OwnerActor = nullptr;

    void DoDamageSweep();
};
