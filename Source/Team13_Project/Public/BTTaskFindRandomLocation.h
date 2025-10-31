// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskFindRandomLocation.generated.h"

/**
 * 
 */
UCLASS()
class TEAM13_PROJECT_API UBTTaskFindRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskFindRandomLocation();
protected:
	// 이 Task가 실행될 때 호출되는 핵심 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
	// 결과를 어떤 Blackboard 키에 저장할지
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector LocationKey;
    
	// 얼마나 멀리까지 찾을지 (반경)
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "100.0"))
	float SearchRadius = 1000.0f;
};
