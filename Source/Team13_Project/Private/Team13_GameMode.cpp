// Fill out your copyright notice in the Description page of Project Settings.


#include "Team13_GameMode.h"
#include "Team13_GameState.h"
#include "Team13_PlayerController.h"

ATeam13_GameMode::ATeam13_GameMode()
{
	GameStateClass = ATeam13_GameState::StaticClass();
	PlayerControllerClass = ATeam13_PlayerController::StaticClass();
}
