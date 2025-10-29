#include "BTTask_RunAway.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RunAway::UBTTask_RunAway()
{
    NodeName = TEXT("Set Safe Escape Location + Prefer Exact Opposite");
}

EBTNodeResult::Type UBTTask_RunAway::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    ACharacter* AIPawn = Cast<ACharacter>(AICon ? AICon->GetPawn() : nullptr);
    if (!AICon || !AIPawn) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
    if (!Target) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AIPawn);
    if (!NavSys) return EBTNodeResult::Failed;

    FVector Start = AIPawn->GetActorLocation();
    FVector Dir = (Start - Target->GetActorLocation()).GetSafeNormal();

    const float RunDistance = 800.f;
    const float CheckAngleStep = 30.f;
    const int32 NumChecks = 6;

    FVector BestLoc = Start;
    bool bFound = false;
    float BestDist = 0.f;

    auto TestLocation = [&](const FVector& Candidate) -> bool
    {
        FNavLocation NavLoc;
        if (!NavSys->ProjectPointToNavigation(Candidate, NavLoc, FVector(200, 200, 200)))
            return false;

        UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(AICon->GetWorld(), Start, NavLoc.Location);
        if (Path && Path->IsValid() && Path->PathPoints.Num() > 1)
        {
            float PathLen = Path->GetPathLength();
            if (PathLen > BestDist)
            {
                BestDist = PathLen;
                BestLoc = NavLoc.Location;
                return true;
            }
        }
        return false;
    };
    
    FVector StraightBack = Start + Dir * RunDistance;
    if (TestLocation(StraightBack))
    {
        bFound = true;
    }
    else
    {
        
        for (int i = 1; i <= NumChecks / 2; i++)
        {
            float Angle = i * CheckAngleStep;
            FVector LeftDir = Dir.RotateAngleAxis(Angle, FVector::UpVector);
            FVector RightDir = Dir.RotateAngleAxis(-Angle, FVector::UpVector);

            if (TestLocation(Start + LeftDir * RunDistance))
            {
                bFound = true;
                break;
            }
            if (TestLocation(Start + RightDir * RunDistance))
            {
                bFound = true;
                break;
            }
        }
    }

    if (bFound)
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("EscapeLocation"), BestLoc);

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

