// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsGameInstance.h"

void UBracketsGameInstance::SetXP(int32 XPAmount)
{
	PlayerXP = PlayerXP + XPAmount;
}

void UBracketsGameInstance::SetPlayerName(FText PlayerName)
{
	PlayerInformation.PlayerName = PlayerName;
}
