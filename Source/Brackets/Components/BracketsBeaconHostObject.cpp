// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsBeaconHostObject.h"
#include "BracketsBeaconClient.h"
#include "Brackets/GameModes/MenuGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Brackets/Player/MenuPlayerController.h"


ABracketsBeaconHostObject::ABracketsBeaconHostObject()
{
	ClientBeaconActorClass = ABracketsBeaconClient::StaticClass();
	BeaconTypeName = ClientBeaconActorClass->GetName();
}

void ABracketsBeaconHostObject::BeginPlay()
{
	UBracketsGameInstance* GameInstance = Cast<UBracketsGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		ClientInformationDataArray.Add(GameInstance->GetPlayerInformationStruct());
		ClientPlayerCardInformationArray.Add(GameInstance->GetPlayerCardInformationStruct());
	}
	UpdatePlayerData();
}

void ABracketsBeaconHostObject::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection)
{
	if (ClientActors.Num() >= PlayerLimit)
	{
		return;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Magenta, TEXT("PlayerKickedforLimit"));
		}
	}
	Super::OnClientConnected(NewClientActor, ClientConnection);
	NewBracketsClient = Cast<ABracketsBeaconClient>(NewClientActor);
	NewBracketsClient->ClientSetIncomingPlayerInformation();

	if (NewClientActor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Blue, TEXT("UserConnected"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, TEXT("User Failed to Connect"));
		}
	}
}

void ABracketsBeaconHostObject::NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor)
{
	int32 LeavingClientIndex = ClientActors.Find(LeavingClientActor);
	LeavingClientIndex = LeavingClientIndex + 1;
	if (LeavingClientIndex > 0)
	{
		//AllUniqueNetIds.RemoveAt(LeavingClientIndex);
		ClientInformationDataArray.RemoveAt(LeavingClientIndex);
		ClientPlayerCardInformationArray.RemoveAt(LeavingClientIndex);
	}
	UpdatePlayerData();
	
	Super::NotifyClientDisconnected(LeavingClientActor);
}

void ABracketsBeaconHostObject::UpdateDataArray()
{

	if (NewBracketsClient != nullptr)
	{
		AllUniqueNetIds.Add(NewBracketsClient->GetUniqueNetId());
		ClientInformationDataArray.Add(NewBracketsClient->GetPlayerInformationStruct());
		ClientPlayerCardInformationArray.Add(NewBracketsClient->GetPlayerCardInformationStruct());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, TEXT("UpdateArrays"));
		}
		UpdatePlayerData();
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, TEXT("Fail"));
		}
	}
}

void ABracketsBeaconHostObject::UpdatePlayerData()
{
	int32 ClientConnections = ClientActors.Num();
	for (int16 i = 0; i < ClientConnections; i++)
	{
		ABracketsBeaconClient* BracketsClient = Cast<ABracketsBeaconClient>(ClientActors[i]);
		if (BracketsClient)
		{
			BracketsClient->ClientUpdateNewPlayerInformation(ClientInformationDataArray, ClientPlayerCardInformationArray, AllUniqueNetIds);
		}
	}

	ClearPlayerInformationArrays();
	int16 InformationArraySize = ClientInformationDataArray.Num();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, FString::Printf(TEXT("InformationArraySize: %d"), InformationArraySize));
	}
	for (int16 i = 0; i < InformationArraySize; i++)
	{
		AllPlayerNames.Add(ClientInformationDataArray[i].PlayerName);
	}
	int16 CardArraySize = ClientPlayerCardInformationArray.Num();
	for (int16 i = 0; i < CardArraySize; i++)
	{
		AllPlayerCards.Add(ClientPlayerCardInformationArray[i].PlayerCardIndex);
		AllPlayerIcons.Add(ClientPlayerCardInformationArray[i].PlayerIconIndex);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Cyan, FString::Printf(TEXT("AllArraySize: %d, %d, %d"), AllPlayerCards.Num(),AllPlayerIcons.Num(),AllPlayerNames.Num()));
	}
	FOnConnectToHostSuccess.Broadcast(true);
}

void ABracketsBeaconHostObject::ClearPlayerInformationArrays()
{
	AllPlayerCards.Empty();
	AllPlayerIcons.Empty();
	AllPlayerNames.Empty();
}

void ABracketsBeaconHostObject::MovePartyToGame(FBlueprintSessionResult JoinResult)
{
	for (AOnlineBeaconClient* Clients : ClientActors)
	{
		ABracketsBeaconClient* BracketsClient = Cast<ABracketsBeaconClient>(Clients);
		if (BracketsClient)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Green, FString::Printf(TEXT("%s"), *JoinResult.OnlineResult.GetSessionIdStr()));
			}
			BracketsClient->ClientFollowHost(JoinResult);
		}
	}
}
