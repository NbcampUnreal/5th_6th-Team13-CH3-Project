// Copyright Epic Games, Inc. All Rights Reserved.

#include "Team13_ProjectGameMode.h"
#include "Team13_ProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATeam13_ProjectGameMode::ATeam13_ProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
