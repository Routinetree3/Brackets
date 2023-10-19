// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchStartWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class BRACKETS_API UMatchStartWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* MatchStartText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
		UWidgetAnimation* TextAnimation;

	FWidgetAnimationDynamicEvent EndDelegate;
	UFUNCTION()
		void AnimationFinished();
	UFUNCTION()
		void StartDestroyAnimation();

protected:
	UFUNCTION()
	virtual void NativeConstruct() override;
	
};
