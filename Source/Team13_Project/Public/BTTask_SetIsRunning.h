// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetIsRunning.generated.h"

/**
 * 
 */
UCLASS()
class TEAM13_PROJECT_API UBTTask_SetIsRunning : public UBTTaskNode
{
	GENERATED_BODY()

	public:
	UBTTask_SetIsRunning();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
