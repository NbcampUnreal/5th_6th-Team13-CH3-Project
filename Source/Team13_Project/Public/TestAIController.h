// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "TestAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class TEAM13_PROJECT_API ATestAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATestAIController();
	FORCEINLINE UBlackboardComponent* GetBlackboardComp() const 
	{ 
		return BlackboardComp; 
	}
	void StartBehaviorTree();
	
protected:
	// [Blackboard Component] : 실제 실행 중 데이터를 저장하는 "기억장치"
	// 실제 값들을 들고 있음 (Key에 해당하는 실시간 값 저장소)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* BlackboardComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree*  BehaviorTreeAsset;

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	UFUNCTION()

	
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn);
private:
	
};
