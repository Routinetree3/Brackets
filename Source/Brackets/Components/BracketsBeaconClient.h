// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconClient.h"
#include "AdvancedFriendsLibrary.h"
#include "Brackets/GameInstance/BracketsGameInstance.h"
#include "BracketsBeaconClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectSuccess, bool, FOnConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectionFailed, bool, FOnConnectionFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFollowHost);

UCLASS()
class BRACKETS_API ABracketsBeaconClient : public AOnlineBeaconClient
{
	GENERATED_BODY()

public:
	ABracketsBeaconClient();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SetPlayerID(FBPUniqueNetId PlayerID);
	UFUNCTION(Client, Reliable)
	void ClientUpdateNewPlayerInformation(const TArray<FPlayerInformation>& PlayerInformationArray, const TArray<FPlayerCardInformation>& PlayerCardInformationArray, const TArray<FBPUniqueNetId>& UniqueNetIdArray);
	UFUNCTION(Client, Reliable)
		void ClientSetIncomingPlayerInformation();
	UFUNCTION(Client, Reliable)
		void ClientFollowHost(FBlueprintSessionResult JoinResult);
	UFUNCTION(Server, Reliable)
		void ServerSetIncomingPlayerInformation(FBPUniqueNetId PlayerNetID, FPlayerInformation PlayerInformation, FPlayerCardInformation PlayerCardInformation);


protected:
	virtual void BeginPlay() override;

	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BlueprintCallable)
	bool ConnectToHostBeacon(FString& Address, const FBlueprintSessionResult& Result);
	UFUNCTION(BlueprintCallable)
	bool ConnectToHostFromAddress(const FString& Address);
	UFUNCTION(BlueprintCallable)
		void FullDestroyBeacon();

	UPROPERTY(BlueprintAssignable)
		FConnectSuccess FOnConnected;
	UPROPERTY(BlueprintAssignable)
		FConnectionFailed FOnConnectionFailed;
	UPROPERTY(BlueprintAssignable)
		FFollowHost OnFollowHost;

	virtual void OnConnected() override;
	virtual void OnFailure() override;

	void ClearPlayerInformationArrays();

	FBPUniqueNetId PlayerUniqueNetId;
	FPlayerInformation ClientPlayerInformation;
	FPlayerCardInformation ClientPlayerCardInformation;
	FBlueprintSessionResult ServerToJoinResult;

	UPROPERTY(BlueprintReadWrite)
		TArray<FText> AllPlayerNames;
	UPROPERTY(BlueprintReadWrite)
		TArray<int32> AllPlayerCards;
	UPROPERTY(BlueprintReadWrite)
		TArray<int32> AllPlayerIcons;

	UPROPERTY(BlueprintReadWrite)
	TArray<FBPUniqueNetId> AllUniqueNetIds;

	UPROPERTY()
	TArray<FPlayerInformation> ClientBeacon_InformationDataArray;
	UPROPERTY()
	TArray<FPlayerCardInformation> ClientBeacon_PlayerCardInformationArray;

public:
	TArray<FText> GetAllPlayerNames() const { return AllPlayerNames; }
	TArray<int32> GetAllPlayerCards() const { return AllPlayerCards; }
	TArray<int32> GetAllPlayerIcons() const { return AllPlayerIcons; }
	FBPUniqueNetId GetUniqueNetId() const { return PlayerUniqueNetId; }
	FPlayerInformation GetPlayerInformationStruct() const { return ClientPlayerInformation; }
	FPlayerCardInformation GetPlayerCardInformationStruct() const { return ClientPlayerCardInformation; }
	UFUNCTION(BlueprintCallable)
	FBlueprintSessionResult GetServerToJoinResult() const { return ServerToJoinResult; }

	
};
