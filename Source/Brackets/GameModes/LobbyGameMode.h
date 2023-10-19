// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"


UCLASS()
class BRACKETS_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 PlayerLimit = 1;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable)
	void TravelToGame(FString LevelLocation); //Called in Blueprints

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void OnMatchStateSet() override;
	void CalculateTime();
	void ResetTime();

	UFUNCTION(BlueprintImplementableEvent)
	void ReadyCountdownFinished();

	UFUNCTION(BlueprintImplementableEvent)
	void AllPlayerReadyForGame();

	UPROPERTY(BlueprintReadWrite)
		int32 NumberOfPlayersReady = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FString MatchLevelLocation = "/Game/Levels/BracketsSinglesPERLevel";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FString MenuLevelLocation = "/Game/Levels/Menu";

private:
	UPROPERTY(EditDefaultsOnly)
	float ReadyCountdownTime = 20.f;
	float PreveusTimeElapsed = 0.f;
	float CountdownTime = 0.f;
	float TimeElapsed = 0.f;

	bool bCount = true;
	bool bOnce = true;

public:
	FORCEINLINE int32 GetNumberOfPlayersReady() const { return NumberOfPlayersReady; }
	FORCEINLINE int32 GetPlayerLimit() const { return PlayerLimit; }

};
