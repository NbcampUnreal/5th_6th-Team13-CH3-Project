// Fill out your copyright notice in the Description page of Project Settings.


#include "AiTestMonster.h"
#include "TestAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAiTestMonster::AAiTestMonster()
{
	AIControllerClass = ATestAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	PrimaryActorTick.bCanEverTick = true;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void AAiTestMonster::SetMovementSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = NewSpeed;
		UE_LOG(LogTemp, Warning, TEXT("[Sparta] Speed changed: %.1f"), NewSpeed);
	}
}

void AAiTestMonster::BeginPlay()
{
	Super::BeginPlay();
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = WalkSpeed;
	UE_LOG(LogTemp, Warning, TEXT("AI Test Spawned"));
}

void AAiTestMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon)
	{	UE_LOG(LogTemp, Warning, TEXT("AI Controller missing"));
		return;
	}
	UBlackboardComponent* BB = AICon->GetBlackboardComponent();
	if (!BB)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI BlackBoard missing"));
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("AI Control/BlackBoard allright"));
	bool bIsRunning = BB->GetValueAsBool(TEXT("IsRunning"));

	float TargetSpeed = bIsRunning ? 600.f : 300.f;
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp && MoveComp->MaxWalkSpeed != TargetSpeed)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AI Speed Set"));
		MoveComp->MaxWalkSpeed = TargetSpeed;
	}
}