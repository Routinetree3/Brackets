// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsBeaconClient.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BracketsBeaconHostObject.h"
#include "FindSessionsCallbackProxy.h"


ABracketsBeaconClient::ABracketsBeaconClient()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void ABracketsBeaconClient::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABracketsBeaconClient::BeginPlay()
{
}

void ABracketsBeaconClient::SetPlayerID(FBPUniqueNetId PlayerID)
{
	PlayerUniqueNetId = PlayerID;
}

bool ABracketsBeaconClient::ConnectToHostBeacon(FString& Address, const FBlueprintSessionResult& Result)
{
	SessionInterface->GetResolvedConnectString(Result.OnlineResult, NAME_BeaconPort, Address);
	FURL Destination = FURL(nullptr, *Address, ETravelType::TRAVEL_Absolute);
	Destination.Port = 7787;

	return InitClient(Destination);
}

bool ABracketsBeaconClient::ConnectToHostFromAddress(const FString& Address)
{
	FURL Destination = FURL(nullptr, *Address, TRAVEL_Absolute);
	Destination.Port = 7787;
	return InitClient(Destination);
}

void ABracketsBeaconClient::FullDestroyBeacon()
{
	DestroyBeacon();
}

void ABracketsBeaconClient::OnConnected()
{
	Super::OnConnected();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Blue, TEXT("Connected"));
	}

	/*ABracketsBeaconHostObject* BeaconHost = Cast<ABracketsBeaconHostObject>(BeaconOwner);
	if (BeaconHost)
	{
		UBracketsGameInstance* GameInstance = Cast<UBracketsGameInstance>(GetGameInstance());
		if (GameInstance)
		{
			BeaconHost->ServerSetIncomingPlayerInformation(this, PlayerUniqueNetId, GameInstance->GetPlayerInformationStruct(), GameInstance->GetPlayerCardInformationStruct());
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Green, TEXT("SentPlayerInformation_CLIENT"));
			}
		}
	}*/
}

void ABracketsBeaconClient::ClientSetIncomingPlayerInformation_Implementation()
{
	UBracketsGameInstance* GameInstance = Cast<UBracketsGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		ServerSetIncomingPlayerInformation(PlayerUniqueNetId, GameInstance->GetPlayerInformationStruct(), GameInstance->GetPlayerCardInformationStruct());
	}
}

void ABracketsBeaconClient::ServerSetIncomingPlayerInformation_Implementation(FBPUniqueNetId PlayerNetID, FPlayerInformation PlayerInformation, FPlayerCardInformation PlayerCardInformation)
{
	PlayerUniqueNetId = PlayerNetID;
	ClientPlayerInformation = PlayerInformation;
	ClientPlayerCardInformation = PlayerCardInformation;
	if (GEngine)
	{
		FString Name = PlayerInformation.PlayerName.ToString();
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Purple, (TEXT("%s"), *Name));
	}
	ABracketsBeaconHostObject* Host = Cast<ABracketsBeaconHostObject>(GetBeaconOwner());
	if (Host)
	{
		Host->UpdateDataArray();
	}
}

void ABracketsBeaconClient::OnFailure()
{
	Super::OnFailure();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Red, TEXT("Connection Failed"));
	}

	ClearPlayerInformationArrays();

	FOnConnectionFailed.Broadcast(true);

}

void ABracketsBeaconClient::ClearPlayerInformationArrays()
{
	AllPlayerCards.Empty();
	AllPlayerIcons.Empty();
	AllPlayerNames.Empty();
}

void ABracketsBeaconClient::ClientUpdateNewPlayerInformation_Implementation(const TArray<FPlayerInformation>& PlayerInformationArray, const TArray<FPlayerCardInformation>& PlayerCardInformationArray, const TArray<FBPUniqueNetId>& UniqueNetIdArray)
{
	ClientBeacon_InformationDataArray = PlayerInformationArray;
	ClientBeacon_PlayerCardInformationArray = PlayerCardInformationArray;
	ClearPlayerInformationArrays();
	int16 InformationArraySize = ClientBeacon_InformationDataArray.Num();
	for (int16 i = 0; i < InformationArraySize; i++)
	{
		AllPlayerNames.Add(ClientBeacon_InformationDataArray[i].PlayerName);
	}
	int16 CardArraySize = ClientBeacon_PlayerCardInformationArray.Num();
	for (int16 i = 0; i < CardArraySize; i++)
	{
		AllPlayerCards.Add(ClientBeacon_PlayerCardInformationArray[i].PlayerCardIndex);
		AllPlayerIcons.Add(ClientBeacon_PlayerCardInformationArray[i].PlayerIconIndex);
	}

	FOnConnected.Broadcast(true);
}

void ABracketsBeaconClient::ClientFollowHost_Implementation(FBlueprintSessionResult JoinResult)
{
	ServerToJoinResult = JoinResult;
	OnFollowHost.Broadcast();
}
