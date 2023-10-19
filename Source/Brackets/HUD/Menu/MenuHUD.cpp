// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuHUD.h"
#include "Gameframework/PlayerController.h"
#include "MultiplayerSession/Public/Menu.h"
#include "BaseCustomizationWidget.h"
#include "GameSelectionMenu.h"

void AMenuHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AMenuHUD::AddMainMenuWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UMenu>(PlayerController, MainMenuWidgetClass);
		MainMenuWidget->AddToViewport();
	}
}

void AMenuHUD::AddCustomizationWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && BaseCustomizationWidgetClass)
	{
		CustomizationWidget = CreateWidget<UBaseCustomizationWidget>(PlayerController, BaseCustomizationWidgetClass);
		CustomizationWidget->AddToViewport();
	}
}

void AMenuHUD::AddGameSelectionWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && GameSelectionWidgetClass)
	{
		GameSelectionWidget = CreateWidget<UGameSelectionMenu>(PlayerController, GameSelectionWidgetClass);
		GameSelectionWidget->AddToViewport();
	}
}

void AMenuHUD::RemoveMainMenuWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && MainMenuWidgetClass)
	{
		MainMenuWidget->RemoveFromParent();
	}
}

void AMenuHUD::RemoveCustomizationWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && BaseCustomizationWidgetClass)
	{
		CustomizationWidget->RemoveFromParent();
	}
}

void AMenuHUD::RemoveSelectionWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && GameSelectionWidgetClass)
	{
		GameSelectionWidget->RemoveFromParent();
	}
}
