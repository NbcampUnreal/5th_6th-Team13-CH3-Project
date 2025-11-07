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
    Collision->OnComponentHit.AddDynamic(this, &AFixedDamageProjectile::OnHit);
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
    if (ProjectileMovement)
    {
        const FVector Dir = InitialDirection.IsNearlyZero()
            ? GetActorForwardVector()
            : InitialDirection.GetSafeNormal();
        ProjectileMovement->Velocity = Dir * ProjectileMovement->InitialSpeed;
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


void AFixedDamageProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
   
    OnBeginOverlap(HitComp, OtherActor, OtherComp, 0, false, Hit);
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

    // 가해 방향 계산
    const FVector Dir = bUsePhysics  ? (Collision->GetComponentVelocity().GetSafeNormal().IsNearlyZero() ? GetActorForwardVector()  : Collision->GetComponentVelocity().GetSafeNormal())
        : (ProjectileMovement
            ? ProjectileMovement->Velocity.GetSafeNormal()
            : GetActorForwardVector());

    // 1) 데미지 1회
    Combat->ApplyFixedDamage(Target, Damage, Dir);

    // 2) 피드백 1회
    AActor* AttackerActor = GetInstigator() ? static_cast<AActor*>(GetInstigator()) : this;
    Combat->ApplyCollisionFeedbackForDefender(Target, AttackerActor, SweepResult);

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
