// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuPlayerController.h"
#include "Brackets/HUD/Menu/MenuHUD.h"
#include "MultiplayerSession/Public/Menu.h"
#include "Components/TextBlock.h"
#include "Brackets/GameModes/MenuGameMode.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "Brackets/Components/BracketsBeaconHostObject.h"

void AMenuPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMenuPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

