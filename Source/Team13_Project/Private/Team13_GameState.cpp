#include "Team13_GameState.h"
#include "Team13_PlayerController.h"
#include "Team13_GameInstance.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

ATeam13_GameState::ATeam13_GameState()
{
	StageDuration = 10.f;
	CurrentStageIndex = 0;
	MaxStageIndex = 2;
}

void ATeam13_GameState::BeginPlay()
{
	Super::BeginPlay();

	/*TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnEnemy::StaticClass(), FoundVolumes);

	int32 BaseSpawnCount = 40;
	float SpawnIncreaseRate = 0.2f;

	EnemyToSpawn = BaseSpawnCount + (BaseSpawnCount * SpawnIncreaseRate);

	for (int32 i = 0; i < BaseSpawnCount; i++)
	{
		if (FoundVolumes.Num() > 0)
		{
			ASpawnEnemy* SpawnEnemy = Cast<ASpawnEnemy>(FoundVolumes[0]);
			if (SpawnEnemy)
			{
				AActor* SpawnedActor = SpawnEnemy->SpawnRandomEnemy();
			}
		}
	}*/
	StartStage();

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ATeam13_GameState::UpdateHUD,
		0.1f,
		true);
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
			Team13_PlayerController->ShowEndMenu(true); //임시
			/*if (플레이어가 죽음 or 시간안에 레벨도달실패)
			{
				Team13_PlayerController->ShowEndMenu(true);
			}
			else (성공)
			{
				Team13_PlayerController->ShowEndMenu(false);
			}*/
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
						//PlayerLevel Text
						if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("LevelText"))))
						{
							LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level : %d"), Team13_GameInstance->CurrentLevel)));
						}
						
						//Timer Text
						if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("TimeText"))))
						{
							float RemainingTime = GetWorldTimerManager().GetTimerRemaining(StageTimerHandle);

							int32 DisplayTime = FMath::Max(0, FMath::CeilToInt(RemainingTime));

							TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time : %d"), DisplayTime)));
						}

						//HP Text
						/*if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("HPText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("%d"), )));
						}*/
						
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
						
						//Speed Text
						/*if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("SpeedText"))))
						{
							LevelText->SetText(FText::FromString(FString::Printf(TEXT("%d"), )));
						}*/
					}
				}
			}
		}
	}
}

