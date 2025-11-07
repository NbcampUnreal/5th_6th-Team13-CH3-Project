#include "Team13_PlayerController.h"
#include "Team13_GameInstance.h"
#include "Team13_GameState.h"
#include "Team13_GameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ATeam13_PlayerController::ATeam13_PlayerController()
	: IMC_HERO(nullptr),
	IA_HERO_Look(nullptr),
	IA_HERO_Accelerate(nullptr),
	IA_HERO_DashSkill(nullptr),
	IA_HERO_MeteorStrike(nullptr),
	IA_HERO_Throw(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr),
	GuideWidgetClass(nullptr),
	GuideWidgetInstance(nullptr),
	SettingWidgetClass(nullptr),
	SettingWidgetInstance(nullptr),
	EndWidgetClass(nullptr),
	EndWidgetInstance(nullptr),
	CreditWidgetClass(nullptr),
	CreditWidgetInstance(nullptr)
{
}

void ATeam13_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	//StartMenu가 있다면 ShowStartMenu호출
	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("StartMenu"))
	{
		ShowStartMenu();
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_HERO)
			{
				Subsystem->AddMappingContext(IMC_HERO, 0);
			}
		}
	}
}

//게임 시작 메뉴
void ATeam13_PlayerController::ShowStartMenu()
{
	WidgetRemove();

	//메인메뉴 생성
	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//마우스 커서O
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void ATeam13_PlayerController::ShowGuideMenu()
{
	WidgetRemove();

	//가이드메뉴 생성
	if (GuideWidgetClass)
	{
		GuideWidgetInstance = CreateWidget<UUserWidget>(this, GuideWidgetClass);
		if (GuideWidgetInstance)
		{
			GuideWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//마우스 커서O
			SetInputMode(FInputModeUIOnly());
		}
	}
}

//셋팅메뉴
void ATeam13_PlayerController::ShowSettingMenu()
{
	WidgetRemove();

	//셋팅메뉴 생성
	if (SettingWidgetClass)
	{
		SettingWidgetInstance = CreateWidget<UUserWidget>(this, SettingWidgetClass);
		if (SettingWidgetInstance)
		{
			SettingWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//마우스 커서O
			SetInputMode(FInputModeUIOnly());
		}
	}
}

//엔딩메뉴
//게임이 끝날 때 호출해서 사용
void ATeam13_PlayerController::ShowEndMenu(bool bIsReStart)
{
	WidgetRemove();

	//엔드메뉴 생성
	if (EndWidgetClass)
	{
		EndWidgetInstance = CreateWidget<UUserWidget>(this, EndWidgetClass);
		if (EndWidgetInstance)
		{
			EndWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//마우스 커서O
			SetInputMode(FInputModeUIOnly());
		}
	}

	if (EndWidgetInstance)
	{
		//Quit버튼 Text
		if (UTextBlock* QuitButton = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("QuitButtonText"))))
		{
			QuitButton->SetText(FText::FromString(TEXT("Quit")));
		}

		//Main Menu버튼 Text
		if (UTextBlock* ReStartButton = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("ReStartButtonText"))))
		{
			ReStartButton->SetText(FText::FromString(TEXT("Main Menu")));
		}

		//Credit버튼 Text
		if (UTextBlock* CreditButton = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("CreditButtonText"))))
		{
			CreditButton->SetText(FText::FromString(TEXT("Credit")));
		}

		//Result Text (true or false)
		if (UTextBlock* ResultText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("ResultText"))))
		{
			ResultText->SetText(FText::FromString(bIsReStart ? TEXT("Loser") : TEXT("Winner")));
		}

		//킬 text
		if (UTextBlock* TotalKillText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName("KillText")))
		{
			if (UTeam13_GameInstance* GameInstance = Cast<UTeam13_GameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				TotalKillText->SetText(FText::FromString(FString::Printf(TEXT("Kill\n%d"), GameInstance->CurrentKill)));
			}
		}

		//스코어 text (누적 경험치)
		if (UTextBlock* TotalScoreText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName("ScoreText")))
		{
			if (UTeam13_GameInstance* GameInstance = Cast<UTeam13_GameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score\n%d"), GameInstance->Score)));
			}
		}

		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			if (ATeam13_PlayerController* Team13_PlayerController = Cast<ATeam13_PlayerController>(PlayerController))
			{
				ACharacter* player = PlayerController->GetCharacter();
				if (AHERO_Character* HeroCharacter = Cast<AHERO_Character>(player))
				{
					//레벨 text
					if (UTextBlock* TotalLevelText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName("LevelText")))
					{
						TotalLevelText->SetText(FText::FromString(FString::Printf(TEXT("Level\n%d"), HeroCharacter->Level)));
					}
				}
			}
		}
	}
}

//크레딧메뉴
void ATeam13_PlayerController::ShowCreditMenu()
{
	WidgetRemove();

	//크레딧메뉴 생성
	if (CreditWidgetClass)
	{
		CreditWidgetInstance = CreateWidget<UUserWidget>(this, CreditWidgetClass);
		if (CreditWidgetInstance)
		{
			CreditWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//마우스 커서O
			SetInputMode(FInputModeUIOnly());
		}
	}
}

UUserWidget* ATeam13_PlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

//게임중 UI
void ATeam13_PlayerController::ShowGameHUD()
{
	WidgetRemove();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			bShowMouseCursor = false;			//마우스 커서X
			SetInputMode(FInputModeGameOnly());

			ATeam13_GameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATeam13_GameState>() : nullptr;
			if (GameState)
			{
				GameState->UpdateHUD();
			}
		}
	}
}

//게임시작
void ATeam13_PlayerController::StartGame()
{

	if (UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		Team13_GameInstance->CurrentStageIndex = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("Team13_SH_Map"));
	SetPause(false);
}

//게임 완전 종료
void ATeam13_PlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(
		GetWorld(),
		this,
		EQuitPreference::Quit,
		false);
}

//실행되는 위젯 제거
void ATeam13_PlayerController::WidgetRemove()
{
	//실행되는 HUD위젯 제거
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	//실행되는 메인메뉴의 위젯 제거
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	//실행되는 가이드메뉴의 위젯 제거
	if (GuideWidgetInstance)
	{
		GuideWidgetInstance->RemoveFromParent();
		GuideWidgetInstance = nullptr;
	}

	//실행되는 셋팅메뉴의 위젯 제거
	if (SettingWidgetInstance)
	{
		SettingWidgetInstance->RemoveFromParent();
		SettingWidgetInstance = nullptr;
	}

	//실행되는 엔드메뉴의 위젯 제거
	if (EndWidgetInstance)
	{
		EndWidgetInstance->RemoveFromParent();
		EndWidgetInstance = nullptr;
	}
}
