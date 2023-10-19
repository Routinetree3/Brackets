// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MenuGameMode.generated.h"

UCLASS()
class BRACKETS_API AMenuGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void TravelToLobby();
	UFUNCTION(BlueprintCallable)
	bool CreateHostBeacon();
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateNewPlayerInformation(AOnlineBeaconClient* NewClientActor);

protected:
	AMenuGameMode();
	virtual void BeginPlay() override;
	virtual void GenericPlayerInitialization(AController* C) override;

	UFUNCTION(BlueprintCallable, Category = "Server")
	void OnHostSuccess(FString levelName);

	UPROPERTY(BlueprintReadOnly)
	class AOnlineBeaconHost* BaseHost;
	UPROPERTY(BlueprintReadOnly)
	class ABracketsBeaconHostObject* BracketsHost;

	class AController* OwnerController;

private:
	

public:
	ABracketsBeaconHostObject* GetHostBeacon() const { return BracketsHost; }
};
