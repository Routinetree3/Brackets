// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsCharacterHUD.h"
#include "CharacterHUDWidget.h"
#include "EquipmentSelectionWidget.h"
#include "Gameframework/PlayerController.h"
#include "RoundEndWidget.h"
#include "LeaderBoardWidget.h"
#include "InGameMenuWidget.h"
#include "MatchStartWidget.h"

void ABracketsCharacterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABracketsCharacterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f + Spread.X),
		ViewportCenter.Y - (TextureHeight / 2.f + Spread.Y)
	);
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}

void ABracketsCharacterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ABracketsCharacterHUD::AddCharacterHUDWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterHUDWidgetClass)
	{
		CharacterHUDWidget = CreateWidget<UCharacterHUDWidget>(PlayerController, CharacterHUDWidgetClass);
		CharacterHUDWidget->AddToViewport();
	}
}

void ABracketsCharacterHUD::AddLeaderBoardWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && LeaderBoardWidgetClass)
	{
		LeaderBoardWidget = CreateWidget<ULeaderBoardWidget>(PlayerController, LeaderBoardWidgetClass);
		LeaderBoardWidget->AddToViewport();
		LeaderBoardWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABracketsCharacterHUD::AddInGameMenuWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && InGameMenuWidgetClass)
	{
		InGameMenuWidget = CreateWidget<UInGameMenuWidget>(PlayerController, InGameMenuWidgetClass);
		InGameMenuWidget->AddToViewport();
		InGameMenuWidget->SetVisibility(ESlateVisibility::Hidden); 
		InGameMenuWidget->bIsFocusable = true;
	}
}
//Might Remove
void ABracketsCharacterHUD::AddMatchStartWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && MatchStartWidgetClass)
	{
		MatchStartWidget = CreateWidget<UMatchStartWidget>(PlayerController, MatchStartWidgetClass);
		MatchStartWidget->AddToViewport();
	}
}

void ABracketsCharacterHUD::AddEquipmentSelectWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (PlayerController && EquipmentSelectionWidgetClass)
	{
		EquipmentSelectionWidget = CreateWidget<UEquipmentSelectionWidget>(PlayerController, EquipmentSelectionWidgetClass);
		EquipmentSelectionWidget->AddToViewport();
		EquipmentSelectionWidget->SetVisibility(ESlateVisibility::Visible);
		EquipmentSelectionWidget->bIsFocusable = true;
		UWorld* World = GetWorld();
		if (World)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(EquipmentSelectionWidget->TakeWidget());
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}
}

void ABracketsCharacterHUD::AddRoundEndWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && RoundEndWidgetClass)
	{
		RoundEndWidget = CreateWidget<URoundEndWidget>(PlayerController, RoundEndWidgetClass);
		RoundEndWidget->AddToViewport();
		RoundEndWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABracketsCharacterHUD::RemoveEquipmentSelectWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && EquipmentSelectionWidgetClass)
	{
		EquipmentSelectionWidget->RemoveFromParent();
		UWorld* World = GetWorld();
		if (World)
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void ABracketsCharacterHUD::RemoveRoundEndWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && RoundEndWidgetClass)
	{
		RoundEndWidget->RemoveFromParent();
	}
}
