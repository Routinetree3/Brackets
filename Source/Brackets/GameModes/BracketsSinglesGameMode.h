// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BracketsSinglesGameMode.generated.h"

namespace MatchState
{
	extern BRACKETS_API const FName RoundStart; //Spawn Character, Gear and Weapons Select, Gameplay Disabled
	extern BRACKETS_API const FName RoundInProgress; //Game Round is in progress, HUD enabled, Gamplay enabled
	extern BRACKETS_API const FName RoundEnd; //HUD/Gameplay disabled, Check what Characters are dead, Heal all alive Characters, Organize Next round
	extern BRACKETS_API const FName RoundTransition; 
	extern BRACKETS_API const FName MatchEnd; //Will Handle the End Podium
}


UCLASS()
class BRACKETS_API ABracketsSinglesGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABracketsSinglesGameMode();
	virtual void Tick(float DeltaTime) override;

	virtual void PlayerEliminated(class ABracketsCharacter* ElimmedCharacter, class ABracketsPlayerController* VictimController, ABracketsPlayerController* AttackerController);


protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	virtual void GenericPlayerInitialization(AController* C) override;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<APlayerController*>ControllerArray;*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AController*>ControllerArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ABracketsPlayerController*>BracketsControllerArray;

	void CalculateTime();

	virtual void HandleRoundStart();
	virtual void HandleRoundInProgress();
	virtual void HandleRoundEnd();
	virtual void HandleRoundTransition();
	UFUNCTION(BlueprintCallable)
	virtual void HandleMatchEnd();
	void HandleUpdatingLeaderBoard();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void HandleMatchInitilization();

	virtual void HandleStateSwitch(FName NewState);
	void CheckPlayersHealth(bool IsEndRound);

	UPROPERTY(EditDefaultsOnly)
		float PrepaireTime = 10.f; //likly will end up being aound 20-30
	UPROPERTY(EditDefaultsOnly)
		float RoundTime = 20.f; //Roughly 2-5 min
	UPROPERTY(EditDefaultsOnly)
		float AvalableOvertime = 0.f; //If Needed
		float Overtime = 0.f;
	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 5.f;
	UPROPERTY(EditDefaultsOnly)
		float MatchEndTime = 10.f;
	float LevelStartingTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
		int32 RoundXP = 25;

	UPROPERTY(EditDefaultsOnly)
	int32 RoundCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 PlayerLimit = 4;

	void AddXP();
	void ResetTime();
	void SetOvertime();

private:
	float CountdownTime = 0.f;
	float TimeElapsed = 0.f;
	float PreveusTimeElapsed = 0.f;
	float LoadingTime = 0.f;
	bool bfirstRound = true;
	bool bFirstKill = true;
	bool bIsDraw = false;

	//bool for reset time
	bool bOnce = true;
	//bool for checking health once in tick
	bool bCheckOnce = true;

	int32 ControllerArrayLength;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	FORCEINLINE float GetTimeElapsed() const { return TimeElapsed; }
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }
	FORCEINLINE float GetPrepaireTime() const { return PrepaireTime; }
	FORCEINLINE float GetRoundTime() const { return RoundTime; }
	FORCEINLINE float GetCooldownTime() const { return CooldownTime; }
	FORCEINLINE float GetMatchEndTime() const { return MatchEndTime; }
	TArray<AController*> GetControllerArray() const { return ControllerArray; }
	int32 GetRoundCount() { return RoundCount; }
};
