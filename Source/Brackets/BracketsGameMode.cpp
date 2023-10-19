// Copyright Epic Games, Inc. All Rights Reserved.

#include "BracketsGameMode.h"
#include "BracketsCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABracketsGameMode::ABracketsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Extra/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;

}
