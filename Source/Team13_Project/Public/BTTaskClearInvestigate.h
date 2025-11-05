// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskClearInvestigate.generated.h"

/**
 * 
 */
UCLASS()
class TEAM13_PROJECT_API UBTTaskClearInvestigate : public UBTTaskNode
{
	GENERATED_BODY()

	public:
	UBTTaskClearInvestigate();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
