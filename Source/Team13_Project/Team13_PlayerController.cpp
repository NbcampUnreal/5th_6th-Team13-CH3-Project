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

	//StartMenu가 있다면 ShowStartMenu호출
	FString CurrentMapName = GetWorld()->GetMapName();
	UE_LOG(LogTemp, Warning, TEXT("Current Map Name: %s"), *CurrentMapName);
	if (CurrentMapName.Contains("StartMenu"))
	{
		ShowStartMenu();
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
			bShowMouseCursor = true;			//마우스 커서작동
			SetInputMode(FInputModeUIOnly());	//UI만 영향을 줌
		}
	}

	//굳이 안써도됨
	if (MainMenuWidgetInstance)
	{
		//Start버튼 Text
		if (UTextBlock* StartButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
		{
			StartButton->SetText(FText::FromString(TEXT("Start")));
		}

		//Guide버튼 Text
		if (UTextBlock* GuideButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("GuideButtonText"))))
		{
			GuideButton->SetText(FText::FromString(TEXT("Guide")));
		}

		//Setting버튼 Text
		if (UTextBlock* SettingButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("SettingButtonText"))))
		{
			SettingButton->SetText(FText::FromString(TEXT("Setting")));
		}

		//Quit버튼 Text
		if (UTextBlock* QuitButton = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("QuitButtonText"))))
		{
			QuitButton->SetText(FText::FromString(TEXT("Quit")));
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
			bShowMouseCursor = true;			//마우스 커서작동
			SetInputMode(FInputModeUIOnly());	//UI만 영향을 줌
		}
	}

	//굳이 안써도됨
	if (GuideWidgetInstance)
	{
		//Back버튼 Text
		if (UTextBlock* BackButton = Cast<UTextBlock>(GuideWidgetInstance->GetWidgetFromName(TEXT("BackButtonText"))))
		{
			BackButton->SetText(FText::FromString(TEXT("Back")));
		}

		//Next버튼 Text
		if (UTextBlock* NextButton = Cast<UTextBlock>(GuideWidgetInstance->GetWidgetFromName(TEXT("NextButtonText"))))
		{
			NextButton->SetText(FText::FromString(TEXT("Next")));
		}
	}
}

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
			bShowMouseCursor = true;			//마우스 커서작동
			SetInputMode(FInputModeUIOnly());	//UI만 영향을 줌
		}
	}

	//굳이 안써도됨
	if (SettingWidgetInstance)
	{
		//Back버튼 Text
		if (UTextBlock* BackButton = Cast<UTextBlock>(SettingWidgetInstance->GetWidgetFromName(TEXT("BackButtonText"))))
		{
			BackButton->SetText(FText::FromString(TEXT("Back")));
		}
	}
}

//게임중 UI
void ATeam13_PlayerController::ShowGameHUD()
{
	WidgetRemove();

	//게임 HUD함수

}

//게임 종료
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
	//실행되는 HUD위젯이 있다면 제거
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	//실행되는 메인메뉴의 위젯이 있다면 제거
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

	//실행되는 셋팅메뉴의 위젯이 있다면 제거
	if (SettingWidgetInstance)
	{
		SettingWidgetInstance->RemoveFromParent();
		SettingWidgetInstance = nullptr;
	}
}
