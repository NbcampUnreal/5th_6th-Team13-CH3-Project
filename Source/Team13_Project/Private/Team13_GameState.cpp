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

ATeam13_GameState::ATeam13_GameState()
{
	StageDuration = 1000.f;
	CurrentStageIndex = 0;
	MaxStageIndex = 2;
}

void ATeam13_GameState::BeginPlay()
{
	Super::BeginPlay();


	StartStage();

	HERO_Character = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (HERO_Character)
	{
		HERO_Character->OnHeroDeath.AddDynamic(this, &ATeam13_GameState::OnGameOver);
	}

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ATeam13_GameState::UpdateHUD,
		0.1f,
		true);
}

void ATeam13_GameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ATeam13_GameState::StartStage()
{
	UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();

	if (PoolManager)
	{
		PoolManager->InitializePool(AAiTestMonster::StaticClass(), 10);
	}

	//HERO_Character = Cast<AHERO_Character>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			Team13_PlayerController->ShowGameHUD();
		}
	}
	
	if(UGameInstance * GameInstance = GetGameInstance())
	{
		UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(GameInstance);
		if (Team13_GameInstance)
		{
			CurrentStageIndex = Team13_GameInstance->CurrentStageIndex;
		}
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
	EndStage();
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
			CurrentStageIndex++;
			Team13_GameInstance->CurrentStageIndex = CurrentStageIndex;

			if (CurrentStageIndex >= MaxStageIndex)
			{
				OnGameOver();
				return;
			}

			if (StageMapNames.IsValidIndex(CurrentStageIndex))
			{
				UGameplayStatics::OpenLevel(GetWorld(), StageMapNames[CurrentStageIndex]);
			}
			else
			{
				OnGameOver();
			}
		}
	}
}

//게임 종료
void ATeam13_GameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			Team13_PlayerController->SetPause(true);
			//Team13_PlayerController->ShowEndMenu(true); //임시
			/*if (HERO_Character->IsDead() ||
				(StageDuration < 0 && HERO_Character->GetHeroLevel() < 5))
			{
				Team13_PlayerController->ShowEndMenu(true);
			}
			else 
			{
				Team13_PlayerController->ShowEndMenu(false);
			}*/
			if (HERO_Character->IsDead())
			{
				UE_LOG(LogTemp, Error, TEXT("Death"));
				Team13_PlayerController->ShowEndMenu(true);
			}
			else 
			{
				UE_LOG(LogTemp, Error, TEXT("Failed Death"));
			}

		}
	}
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
							TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time : %0.f"), RemainingTime)));
						}

						//Exp Text
						if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("ExpText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Team13_GameInstance->CurrentExp, Team13_GameInstance->MaxExp)));
						}
						
						//Stage Text
						if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("StageText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("Stagt : %d"), CurrentStageIndex + 1)));
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
								LevelText->SetText(FText::FromString(FString::Printf(TEXT("%0.f"), HeroCharacter->CURRENT_V)));
							}

							//HP Text
							if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("HPText"))))
							{
								ExpText->SetText(FText::FromString(FString::Printf(TEXT("%0.f"), HeroCharacter->HP)));
							}

							//PlayerLevel Text
							if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("LevelText"))))
							{
								LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level : %d"), HeroCharacter->Level)));
							}
						}
					}
				}
			}
		}
	}
}

