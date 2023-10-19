// Fill out your copyright notice in the Description page of Project Settings.


#include "RoundEndWidget.h"
#include "Brackets/Player/BracketsPlayerController.h"

void URoundEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EndDelegate.BindDynamic(this, &URoundEndWidget::AnimationFinished);

	BindToAnimationFinished(TextAnimation, EndDelegate);
}

void URoundEndWidget::AnimationFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("OnAnimationFinishedPlaying"))
	SetVisibility(ESlateVisibility::Hidden);
	ABracketsPlayerController* BracketsPlayerController = Cast<ABracketsPlayerController>(GetOwningPlayer());
	if (BracketsPlayerController)
	{
		BracketsPlayerController->ShowHUDLeaderBoard(true, true);

	}
}



