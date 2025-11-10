#include "Team13_GameInstance.h"
#include "Team13_GameMode.h"

UTeam13_GameInstance::UTeam13_GameInstance()
{
	CurrentStageIndex = 0;
	CurrentKill = 0;
	Score = 0;
}

void UTeam13_GameInstance::AddScore(int32 Amount)
{
	Score += Amount;
}

void UTeam13_GameInstance::AddToKill()
{
	++CurrentKill;
}

//Instance 내용 초기화
void UTeam13_GameInstance::clear()
{
	CurrentKill = 0;
	Score = 0;
	CurrentStageIndex = 0;
}
