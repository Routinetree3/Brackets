// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyUserWidget;

UCLASS()
class BRACKETS_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
		
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(Client, Reliable)
		void ClientJoinMidgame(float ReadyTime);
	UFUNCTION()
		void SetStartCountdown(bool ShouldStartCountdown);

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void AddLobbyWidget();
	void SetLobbyTime(float Countdown);

	void ResetTime();
	void SetHUDTime();
	float GetServerTime();
	virtual void ReceivedPlayer() override;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	// Requests the current server time, passing in the clients time when requested
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);
	// Reports the current Server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecivedClientRequest);

	float ClientServerDelta = 0.f; //Diffrence between client and server time
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float ReadyCountdownTime = 20.f;
	float PreveusTimeElapsed = 0.f;
	float CountdownTime = 0.f;
	float TimeElapsed = 0.f;
	UPROPERTY(Replicated)
	bool bStartCountdown = false;
	bool bVisibilityOnce = true;

	UPROPERTY(BlueprintReadOnly)
		ULobbyUserWidget* LobbyWidget;
	UPROPERTY(BlueprintReadOnly)
		class AMenuHUD* MenuHUD;

	UPROPERTY(EditAnywhere, Category = "LobbyWidget")
		TSubclassOf<class UUserWidget> LobbyWidgetClass;
	
};
