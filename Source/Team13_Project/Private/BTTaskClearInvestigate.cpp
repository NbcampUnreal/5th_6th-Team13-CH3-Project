#include "BTTaskClearInvestigate.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTaskClearInvestigate::UBTTaskClearInvestigate()
{
	NodeName = TEXT("Clear Investigating Flag");
	
}

EBTNodeResult::Type UBTTaskClearInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp != nullptr)
	{
		BlackboardComp->SetValueAsBool("IsInvestigating",false);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

