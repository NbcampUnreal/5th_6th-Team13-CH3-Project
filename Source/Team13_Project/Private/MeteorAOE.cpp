#include "MeteorAOE.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "CombatComponent.h"
#include "HitDamageable.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"  
#include "CollisionQueryParams.h" 
#include "CollisionShape.h"
#include "Kismet/KismetSystemLibrary.h"
AMeteorAOE::AMeteorAOE()
{
    PrimaryActorTick.bCanEverTick = false;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->InitSphereRadius(100.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionObjectType(ECC_WorldDynamic);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SetRootComponent(Sphere);
}

void AMeteorAOE::BeginPlay()
{
    Super::BeginPlay();

    

    Sphere->SetSphereRadius(Radius);

    // 자동 실행 제거
    if (LifeSeconds > 0.f)
        SetLifeSpan(LifeSeconds);
}
void AMeteorAOE::Trigger()
{
    DoDamageSweep();
}
void AMeteorAOE::DoDamageSweep()
{
    if (!SourceCombat) return;
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Center = GetActorLocation();

    
    TArray<AActor*> OverlappedActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    if (OwnerActor) ActorsToIgnore.Add(OwnerActor);

    const bool bAny = UKismetSystemLibrary::SphereOverlapActors(
        this,
        Center,
        Radius,
        ObjectTypes,
        /*ClassFilter=*/nullptr,
        ActorsToIgnore,
        OverlappedActors
    );

    if (!bAny) return;

    for (AActor* Other : OverlappedActors)
    {
        if (!Other) continue;

        TScriptInterface<IHitDamageable> Target(Other);
        if (!Target) continue;

        const FVector KnockDir = (Other->GetActorLocation() - Center).GetSafeNormal();

        
        SourceCombat->ApplyInstantKill(Target, KnockDir);

        
        FHitResult FakeHit;
        FakeHit.ImpactPoint = Center;
        FakeHit.ImpactNormal = KnockDir;
        FakeHit.Location = Other->GetActorLocation();

        SourceCombat->ApplyCollisionFeedbackForDefender(Target, OwnerActor ? OwnerActor : this, FakeHit);
    }
}