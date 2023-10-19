// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedSessions/Classes/AdvancedFriendsGameInstance.h"
#include "BracketsGameInstance.generated.h"

/// <summary>
///	Save the Data that the player selected curing customization, to be used in gameplay when creating objects
///	Data is selected from customization all done on Bluprints
/// Non Pointer Data can be set in bluprints by simply making the struct
/// Pointer data would need to be sent through a Setter called on bluprint with functionaility on C++
/// setters can be declared in the structs??
/// </summary>

USTRUCT(BlueprintType)
struct FPlayerInformation
{
	GENERATED_USTRUCT_BODY()

		//Replace with Unique ID
		UPROPERTY()
		FText PlayerName;
		UPROPERTY()
		int32 PlayerLevel;
		UPROPERTY()
		int32 XP;
		UPROPERTY()
		int32 PlayerWins;
		UPROPERTY()
		int32 PlayerLosses;
		UPROPERTY()
		int32 PlayerKills;

		FPlayerInformation()
		{
			PlayerName = FText();
			PlayerLevel = 0;
			XP = 0;
			PlayerWins = 0;
			PlayerLosses = 0;
			PlayerKills = 0;
		}
};

USTRUCT(BlueprintType)
struct FPlayerCardInformation
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		int32 PlayerCardIndex;
	UPROPERTY()
		int32 PlayerIconIndex;

	FPlayerCardInformation()
	{
		PlayerCardIndex = 0;
		PlayerIconIndex = 0;
	}
};

UCLASS()
class BRACKETS_API UBracketsGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	void SetXP(int32 XPAmount);
	void SetPreveusPartyIP();
	UFUNCTION(BlueprintCallable)
	void SetPlayerName(FText PlayerName);

private:
	int32 PlayerXP = 0;
	FPlayerInformation PlayerInformation;
	FPlayerCardInformation PlayerCardInformation;

public:
	int32 GetGIPlayerXP() { return PlayerXP; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FText GetPlayerName() const { return PlayerInformation.PlayerName; }
	FPlayerInformation GetPlayerInformationStruct() { return PlayerInformation; }
	FPlayerCardInformation GetPlayerCardInformationStruct() { return PlayerCardInformation; }
};
