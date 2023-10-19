// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuGameMode.h"
#include "MultiplayerSession/Public/MultiplayerSessionsSubsystem.h"
#include "Brackets/Player/MenuPlayerController.h"
#include "Brackets/Components/BracketsBeaconHostObject.h"
#include "Brackets/Components/BracketsBeaconClient.h"
#include "OnlineBeaconHost.h"
#include "OnlineBeaconClient.h"

AMenuGameMode::AMenuGameMode()
{
}

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AMenuGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);
	OwnerController = C;
}

void AMenuGameMode::TravelToLobby()
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(FString("/Game/Levels/Lobby?listen"));
	}
}

void AMenuGameMode::OnHostSuccess(FString levelName)
{
	UWorld* World = GetWorld();
	FURL newURL = FURL(*levelName);
	World->Listen(newURL);
}

bool AMenuGameMode::CreateHostBeacon()
{
	BaseHost = GetWorld()->SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass());
	if (BaseHost && BaseHost->InitHost())
	{
		BaseHost->PauseBeaconRequests(false);

		BracketsHost = GetWorld()->SpawnActor<ABracketsBeaconHostObject>(ABracketsBeaconHostObject::StaticClass());
		if (BracketsHost)
		{
			BaseHost->RegisterHost(BracketsHost);
			return true;
		}
	}
	return false;
}
