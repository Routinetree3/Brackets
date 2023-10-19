// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameMenuWidget.generated.h"

class UButton;

UCLASS()
class BRACKETS_API UInGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
		UButton* ContinueButton;
	UPROPERTY(meta = (BindWidget))
		UButton* OptionsButton;


};
