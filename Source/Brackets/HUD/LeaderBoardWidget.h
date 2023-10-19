// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LeaderBoardWidget.generated.h"

class UTextBlock;
class UWidgetSwitcher;
class AController;
class APlayerState;

UCLASS()
class BRACKETS_API ULeaderBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetPlayerNameAndPlace();
	UFUNCTION()
	void UpdatePlayerLeaderBoardNames();//fix and update calls
	UFUNCTION()
	void UpdatePlayerNameText(int32 Itterator, int32 RoundNumber, FString PlayerName);

protected:
	UFUNCTION()
	virtual bool Initialize() override;
	UFUNCTION()
	void CreateNameSlotAray();

	UPROPERTY()
		TArray<AController*>ControllerArray;
	UPROPERTY()
		TArray<APlayerState*>PlayerStateArray;
	UPROPERTY()
		int32 CurrentRound = 0;
	int32 WidgetSwitchIndex = 0;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WidgetRoundSwitcher;

	///////////////////////////////////////////

	UPROPERTY()
	TArray<UTextBlock*> R1Array;
	UPROPERTY()
	TArray<UTextBlock*> R2Array;
	UPROPERTY()
	TArray<UTextBlock*> R3Array;
	UPROPERTY()
	TArray<UTextBlock*> R4Array;

public: // Round 1 (16 players) Name slots
	UPROPERTY(meta = (BindWidget))
	UTextBlock* R1_NameSlot1;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot2;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot3;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot4;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot5;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot6;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot7;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot8;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot9;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot10;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot11;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot12;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot13;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot14;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot15;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R1_NameSlot16;

///////////////////////////////////////////

public: // Round 2 (8 players) Name slots
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot1;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot2;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot3;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot4;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot5;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot6;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot7;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R2_NameSlot8;

///////////////////////////////////////////

public:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R3_NameSlot1;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R3_NameSlot2;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R3_NameSlot3;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R3_NameSlot4;

///////////////////////////////////////////

public:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R4_NameSlot1;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* R4_NameSlot2;
	
};
