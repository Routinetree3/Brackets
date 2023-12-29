// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BracketsPlayerController.generated.h"

class AThrowableProjectile;
class ABracketsCharacterHUD;
class UCharacterHUDWidget;
class ABracketsSinglesGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingToHigh);

UCLASS()
class BRACKETS_API ABracketsPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HandleMatchHasStarted();
	void HandleControllerRoundStart();
	void HandleControllerRoundInProgress();
	void HandleControllerRoundEnd();
	void HandleControllerMatchEnd();

	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDAmmoCarried(int32 Ammo);

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDMatchCountdown(float Countdown);
	void SetHUDSelectionCountdown(float Countdown);
	//False means defeated, True means Alive/Victory
	void SetHUDRoundEnd(bool bHasWon);
	void ShowHUDEscMenu(bool Pressed);
	void ShowHUDLeaderBoard(bool Show, bool ShowMouse);
	void SetLethalHUDIcon(TArray<TSubclassOf<AThrowableProjectile>> LethalArray);
	void SetNonLethalHUDIcon(TArray<TSubclassOf<AThrowableProjectile>> NonLethalArray);
	void SetPrimaryHUDIcon(UTexture2D* Silhouette);
	void SetSecondaryHUDIcon(UTexture2D* Silhouette);
	void RemoveHUDWeaponIcons();
	void AddOvertime(float OvertimeAmount);

	UFUNCTION(BlueprintImplementableEvent)
	void StartCameraShake();

	void SetIsDead();

	void OnMatchSateSet(FName State);
	virtual void OnPossess(APawn* InPawn) override;
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	// Sync time between client and server

	// Requests the current server time, passing in the clients time when requested
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	// Reports the current Server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecivedClientRequest);

	UFUNCTION(Client, Unreliable)
	void ClientSetHUDRoundEnd(bool bHasWon);

	float ClientServerDelta = 0.f; //Diffrence between client and server time
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	UPROPERTY(Replicated)
	bool bIsDeadCPP = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDead = false;
	void ResetTime();
	void ResetStats();

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float MatchEnd, float StartingTime);
	
	UPROPERTY(EditAnywhere)
		float CheckPingFrequency = 10.f;
	UPROPERTY(EditAnywhere)
		float HighPingThreshold = 120.f;

	UPROPERTY(EditAnywhere)
		float HighPingDuration = 5.f;

	float HighPingRunningTime = 0.f;

	UFUNCTION()
	void CheckPing(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

private:
	UPROPERTY()
	ABracketsCharacterHUD* BracketsCharacterHUD;
	UPROPERTY()
	UCharacterHUDWidget* CharacterHUDWidget;
	UPROPERTY()
	ABracketsSinglesGameMode* BracketsSinglesGameMode;

	bool bInitializeCharacterHUDWidget = false;

	UFUNCTION()
		void OnRep_MatchState();
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;

	bool bOnce = true;
	float LoadingTime = 0.f;
	bool bfirstRound = true;
	float PrepaireTime = 0.f;
	float RoundTime = 0.f;
	float CooldownTime = 0.f;
	float MatchEndTime = 0.f;
	float PreveusTimeElapsed = 0.f;
	float LevelStartingTime = 0.f;
	float CountdownTime = 0.f;
	float PrepaireCountdown = 0.f;
	float RoundCountdown = 0.f;
	float CoolCountdown = 0.f;
	float MatchEndCountdown = 0.f;
	uint32 CountDownInt = 0;

	UPROPERTY(Replicated)
	float OverTime = 0.f;

	float HUDHealth;
	float HUDMaxHealth;
	UPROPERTY(EditDefaultsOnly)
	FString VictoryText = "[VICTORY]";
	UPROPERTY(EditDefaultsOnly)
	FString DefeatText = "[DEFEAT]";
	bool bWidgetWasVisible = false;
	float StateTimeleft = 0;

	public:
	FORCEINLINE bool GetbIsDead() { return bIsDead; }
	FORCEINLINE bool GetbIsDeadCPP() { return bIsDeadCPP; }
};
