// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BracketsSinglesGameState.generated.h"

class ABracketsPlayerState;
class AController;

UCLASS()
class BRACKETS_API ABracketsSinglesGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void FirstPlayerKill(ABracketsPlayerState* AttackerPlayerState);
	void SetCurrentRound(int32 RoundNumber);
	void UpdateLeaderboard();

	void DEBUGlength();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetCurrentRound(int32 RoundNumber);

private:
	float TopScore = 0.f;
	UPROPERTY()
	int32 CurrentRound = 0;
	UPROPERTY(Replicated)
	ABracketsPlayerState* FirstPlayerWithKill;

	UPROPERTY(Replicated)
	TArray<APlayerState*>MasterPlayerStateArray;

public:
	ABracketsPlayerState* GetFristPlayerWithKill() { return FirstPlayerWithKill; }
	TArray<APlayerState*> GetMasterPlayerStateArray() { return MasterPlayerStateArray; }
	TArray<APlayerState*> GetPlayerStateArray() { return PlayerArray; }
	int32 GetCurrentRound() { return CurrentRound; }
};
