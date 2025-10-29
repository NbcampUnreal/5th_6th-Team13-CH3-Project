#include "Team13_GameState.h"
#include "Team13_PlayerController.h"
#include "Team13_GameInstance.h"
#include "SpawnEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"


ATeam13_GameState::ATeam13_GameState()
{
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
	UpdateHUD();
	StartLevel();
}



void ATeam13_GameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
		{
			Team13_PlayerController->ShowGameHUD();
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
						
						//HP Text
						/*if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("HPText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), )));
						}*/
						
						//Exp Text
						if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("ExpText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Team13_GameInstance->CurrentExp, Team13_GameInstance->MaxExp)));
						}
						
						//Stage Text
						/*if (UTextBlock* ExpText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("StageText"))))
						{
							ExpText->SetText(FText::FromString(FString::Printf(TEXT("Stagt : %d"), )));
						}*/
						
						//SpeedText
						/*if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("SpeedText"))))
						{
							LevelText->SetText(FText::FromString(FString::Printf(TEXT("%d"), )));
						}*/
					}
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
}
