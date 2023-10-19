// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BracketsCharacterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;

};

UCLASS()
class BRACKETS_API ABracketsCharacterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterHUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "EquipmentSelectionWidget")
	TSubclassOf<UUserWidget> EquipmentSelectionWidgetClass;

	UPROPERTY(EditAnywhere, Category = "RoundEndWidget")
	TSubclassOf<UUserWidget> RoundEndWidgetClass;

	UPROPERTY(EditAnywhere, Category = "LeaderBoardWidget")
	TSubclassOf<UUserWidget> LeaderBoardWidgetClass;

	UPROPERTY(EditAnywhere, Category = "InGameMenuWidget")
	TSubclassOf<UUserWidget> InGameMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "MatchStartWidget")
	TSubclassOf<UUserWidget> MatchStartWidgetClass;

	UPROPERTY()
	class UCharacterHUDWidget* CharacterHUDWidget;

	UPROPERTY()
	class UEquipmentSelectionWidget* EquipmentSelectionWidget;

	UPROPERTY()
	class URoundEndWidget* RoundEndWidget;

	UPROPERTY()
	class ULeaderBoardWidget* LeaderBoardWidget;

	UPROPERTY()
	class UInGameMenuWidget* InGameMenuWidget;

	UPROPERTY()
	class UMatchStartWidget* MatchStartWidget;

	void AddCharacterHUDWidget();
	void AddLeaderBoardWidget();
	void AddEquipmentSelectWidget();
	void AddRoundEndWidget();
	void AddInGameMenuWidget();
	void AddMatchStartWidget();

	void RemoveEquipmentSelectWidget();
	void RemoveRoundEndWidget();

protected:

	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
	
	UPROPERTY(EditAnywhere)
		float CrosshairSpreadMax = 16.f;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

};
