#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Team13_PlayerController.generated.h"

class UUserWidget;
class UTextBlock;

UCLASS()
class TEAM13_PROJECT_API ATeam13_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATeam13_PlayerController();

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

	virtual void BeginPlay() override;

	/*UFUNCTION(BlueprintPure, Category = "HUD")
	UUserWidget* GetHudWidget() const;*/
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameHUD();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowStartMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowGuideMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowSettingMenu();
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void QuitGame();

	void WidgetRemove();
};
