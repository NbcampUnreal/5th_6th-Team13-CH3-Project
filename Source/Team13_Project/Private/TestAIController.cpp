#include "TestAIController.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"


ATestAIController::ATestAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(1.0f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	

	// Blackboard Component 생성
	// 이건 실제 데이터를 담는 실행용 컨테이너 (게임 도중 키/값을 저장)
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard"));

}


void ATestAIController::StartBehaviorTree()
{
	if (BehaviorTreeAsset)
	{
		// Behavior Tree 실행 시작
		RunBehaviorTree(BehaviorTreeAsset);
		
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Behavior Tree started"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Sparta] Behavior Tree Asset not set!"));
	}
}

void ATestAIController::BeginPlay()
{
	Super::BeginPlay();

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ATestAIController::OnPerceptionUpdated);
	if (BlackboardComp)
	{
		// 초기값 설정 – 시작할 때 Blackboard에 값 미리 넣어둠
		// BT에서 이 값들을 조건 판단에 사용할 수 있음
		BlackboardComp->SetValueAsBool(TEXT("CanSeeTarget"), false); // 타겟 탐지 여부 초기화
		BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false); // 조사 중 상태 초기화
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Blackboard initialized successfully"));
		StartBehaviorTree();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Sparta] Blackboard Component not found!"));
	}
	
	
}

void ATestAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Test] AI Controller is controlling %s."), *InPawn->GetName());
	}
}



void ATestAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Actor != PlayerPawn || !BlackboardComp) 
	{
		return;
	}
    
	if (Stimulus.WasSuccessfullySensed())
	{
		// Blackboard에 정보 저장
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
		BlackboardComp->SetValueAsBool(TEXT("CanSeeTarget"), true);
		BlackboardComp->SetValueAsVector(TEXT("TargetLastKnownLocation"), Actor->GetActorLocation());
		BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false);
		UE_LOG(LogTemp, Warning, TEXT("[AI] Target Found! %s"),
			BlackboardComp->GetValueAsBool("CanSeeTarget") ? TEXT("true"): TEXT("false"));
	}
	else
	{
		// 더 이상 보지 못함
		BlackboardComp->SetValueAsBool(TEXT("CanSeeTarget"), false);
		// 조사 모드 시작
		BlackboardComp->SetValueAsVector(TEXT("TargetLastKnownLocation"), Actor->GetActorLocation());
		BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), true);
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), nullptr);
		UE_LOG(LogTemp, Warning, TEXT("[AI] Target Lost! %s"),
			BlackboardComp->GetValueAsBool("CanSeeTarget") ? TEXT("true"): TEXT("false"));
	}
}
