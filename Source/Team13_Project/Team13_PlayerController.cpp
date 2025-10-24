#include "Team13_PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

ATeam13_PlayerController::ATeam13_PlayerController() 
	: HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr),
	GuideWidgetClass(nullptr),
	GuideWidgetInstance(nullptr),
	SettingWidgetClass(nullptr),
	SettingWidgetInstance(nullptr)
{
}

void ATeam13_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	//StartMenu�� �ִٸ� ShowStartMenuȣ��
	FString CurrentMapName = GetWorld()->GetMapName();
	UE_LOG(LogTemp, Warning, TEXT("Current Map Name: %s"), *CurrentMapName);
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
			bShowMouseCursor = true;			//���콺 Ŀ���۵�
			SetInputMode(FInputModeUIOnly());	//UI�� ������ ��
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
			bShowMouseCursor = true;			//���콺 Ŀ���۵�
			SetInputMode(FInputModeUIOnly());	//UI�� ������ ��
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
			bShowMouseCursor = true;			//���콺 Ŀ���۵�
			SetInputMode(FInputModeUIOnly());	//UI�� ������ ��
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

//������ UI
void ATeam13_PlayerController::ShowGameHUD()
{
	WidgetRemove();

	//���� HUD�Լ�

}

//���� ����
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
	//����Ǵ� HUD������ �ִٸ� ����
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	//����Ǵ� ���θ޴��� ������ �ִٸ� ����
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (GuideWidgetInstance)
	{
		GuideWidgetInstance->RemoveFromParent();
		GuideWidgetInstance = nullptr;
	}

	//����Ǵ� ���ø޴��� ������ �ִٸ� ����
	if (SettingWidgetInstance)
	{
		SettingWidgetInstance->RemoveFromParent();
		SettingWidgetInstance = nullptr;
	}
}
