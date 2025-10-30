// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SetIsRunning.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetIsRunning::UBTTask_SetIsRunning()
{
	NodeName = TEXT("Switch Running/Walk");
}

EBTNodeResult::Type UBTTask_SetIsRunning::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp != nullptr)
	{
		bool bCurruntRunning = BlackboardComp->GetValueAsBool("IsRunning");
		BlackboardComp->SetValueAsBool("IsRunning",!bCurruntRunning);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
