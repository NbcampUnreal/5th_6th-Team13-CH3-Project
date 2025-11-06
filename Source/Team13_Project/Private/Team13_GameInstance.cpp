#include "Team13_GameInstance.h"
#include "Team13_GameMode.h"

UTeam13_GameInstance::UTeam13_GameInstance()
{
	CurrentStageIndex = 0;

	//임시
	CurrentExp = 0;
	MaxExp = 100;
	CurrentLevel = 1;
	CurrentKill = 0;

}

////EXP를 얻고 100이 넘으면 Levelup
/*void UTeam13_GameInstance::AddToEXP(int32 Amount)
{
	CurrentExp += Amount;
	if (CurrentExp >= MaxExp)
	{
		ATeam13_GameMode* GameMode = GetWorld()->GetAuthGameMode<ATeam13_GameMode>();
		if (GameMode)
		{
			
		}
		CurrentLevel++;
		CurrentExp -= MaxExp;
	}
}*/

void UTeam13_GameInstance::AddToKill()
{
	++CurrentKill;
}
