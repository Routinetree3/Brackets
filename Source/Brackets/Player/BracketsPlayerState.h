// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BracketsPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BRACKETS_API ABracketsPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

	void AddToScore(float ScoreAmount);
	void SetHasTakenDamage(bool TakenDamge);
	void SetIsDead(bool IsDead);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUpdateLeaderboard();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetIsDead(bool IsDead);

protected:
	UPROPERTY(/*ReplicatedUsing = OnRep_Defeats*/)
	int32 Defeats;
	UPROPERTY(Replicated)
	bool bHasTakenDamage = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDead = false;

private:
	UPROPERTY()
	class ABracketsCharacter* Character;
	UPROPERTY()
	class ABracketsPlayerController* Controller;
	UPROPERTY()
	int32 CurrentRound = 0;
	UPROPERTY()
	TArray<APlayerState*>PlayerStateArray;

public:
	FORCEINLINE bool GetIsDead() { return bIsDead; }

};
