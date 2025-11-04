// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameState.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "AiTestMonster.h"
#include "ObjectPoolManager.h"

ATeam13_GameState::ATeam13_GameState()
{
}

void ATeam13_GameState::BeginPlay()
{
	Super::BeginPlay();

    UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();
    
    if (PoolManager)
    {
        PoolManager->InitializePool(AAiTestMonster::StaticClass(), 50);
    }
}