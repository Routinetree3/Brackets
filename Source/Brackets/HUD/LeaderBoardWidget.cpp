// Fill out your copyright notice in the Description page of Project Settings.


#include "LeaderBoardWidget.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Brackets/GameModes/BracketsSinglesGameMode.h"
#include "Brackets/GameStates/BracketsSinglesGameState.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Brackets/Player/BracketsPlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"

bool ULeaderBoardWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	CreateNameSlotAray();

	return true;
}

void ULeaderBoardWidget::CreateNameSlotAray()
{
	R1Array.AddUnique(R1_NameSlot1);
	R1Array.AddUnique(R1_NameSlot2);
	R1Array.AddUnique(R1_NameSlot3);
	R1Array.AddUnique(R1_NameSlot4);
	R1Array.AddUnique(R1_NameSlot5);
	R1Array.AddUnique(R1_NameSlot6);
	R1Array.AddUnique(R1_NameSlot7);
	R1Array.AddUnique(R1_NameSlot8);
	R1Array.AddUnique(R1_NameSlot9);
	R1Array.AddUnique(R1_NameSlot10);
	R1Array.AddUnique(R1_NameSlot11);
	R1Array.AddUnique(R1_NameSlot12);
	R1Array.AddUnique(R1_NameSlot13);
	R1Array.AddUnique(R1_NameSlot14);
	R1Array.AddUnique(R1_NameSlot15);
	R1Array.AddUnique(R1_NameSlot16);

	R2Array.AddUnique(R2_NameSlot1);
	R2Array.AddUnique(R2_NameSlot2);
	R2Array.AddUnique(R2_NameSlot3);
	R2Array.AddUnique(R2_NameSlot4);
	R2Array.AddUnique(R2_NameSlot5);
	R2Array.AddUnique(R2_NameSlot6);
	R2Array.AddUnique(R2_NameSlot7);
	R2Array.AddUnique(R2_NameSlot8);

	R3Array.AddUnique(R3_NameSlot1);
	R3Array.AddUnique(R3_NameSlot2);
	R3Array.AddUnique(R3_NameSlot3);
	R3Array.AddUnique(R3_NameSlot4);

	R4Array.AddUnique(R4_NameSlot1);
	R4Array.AddUnique(R4_NameSlot2);
}

void ULeaderBoardWidget::SetPlayerNameAndPlace()
{
	class ABracketsSinglesGameState* BracketsGameState = Cast<ABracketsSinglesGameState>(UGameplayStatics::GetGameState(this));

	if (BracketsGameState && CurrentRound != BracketsGameState->GetCurrentRound())
	{
		BracketsGameState->DEBUGlength();
		PlayerStateArray.Empty();
		PlayerStateArray.Append(BracketsGameState->GetMasterPlayerStateArray());
		CurrentRound = BracketsGameState->GetCurrentRound();
	}

	int32 ArrayLength = PlayerStateArray.Num();
	int32 PassInIttorator = 0;

	for (int32 Itterator = 0; Itterator < ArrayLength; Itterator++)
	{

		APlayerState* PlayerInArray = PlayerStateArray[Itterator];
		if (PlayerInArray)
		{
			FString TempPlayerName = PlayerInArray->GetPlayerName();
			class ABracketsPlayerState* BracketsPlayerState = Cast<ABracketsPlayerState>(PlayerInArray);
			if (BracketsPlayerState)
			{
				FString PlayerName = BracketsPlayerState->GetPlayerName();
				UpdatePlayerNameText(PassInIttorator, CurrentRound, PlayerName);
				PassInIttorator++;
			}
		}
	}
}

