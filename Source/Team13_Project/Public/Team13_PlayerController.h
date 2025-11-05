#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Team13_PlayerController.generated.h"

class UUserWidget;
class UTextBlock;
class UInputMappingContext;
class UInputAction;

UCLASS()
class TEAM13_PROJECT_API ATeam13_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATeam13_PlayerController();

	//Player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC_HERO;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_HERO_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_HERO_Accelerate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_HERO_DashSkill;

	//게임중 HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	UUserWidget* HUDWidgetInstance;

	//메인메뉴 HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	UUserWidget* MainMenuWidgetInstance;

	//가이드 HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> GuideWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	UUserWidget* GuideWidgetInstance;

	//셋팅 HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> SettingWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	UUserWidget* SettingWidgetInstance;

	//엔딩 HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> EndWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	UUserWidget* EndWidgetInstance;

	//크레딧 HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> CreditWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu")
	UUserWidget* CreditWidgetInstance;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "HUD")
	UUserWidget* GetHUDWidget() const;
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameHUD();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowStartMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowGuideMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowSettingMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowEndMenu(bool bIsReStart);
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowCreditMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void StartGame();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void QuitGame();

	void WidgetRemove();
};
