// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsPlayerState.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Brackets/HUD/BracketsCharacterHUD.h"
#include "Brackets/HUD/LeaderBoardWidget.h"
#include "Brackets/BracketsCharacter.h"
#include "Net/UnrealNetwork.h"

void ABracketsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABracketsPlayerState, bHasTakenDamage);
	//DOREPLIFETIME(ABracketsPlayerState, bIsDead);
}

//Implement A Leaderboard
void ABracketsPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	//////////////////
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Black,
		FString::Printf(TEXT("Score: %f"), ScoreAmount)
	);
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Yellow,
		FString::Printf(TEXT("Score: %f"), GetScore())
	);
	//////////////////
}

void ABracketsPlayerState::SetHasTakenDamage(bool TakenDamge)
{
	bHasTakenDamage = TakenDamge;
}

void ABracketsPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
}

void ABracketsPlayerState::SetIsDead(bool IsDead)
{
	MulticastSetIsDead(IsDead);
}

void ABracketsPlayerState::MulticastSetIsDead_Implementation(bool IsDead)
{
	bIsDead = IsDead;
}

void ABracketsPlayerState::MulticastUpdateLeaderboard_Implementation()
{
	ABracketsPlayerController* BracketsPlayerController = Cast<ABracketsPlayerController>(GetPlayerController());
	if (BracketsPlayerController)
	{
		ABracketsCharacterHUD* BracketsCharacterHUD = Cast<ABracketsCharacterHUD>(BracketsPlayerController->GetHUD());
		if (BracketsCharacterHUD)
		{
			bool bLeaderBoardWidgetValid = BracketsCharacterHUD && BracketsCharacterHUD->LeaderBoardWidget;

			if (bLeaderBoardWidgetValid)
			{
				BracketsCharacterHUD->LeaderBoardWidget->UpdatePlayerLeaderBoardNames();
			}
		}
	}
}

