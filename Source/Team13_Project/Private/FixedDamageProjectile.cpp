#include "FixedDamageProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CombatComponent.h"
<<<<<<< Updated upstream
#include "Kismet/GameplayStatics.h"

=======
#include "HitDamageable.h"
#include "Kismet/GameplayStatics.h"


>>>>>>> Stashed changes
AFixedDamageProjectile::AFixedDamageProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

<<<<<<< Updated upstream
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    RootComponent = Collision;
    Collision->InitSphereRadius(10.f);
    // 기본은 쿼리 전용 프리셋(프로젝트의 "Projectile" 프리셋이 있다면 그걸로)
    Collision->SetCollisionProfileName(TEXT("Projectile"));
    Collision->SetGenerateOverlapEvents(true);
    Collision->SetNotifyRigidBodyCollision(false);

    Collision->OnComponentBeginOverlap.AddDynamic(this, &AFixedDamageProjectile::OnBeginOverlap);
    Collision->OnComponentHit.AddDynamic(this, &AFixedDamageProjectile::OnHit);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->InitialSpeed = InitialSpeed;
    ProjectileMovement->MaxSpeed = InitialSpeed;
}

=======
    
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    RootComponent = Collision;
    Collision->InitSphereRadius(10.f);
    Collision->SetCollisionProfileName(TEXT("Projectile"));
    Collision->SetGenerateOverlapEvents(true);
    Collision->OnComponentBeginOverlap.AddDynamic(this, &AFixedDamageProjectile::OnBeginOverlap);

    
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->InitialSpeed = 2000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
}


>>>>>>> Stashed changes
void AFixedDamageProjectile::BeginPlay()
{
    Super::BeginPlay();

<<<<<<< Updated upstream
=======
   
>>>>>>> Stashed changes
    if (LifeSeconds > 0.f)
        SetLifeSpan(LifeSeconds);

    
<<<<<<< Updated upstream
    if (bUsePhysics)
    {
        
        Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Collision->SetSimulatePhysics(true);
        Collision->SetNotifyRigidBodyCollision(true);
        
        if (ProjectileMovement) ProjectileMovement->Deactivate();
    }
    else
    {
        
        Collision->SetSimulatePhysics(false);
        Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Collision->SetNotifyRigidBodyCollision(false);
        if (ProjectileMovement)
        {
            ProjectileMovement->SetActive(true);
            ProjectileMovement->InitialSpeed = InitialSpeed;
            ProjectileMovement->MaxSpeed = InitialSpeed;
        }
    }

    ApplyTravelVelocity();
}

void AFixedDamageProjectile::ApplyTravelVelocity()
{
    const FVector Forward = GetActorForwardVector();

    if (bUsePhysics)
    {
        
        Collision->AddImpulse(Forward * PhysicsImpulse, NAME_None, true);
        
    }
    else
    {
        if (ProjectileMovement)
        {
            ProjectileMovement->Velocity = Forward * InitialSpeed;
        }
    }
}

void AFixedDamageProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    
    TryApplyDamageTo(OtherActor, SweepResult);
}

void AFixedDamageProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
   
    TryApplyDamageTo(OtherActor, Hit);
}

void AFixedDamageProjectile::TryApplyDamageTo(AActor* OtherActor, const FHitResult& Hit)
=======
    if (ProjectileMovement)
    {
        const FVector Forward = GetActorForwardVector();
        ProjectileMovement->Velocity = Forward * ProjectileMovement->InitialSpeed;
    }
}


void AFixedDamageProjectile::OnBeginOverlap( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult)
>>>>>>> Stashed changes
{
    if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
        return;

    if (IsFriendlyToMe(OtherActor))
        return;

    if (!OtherActor->GetClass()->ImplementsInterface(UHitDamageable::StaticClass()))
        return;

    UCombatComponent* Combat = ResolveCombatComponent();
    if (!Combat)
        return;

    TScriptInterface<IHitDamageable> Target;
    Target.SetObject(OtherActor);
    Target.SetInterface(Cast<IHitDamageable>(OtherActor));

<<<<<<< Updated upstream
   
    FVector Dir = FVector::ZeroVector;
    if (bUsePhysics)
    {
        Dir = Collision->GetComponentVelocity().GetSafeNormal();
        if (Dir.IsNearlyZero()) Dir = GetActorForwardVector();
    }
    else
    {
        Dir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
    }

    
    Combat->ApplyFixedDamage(Target, Damage, Dir);

    AActor* AttackerActor = GetInstigator() ? static_cast<AActor*>(GetInstigator()) : this;
    Combat->ApplyCollisionFeedbackForDefender(Target, AttackerActor, Hit);
=======
    const FVector Dir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
    Combat->ApplyFixedDamage(Target, Damage, Dir);

>>>>>>> Stashed changes
    if (bDestroyOnHit)
        Destroy();
}

bool AFixedDamageProjectile::IsFriendlyToMe(AActor* Other) const
{
<<<<<<< Updated upstream
    if (!Other) return false;
    const bool bOtherIsPlayer = Other->ActorHasTag(TEXT("Player"));
    const bool bOtherIsMonster = Other->ActorHasTag(TEXT("Monster"));
    if (Team == ETeam::Player && bOtherIsPlayer)  return true;
    if (Team == ETeam::Monster && bOtherIsMonster) return true;
=======
    if (!Other)
        return false;

    const bool bIsPlayer = Other->ActorHasTag(TEXT("Player"));
    const bool bIsMonster = Other->ActorHasTag(TEXT("Monster"));

    if (Team == ETeam::Player && bIsPlayer) return true;
    if (Team == ETeam::Monster && bIsMonster) return true;

>>>>>>> Stashed changes
    return false;
}

UCombatComponent* AFixedDamageProjectile::ResolveCombatComponent() const
{
<<<<<<< Updated upstream
    if (SourceCombat) return SourceCombat;
    if (APawn* InstigatorPawn = GetInstigator())
        return Cast<UCombatComponent>(InstigatorPawn->GetComponentByClass(UCombatComponent::StaticClass()));
    return nullptr;
=======
    if (SourceCombat)
        return SourceCombat;

    APawn* InstigatorPawn = GetInstigator();
    if (!InstigatorPawn)
        return nullptr;

    return Cast<UCombatComponent>(InstigatorPawn->GetComponentByClass(UCombatComponent::StaticClass()));
>>>>>>> Stashed changes
}
