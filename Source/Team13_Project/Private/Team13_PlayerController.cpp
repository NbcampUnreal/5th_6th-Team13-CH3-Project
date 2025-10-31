#include "Team13_PlayerController.h"
#include "Team13_GameInstance.h"
#include "Team13_GameState.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

ATeam13_PlayerController::ATeam13_PlayerController()
	: HUDWidgetClass(nullptr),
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

	//StartMenu�� �ִٸ� ShowStartMenuȣ��
	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("StartMenu"))
	{
		ShowStartMenu();
	}
}

//���� ���� �޴�
void ATeam13_PlayerController::ShowStartMenu()
{
	WidgetRemove();

	//���θ޴� ����
	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//���콺 Ŀ��O
			SetInputMode(FInputModeUIOnly());
		}
	}

	//���� �Ƚᵵ��
	if (MainMenuWidgetInstance)
	{
		//Start��ư Text
		if (UTextBlock* StartButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
		{
			StartButton->SetText(FText::FromString(TEXT("Start")));
		}

		//Guide��ư Text
		if (UTextBlock* GuideButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("GuideButtonText"))))
		{
			GuideButton->SetText(FText::FromString(TEXT("Guide")));
		}

		//Setting��ư Text
		if (UTextBlock* SettingButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("SettingButtonText"))))
		{
			SettingButton->SetText(FText::FromString(TEXT("Setting")));
		}

		//Quit��ư Text
		if (UTextBlock* QuitButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("QuitButtonText"))))
		{
			QuitButton->SetText(FText::FromString(TEXT("Quit")));
		}
	}
}

void ATeam13_PlayerController::ShowGuideMenu()
{
	WidgetRemove();

	//���̵�޴� ����
	if (GuideWidgetClass)
	{
		GuideWidgetInstance = CreateWidget<UUserWidget>(this, GuideWidgetClass);
		if (GuideWidgetInstance)
		{
			GuideWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//���콺 Ŀ��O
			SetInputMode(FInputModeUIOnly());
		}
	}

	//���� �Ƚᵵ��
	if (GuideWidgetInstance)
	{
		//Back��ư Text
		if (UTextBlock* BackButton = Cast<UTextBlock>(GuideWidgetInstance->GetWidgetFromName(TEXT("BackButtonText"))))
		{
			BackButton->SetText(FText::FromString(TEXT("Back")));
		}

		//Next��ư Text
		if (UTextBlock* NextButton = Cast<UTextBlock>(GuideWidgetInstance->GetWidgetFromName(TEXT("NextButtonText"))))
		{
			NextButton->SetText(FText::FromString(TEXT("Next")));
		}
	}
}

//���ø޴�
void ATeam13_PlayerController::ShowSettingMenu()
{
	WidgetRemove();

	//���ø޴� ����
	if (SettingWidgetClass)
	{
		SettingWidgetInstance = CreateWidget<UUserWidget>(this, SettingWidgetClass);
		if (SettingWidgetInstance)
		{
			SettingWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//���콺 Ŀ��O
			SetInputMode(FInputModeUIOnly());
		}
	}

	//���� �Ƚᵵ��
	if (SettingWidgetInstance)
	{
		//Back��ư Text
		if (UTextBlock* BackButton = Cast<UTextBlock>(SettingWidgetInstance->GetWidgetFromName(TEXT("BackButtonText"))))
		{
			BackButton->SetText(FText::FromString(TEXT("Back")));
		}
	}
}

