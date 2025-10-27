// Fill out your copyright notice in the Description page of Project Settings.


#include "AiTestMonster.h"
#include "TestAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AAiTestMonster::AAiTestMonster()
{
	AIControllerClass = ATestAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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
