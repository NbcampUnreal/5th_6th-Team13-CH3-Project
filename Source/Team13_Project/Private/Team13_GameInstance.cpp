#include "Team13_GameInstance.h"

UTeam13_GameInstance::UTeam13_GameInstance()
{
	CurrentStageIndex = 0;

	//�ӽ�
	CurrentExp = 0;
	MaxExp = 100;
	CurrentLevel = 1;
	CurrentKill = 0;

}

////EXP�� ��� 100�� ������ Levelup
//void UTeam13_GameInstance::AddToEXP(int32 Amount)
//{
//	CurrentExp += Amount;
//	if (CurrentExp >= MaxExp)
//	{
//		CurrentLevel++;
//		CurrentExp -= MaxExp;
//	}
//}
//
//void UTeam13_GameInstance::AddToKill()
//{
//	if (���Ͱ� ����� ��)
//	{
//		++CurrentKill;
//	}
//}