void ULeaderBoardWidget::UpdatePlayerLeaderBoardNames()
{
	class ABracketsSinglesGameState* BracketsGameState = Cast<ABracketsSinglesGameState>(UGameplayStatics::GetGameState(this));

	if (BracketsGameState)
	{
		PlayerStateArray.Empty();
		PlayerStateArray.Append(BracketsGameState->GetMasterPlayerStateArray());
		CurrentRound = BracketsGameState->GetCurrentRound();
		WidgetRoundSwitcher->SetActiveWidgetIndex(CurrentRound);
	}
	int32 ArrayLength = PlayerStateArray.Num();
	int32 Itterator = 0;
	int32 PlayerState1 = 0;
	int32 PlayerState2 = 1;


	while (Itterator < ArrayLength / 2)
	{
		APlayerState* PlayerInArray1 = PlayerStateArray[PlayerState1];
		APlayerState* PlayerInArray2 = PlayerStateArray[PlayerState2];
		if (PlayerInArray1 && PlayerInArray2)
		{
			class ABracketsPlayerState* BracketsPlayerState1 = Cast<ABracketsPlayerState>(PlayerInArray1);
			class ABracketsPlayerState* BracketsPlayerState2 = Cast<ABracketsPlayerState>(PlayerInArray2);
			
			if (BracketsPlayerState1->GetIsDead() || BracketsPlayerState2->GetIsDead()) //Check to see if one of the players in the bracket is dead
			{
				if (BracketsPlayerState1->GetIsDead()) //Check to see if player 1 is dead
				{
					//FString PlayerName = BracketsPlayerState2->GetPlayerName();
					FString PlayerName = BracketsPlayerState2->GetPlayerName();
					UpdatePlayerNameText(Itterator, CurrentRound - 1, PlayerName); // Current round - 1 moves the name to the next round on the leader board
					/*PlayerStateArray.RemoveAt(PlayerState1); // Remove dead
					return;*/
				}
				else if (BracketsPlayerState2->GetIsDead()) //Check to see if player 2 is dead
				{
					//FString PlayerName = BracketsPlayerState1->GetPlayerName();
					FString PlayerName = BracketsPlayerState1->GetPlayerName();
					UpdatePlayerNameText(Itterator, CurrentRound - 1, PlayerName);
					/*PlayerStateArray.RemoveAt(PlayerState2); // Remove dead
					return;*/
				}
				//Set the index for the next look through ad one because one memeber of the pair was removed
				/*PlayerState1 = PlayerState1 + 1;
				PlayerState2 = PlayerState2 + 1;
				PlayerState1 = FMath::Clamp(PlayerState1, 0, ArrayLength - 1);
				PlayerState2 = FMath::Clamp(PlayerState2, 0, ArrayLength);*/
			}
			PlayerState1 = PlayerState1 + 2;
			PlayerState2 = PlayerState2 + 2;
			PlayerState1 = FMath::Clamp(PlayerState1, 0, ArrayLength - 1);
			PlayerState2 = FMath::Clamp(PlayerState2, 0, ArrayLength);
			/*else
			{
				//Because none were dead, move on to the next pair.
				PlayerState1 = PlayerState1 + 2;
				PlayerState2 = PlayerState2 + 2;
				PlayerState1 = FMath::Clamp(PlayerState1, 0, ArrayLength - 1);
				PlayerState2 = FMath::Clamp(PlayerState2, 0, ArrayLength);
			}*/
		}
		Itterator++;
	}
}

void ULeaderBoardWidget::UpdatePlayerNameText(int32 Itterator, int32 RoundNumber, FString PlayerName)
{
	if (RoundNumber == 0) { return; }
	if (RoundNumber == 4) //16 player // Qualifying
	{
		Itterator = FMath::Clamp(Itterator, 0, 15);
		R1Array[Itterator]->SetText(FText::FromString(PlayerName));
	}
	if (RoundNumber == 3) //8 player // Eleimination
	{
		Itterator = FMath::Clamp(Itterator, 0, 7);
		R2Array[Itterator]->SetText(FText::FromString(PlayerName));
	}
	if (RoundNumber == 2) //4 player // Semi Finial
	{
		Itterator = FMath::Clamp(Itterator, 0, 3);
		R3Array[Itterator]->SetText(FText::FromString(PlayerName));
	}
	if (RoundNumber == 1) //2 player // Finial
	{
		Itterator = FMath::Clamp(Itterator, 0, 1);
		R4Array[Itterator]->SetText(FText::FromString(PlayerName));
	}
}





