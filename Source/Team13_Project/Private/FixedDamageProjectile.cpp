#include "FixedDamageProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CombatComponent.h"

#include "Kismet/GameplayStatics.h"


#include "HitDamageable.h"



AFixedDamageProjectile::AFixedDamageProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

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



void AFixedDamageProjectile::BeginPlay()
{
    Super::BeginPlay();   

    if (LifeSeconds > 0.f)
        SetLifeSpan(LifeSeconds);

    

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
{

    if (ProjectileMovement)
    {
        const FVector Forward = GetActorForwardVector();
        ProjectileMovement->Velocity = Forward * ProjectileMovement->InitialSpeed;
    }
}


void AFixedDamageProjectile::OnBeginOverlap( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult)
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
    const FVector Dir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
    Combat->ApplyFixedDamage(Target, Damage, Dir);

    if (bDestroyOnHit)
        Destroy();
}

bool AFixedDamageProjectile::IsFriendlyToMe(AActor* Other) const
{
    const bool bIsPlayer = Other->ActorHasTag(TEXT("Player"));
    const bool bIsMonster = Other->ActorHasTag(TEXT("Monster"));

    if (Team == ETeam::Player && bIsPlayer) return true;
    if (Team == ETeam::Monster && bIsMonster) return true;

    return false;
}

UCombatComponent* AFixedDamageProjectile::ResolveCombatComponent() const
{

    
    if (SourceCombat)
        return SourceCombat;

    APawn* InstigatorPawn = GetInstigator();
    if (!InstigatorPawn)
        return nullptr;

    return Cast<UCombatComponent>(InstigatorPawn->GetComponentByClass(UCombatComponent::StaticClass()));
}
