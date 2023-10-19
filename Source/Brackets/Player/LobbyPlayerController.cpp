// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "Brackets/HUD/Menu/LobbyUserWidget.h"
#include "Brackets/GameModes/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerController, bStartCountdown);
}

void ALobbyPlayerController::ClientJoinMidgame_Implementation(float ReadyTime)
{
	ReadyCountdownTime = ReadyTime;
}

void ALobbyPlayerController::SetStartCountdown(bool ShouldStartCountdown)
{
	bStartCountdown = ShouldStartCountdown;
}

void ALobbyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckTimeSync(DeltaTime);
	if (bStartCountdown)
	{
		if (LobbyWidget && !bVisibilityOnce)
		{
			LobbyWidget->TimeText->SetVisibility(ESlateVisibility::Visible);
			bVisibilityOnce = true;
			ResetTime();
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Yellow, TEXT("bStartCountdown"));
			}
		}
		SetHUDTime();
	}
	else
	{
		ResetTime();
		if (LobbyWidget && bVisibilityOnce)
		{
			LobbyWidget->TimeText->SetVisibility(ESlateVisibility::Hidden);
			bVisibilityOnce = false;
		}
	}
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyPlayerController::AddLobbyWidget()
{
	LobbyWidget = CreateWidget<ULobbyUserWidget>(this, LobbyWidgetClass);
	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport();
	}
}

void ALobbyPlayerController::SetLobbyTime(float Countdown)
{
	if (LobbyWidget)
	{
		if (Countdown < 0.f)
		{
			LobbyWidget->TimeText->SetText(FText::FromString("00:00"));
			return;
		}
		int32 Minutes = FMath::FloorToInt(Countdown / 60.f);
		int32 Seconds = Countdown - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Blue, FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
		}*/
		LobbyWidget->TimeText->SetText(FText::FromString(CountdownText));
	}
}

void ALobbyPlayerController::ResetTime()
{
	PreveusTimeElapsed = GetServerTime();
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Purple, TEXT("ResetTime"));
	}
}

void ALobbyPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = 0;

	CountdownTime = (ReadyCountdownTime + PreveusTimeElapsed) - GetServerTime();
	UE_LOG(LogTemp, Warning, TEXT("CountdownTime: %f"), CountdownTime);
	SecondsLeft = FMath::CeilToInt(CountdownTime);

	if (0 != SecondsLeft)
	{
		SetLobbyTime(SecondsLeft);
	}
}

float ALobbyPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ALobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ALobbyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ALobbyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ALobbyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerRecivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
