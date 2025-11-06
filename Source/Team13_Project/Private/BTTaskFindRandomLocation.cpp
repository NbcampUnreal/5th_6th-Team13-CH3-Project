// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskFindRandomLocation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "TestAIController.h"

UBTTaskFindRandomLocation::UBTTaskFindRandomLocation()
{
	NodeName = TEXT("Find Random Location");
	LocationKey.AddVectorFilter(this,
		GET_MEMBER_NAME_CHECKED(UBTTaskFindRandomLocation,LocationKey));
	
}

EBTNodeResult::Type UBTTaskFindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1단계: 필요한 것들 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
    
	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return EBTNodeResult::Failed;
    
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;
    
	// 2단계: 랜덤 위치 찾기
	FNavLocation RandomLocation;
	bool bFound = NavSystem->GetRandomReachablePointInRadius(
		MyPawn->GetActorLocation(),  // 내 위치를 중심으로
		SearchRadius,                // 이 반경 안에서
		RandomLocation               // 결과를 여기 저장
	);
    
	// 3단계: 찾았으면 Blackboard에 저장
	if (bFound)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsVector(LocationKey.SelectedKeyName, RandomLocation.Location);
			UE_LOG(LogTemp, Log, TEXT("[FindRandom] 새로운 목적지: %s"), *RandomLocation.Location.ToString());
			return EBTNodeResult::Succeeded;  // 성공ㅋㅋ
		}
	}
    
	UE_LOG(LogTemp, Warning, TEXT("[FindRandom] 갈 곳을 찾지 못했습니다"));
	return EBTNodeResult::Failed;  // 실패
}