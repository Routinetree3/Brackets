// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;

UCLASS()
class BRACKETS_API UCharacterHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* WeaponAmmo;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* AmmoCarried;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* RoundCountdownText;

	UPROPERTY(meta = (BindWidget))
		UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
		UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
		UImage* PrimaryIcon;

	UPROPERTY(meta = (BindWidget))
		UImage* SecondaryIcon;

	UPROPERTY(meta = (BindWidget))
		UImage* LethalSlot1;

	UPROPERTY(meta = (BindWidget))
		UImage* LethalSlot2;

	UPROPERTY(meta = (BindWidget))
		UImage* NonLethalSlot1;

	UPROPERTY(meta = (BindWidget))
		UImage* NonLethalSlot2;
};
