// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsSinglesGameState.h"
#include "Brackets/Player/BracketsPlayerState.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

void ABracketsSinglesGameState::BeginPlay()
{
	Super::BeginPlay();
	bReplicates = true;
}

void ABracketsSinglesGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABracketsSinglesGameState, FirstPlayerWithKill);
	DOREPLIFETIME(ABracketsSinglesGameState, MasterPlayerStateArray);
}

void ABracketsSinglesGameState::FirstPlayerKill(ABracketsPlayerState* AttackerPlayerState)
{
	FirstPlayerWithKill = AttackerPlayerState;
}

void ABracketsSinglesGameState::SetCurrentRound(int32 RoundNumber)
{
	CurrentRound = RoundNumber;
	MasterPlayerStateArray.Empty();
	MasterPlayerStateArray.Append(PlayerArray);
	MulticastSetCurrentRound(RoundNumber);
}

void ABracketsSinglesGameState::MulticastSetCurrentRound_Implementation(int32 RoundNumber)
{
	CurrentRound = RoundNumber;
}

void ABracketsSinglesGameState::UpdateLeaderboard()
{
	int32 ArrayLength = PlayerArray.Num();
	for (int32 i = 0; i < ArrayLength; i++)
	{
		ABracketsPlayerState* BPlayerState = Cast<ABracketsPlayerState>(PlayerArray[i]);
		if (BPlayerState)
		{
			BPlayerState->MulticastUpdateLeaderboard();
		}
	}
}

void ABracketsSinglesGameState::DEBUGlength()
{
	int32 Length = PlayerArray.Num();
	for (int32 i = 0; i < Length; i++)
	{
		APlayerState* PlayerInArray = PlayerArray[i];
		if (PlayerInArray)
		{
			FName PlayerName = PlayerArray[i]->GetFName();
			UE_LOG(LogTemp, Warning, TEXT("DEBUGlength_Name: %s"), *PlayerName.ToString())
		}
	}
}
