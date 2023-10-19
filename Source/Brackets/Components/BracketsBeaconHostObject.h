// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconHostObject.h"
#include "AdvancedFriendsLibrary.h"
#include "Brackets/GameInstance/BracketsGameInstance.h"
#include "BracketsBeaconHostObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConnectToHostSuccess, bool, FOnConnectToHostSuccess);

UCLASS()
class BRACKETS_API ABracketsBeaconHostObject : public AOnlineBeaconHostObject
{
	GENERATED_BODY()
	
public:
	ABracketsBeaconHostObject();
	void UpdatePlayerData();

	UFUNCTION(BlueprintCallable)
		void UpdateDataArray();
	UFUNCTION(BlueprintCallable)
	void MovePartyToGame(FBlueprintSessionResult JoinResult);

protected:
	virtual void BeginPlay() override;
	virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;
	virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;

	//does not count the host
	int32 PlayerLimit = 4;

	// 0 is Host Data!!! do not clear host data
	TArray<FPlayerInformation> ClientInformationDataArray;
	// 0 is Host Data!!! do not clear host data
	TArray<FPlayerCardInformation> ClientPlayerCardInformationArray;
	UPROPERTY(BlueprintReadWrite)
	TArray<FText> AllPlayerNames;
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> AllPlayerCards;
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> AllPlayerIcons;
	void ClearPlayerInformationArrays();

	UPROPERTY(BlueprintReadWrite)
	TArray<FBPUniqueNetId> AllUniqueNetIds;

	UPROPERTY(BlueprintAssignable)
	FConnectToHostSuccess FOnConnectToHostSuccess;

	class ABracketsBeaconClient* NewBracketsClient;

public:
	TArray<AOnlineBeaconClient*> GetClientArray() const { return ClientActors; }
	ABracketsBeaconClient* GetNewBracketsClient() const { return NewBracketsClient; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetNumberOfClients() const { return ClientActors.Num(); }
};
