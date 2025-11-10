#include "Team13_GameState.h"
#include "Team13_PlayerController.h"
#include "Team13_GameInstance.h"
#include "HERO_Character.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "ObjectPoolManager.h"
#include "AiTestMonster.h"
#include "Team13_GameMode.h"

ATeam13_GameState::ATeam13_GameState()
{
	StageDuration = 999.f;
	CurrentStageIndex = 0;
}

void ATeam13_GameState::BeginPlay()
{
	Super::BeginPlay();

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("Team13_SH_Map") || CurrentMapName.Contains("Team13_SH_Map_2"))
	{
		StartStage();
	}

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
				if (CurrentMapName.Contains("EndMenu"))
				{
					Team13_PlayerController->ShowEndMenu(Team13_GameInstance->bIsLose);
				}
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ATeam13_GameState::UpdateHUD,
		0.1f,
		true);

	HERO_Character = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (HERO_Character)
	{
		HERO_Character->OnHeroDeath.AddDynamic(this, &ATeam13_GameState::OnGameOver);
	}
}

void ATeam13_GameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
		if (Team13_GameInstance)
		{
			if (Team13_GameInstance->CurrentStageIndex == 0) {
				MainMenuStage();
			}
		}
	}
}

void ATeam13_GameState::StartStage()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			Team13_PlayerController->ShowGameHUD();
		}
	}

	if (ATeam13_GameMode* GameMode = GetWorld()->GetAuthGameMode<ATeam13_GameMode>())
	{
		GameMode->StartGameStage();
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (!CurrentMapName.Contains("StartMenu"))
	{
		GetWorldTimerManager().SetTimer(
			StageTimerHandle,
			this,
			&ATeam13_GameState::OnStageTimeUp,
			StageDuration,
			false);
	}

}

void ATeam13_GameState::OnStageTimeUp()
{
	OnGameOver();
}

//스테이지 종료
void ATeam13_GameState::EndStage()
{
	GetWorldTimerManager().ClearTimer(StageTimerHandle);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
		if (Team13_GameInstance)
		{
			++Team13_GameInstance->CurrentStageIndex;
			CurrentStageIndex = Team13_GameInstance->CurrentStageIndex;
			//if (CurrentStageIndex >= Team13_GameInstance->MaxStageIndex)
			//{
			//	OnGameOver();
			//	return;
			//}

			if (StageMapNames.IsValidIndex(Team13_GameInstance->CurrentStageIndex))
			{
				FTimerHandle TimerHandle;
				GetWorldTimerManager().SetTimer(TimerHandle, [this]() {
					UGameplayStatics::OpenLevel(GetWorld(), StageMapNames[CurrentStageIndex]);
					}, 1.0f, false);
			}
			else
			{
				OnGameOver();
			}
		}
	}
}

void ATeam13_GameState::MainMenuStage()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			UGameplayStatics::OpenLevel(GetWorld(), FName("StartMenu"));
			Team13_PlayerController->ShowStartMenu();
		}
	}
}

//게임 종료
void ATeam13_GameState::OnGameOver()
{
	if (bGameOverTriggered)
		return;

	bGameOverTriggered = true;

	if (HERO_Character)
	{
		HERO_Character->OnHeroDeath.RemoveDynamic(this, &ATeam13_GameState::OnGameOver);
	}

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
			{
				if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
						if (!Team13_GameInstance) return;

						Team13_PlayerController->SetPause(true);

						if (HERO_Character->GetHeroLevel() >= Team13_GameInstance->MaxLevels[CurrentStageIndex] )
							Team13_GameInstance->bIsLose = false;
						else
							Team13_GameInstance->bIsLose = true;

						UGameplayStatics::OpenLevel(GetWorld(), FName("EndMenu"));
					}
				}
			}
		}, 1.0f, false);
}


void ATeam13_GameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = Team13_PlayerController->GetHUDWidget())
			{
				if (UGameInstance* GameInstance = GetGameInstance())
				{
					UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
					if (Team13_GameInstance)
					{
						//Timer Text
						if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("TimeText"))))
						{
							float RemainingTime = GetWorldTimerManager().GetTimerRemaining(StageTimerHandle);
							TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time\n%0.f"), RemainingTime)));
						}

						//Stage Text
						if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("StageText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("Stage\n%d"), Team13_GameInstance->CurrentStageIndex + 1)));
						}

						//Kill Text
						if (UTextBlock* KillText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("KillText"))))
						{
							KillText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Team13_GameInstance->CurrentKill)));
						}
					}
					ACharacter* Character = PlayerController->GetCharacter();
					if (AHERO_Character* HeroCharacter = Cast<AHERO_Character>(Character))
					{
						if (HeroCharacter)
						{
							//Speed Text
							if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("SpeedText"))))
							{
								LevelText->SetText(FText::FromString(FString::Printf(TEXT("%0.f"), HeroCharacter->GetCurrentSpeed())));
							}

							//HP Text
							if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("HPText"))))
							{
								ExpText->SetText(FText::FromString(FString::Printf(TEXT("%0.f"), HeroCharacter->GetCurrentHealth())));
							}

							//PlayerLevel Text
							if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("LevelText"))))
							{
								LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level\n%d"), HeroCharacter->GetHeroLevel())));
							}

							//EXP text
							if (UTextBlock* EXPText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName("ExpText")))
							{
								EXPText->SetText(FText::FromString(FString::Printf(TEXT("%0.f / %0.f"), HeroCharacter->EXP, HeroCharacter->MAX_EXP)));
							}
						}
					}
				}
			}
		}
	}
}

