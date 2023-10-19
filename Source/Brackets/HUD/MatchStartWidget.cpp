// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchStartWidget.h"

void UMatchStartWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EndDelegate.BindDynamic(this, &UMatchStartWidget::AnimationFinished);

	BindToAnimationFinished(TextAnimation, EndDelegate);
}

void UMatchStartWidget::AnimationFinished()
{
	RemoveFromParent();
}

void UMatchStartWidget::StartDestroyAnimation()
{
	PlayAnimation(TextAnimation);
}