//�����޴�
//������ ���� �� ȣ���ؼ� ���
void ATeam13_PlayerController::ShowEndMenu(bool bIsReStart)
{
	WidgetRemove();

	//����޴� ����
	if (EndWidgetClass)
	{
		EndWidgetInstance = CreateWidget<UUserWidget>(this, EndWidgetClass);
		if (EndWidgetInstance)
		{
			EndWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//���콺 Ŀ��O
			SetInputMode(FInputModeUIOnly());
		}
	}

	if (EndWidgetInstance)
	{
		//Quit��ư Text
		if (UTextBlock* QuitButton = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("QuitButtonText"))))
		{
			QuitButton->SetText(FText::FromString(TEXT("Quit")));
		}

		//Main Menu��ư Text
		if (UTextBlock* ReStartButton = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("ReStartButtonText"))))
		{
			ReStartButton->SetText(FText::FromString(TEXT("Main Menu")));
		}

		//Credit��ư Text
		if (UTextBlock* CreditButton = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("CreditButtonText"))))
		{
			CreditButton->SetText(FText::FromString(TEXT("Credit")));
		}

		//Result Text (true or false)
		if (UTextBlock* ResultText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName(TEXT("ResultText"))))
		{
			ResultText->SetText(FText::FromString(bIsReStart ? TEXT("Loser") : TEXT("Winner")));
		}

		//���� text
		if (UTextBlock* TotalLevelText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName("LevelText")))
		{
			if (UTeam13_GameInstance* GameInstance = Cast<UTeam13_GameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				TotalLevelText->SetText(FText::FromString(FString::Printf(TEXT("Level : %d"), GameInstance->CurrentLevel)));
			}
		}

		//ų text
		if (UTextBlock* TotalKillText = Cast<UTextBlock>(EndWidgetInstance->GetWidgetFromName("KillText")))
		{
			if (UTeam13_GameInstance* GameInstance = Cast<UTeam13_GameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				TotalKillText->SetText(FText::FromString(FString::Printf(TEXT("Kill : %d"), GameInstance->CurrentKill)));
			}
		}
	}
}

//ũ�����޴�
void ATeam13_PlayerController::ShowCreditMenu()
{
	WidgetRemove();

	//ũ�����޴� ����
	if (CreditWidgetClass)
	{
		CreditWidgetInstance = CreateWidget<UUserWidget>(this, CreditWidgetClass);
		if (CreditWidgetInstance)
		{
			CreditWidgetInstance->AddToViewport();
			bShowMouseCursor = true;			//���콺 Ŀ��O
			SetInputMode(FInputModeUIOnly());
		}
	}
}

UUserWidget* ATeam13_PlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

//������ UI
void ATeam13_PlayerController::ShowGameHUD()
{
	WidgetRemove();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			bShowMouseCursor = false;			//���콺 Ŀ��X
			SetInputMode(FInputModeUIOnly());

			ATeam13_GameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATeam13_GameState>() : nullptr;
			if (GameState)
			{
				GameState->UpdateHUD();
			}
		}
	}
}

//���ӽ���
void ATeam13_PlayerController::StartGame()
{
	if (UTeam13_GameInstance* Team13_GameInstance = Cast<UTeam13_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		Team13_GameInstance->CurrentStageIndex = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("Team13_SH_Map"));
	SetPause(false);
}

//���� ���� ����
void ATeam13_PlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(
		GetWorld(),
		this,
		EQuitPreference::Quit,
		false);
}

//����Ǵ� ���� ����
void ATeam13_PlayerController::WidgetRemove()
{
	//����Ǵ� HUD���� ����
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	//����Ǵ� ���θ޴��� ���� ����
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	//����Ǵ� ���̵�޴��� ���� ����
	if (GuideWidgetInstance)
	{
		GuideWidgetInstance->RemoveFromParent();
		GuideWidgetInstance = nullptr;
	}

	//����Ǵ� ���ø޴��� ���� ����
	if (SettingWidgetInstance)
	{
		SettingWidgetInstance->RemoveFromParent();
		SettingWidgetInstance = nullptr;
	}

	//����Ǵ� ����޴��� ���� ����
	if (EndWidgetInstance)
	{
		EndWidgetInstance->RemoveFromParent();
		EndWidgetInstance = nullptr;
	}
}
