// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MenuHUD.generated.h"

class UUserWidget;

UCLASS()
class BRACKETS_API AMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "MainMenuWidget")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY(BlueprintReadWrite)
	class UMenu* MainMenuWidget;

	UPROPERTY(EditAnywhere, Category = "CustomizationWidget")
	TSubclassOf<UUserWidget> BaseCustomizationWidgetClass;
	UPROPERTY(BlueprintReadWrite)
	class UBaseCustomizationWidget* CustomizationWidget;

	UPROPERTY(EditAnywhere, Category = "GameSelectionWidget")
	TSubclassOf<UUserWidget> GameSelectionWidgetClass;
	UPROPERTY(BlueprintReadWrite)
	class UGameSelectionMenu* GameSelectionWidget;

	void AddMainMenuWidget();
	void AddCustomizationWidget();
	void AddGameSelectionWidget();
	void RemoveMainMenuWidget();
	void RemoveCustomizationWidget();
	void RemoveSelectionWidget();

protected:

	virtual void BeginPlay() override;

public:
	FORCEINLINE UMenu* GetMainMenuWidget() const { return MainMenuWidget; }

	
};
