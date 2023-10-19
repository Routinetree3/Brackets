// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Brackets/GameInstance/BracketsGameInstance.h"
#include "MenuPlayerController.generated.h"

class ULobbyUserWidget;
class ABracketsBeaconHostObject;

UCLASS()
class BRACKETS_API AMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	TArray<FText> AllPlayerNames;
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> AllPlayerCards;
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> AllPlayerIcons;
	UPROPERTY()
	TArray<FPlayerInformation> Controller_InformationDataArray;
	UPROPERTY()
	TArray<FPlayerCardInformation> Controller_PlayerCardInformationArray;
};
