// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoundEndWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

UCLASS()
class BRACKETS_API URoundEndWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EndRoundText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* TextAnimation;

	FWidgetAnimationDynamicEvent EndDelegate;

	UFUNCTION()
	void AnimationFinished();

protected:
	UFUNCTION()
	virtual void NativeConstruct() override;

};
