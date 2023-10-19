// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Brackets/Player/LobbyPlayerController.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALobbyPlayerController* LobbyController = Cast<ALobbyPlayerController>(NewPlayer);
	if (LobbyController)
	{
		LobbyController->ClientJoinMidgame(ReadyCountdownTime);
	}
}

void ALobbyGameMode::TravelToGame(FString LevelLocation)
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel((FString("%s?listen"), LevelLocation));
	}
}

void ALobbyGameMode::Tick(float DeltaTime)
{
	if (NumPlayers == PlayerLimit && bCount)
	{
		if (PlayerLimit == NumberOfPlayersReady)
		{
			bCount = false;
			AllPlayerReadyForGame();
		}
		if (bOnce)
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(*It);
				if (LobbyPlayerController)
				{
					LobbyPlayerController->SetStartCountdown(true);
					UE_LOG(LogTemp, Warning, TEXT("SetStartCountdown(true)"));
				}
			}
			bOnce = false;
		}
		CalculateTime();
	}
	else
	{
		ResetTime();
		if (!bOnce)
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(*It);
				if (LobbyPlayerController)
				{
					LobbyPlayerController->SetStartCountdown(false);
					UE_LOG(LogTemp, Warning, TEXT("SetStartCountdown(false)"));
				}
			}
			bOnce = true;
		}
	}
}

void ALobbyGameMode::CalculateTime()
{
	TimeElapsed = GetWorld()->GetTimeSeconds();

	CountdownTime = (ReadyCountdownTime + PreveusTimeElapsed) - TimeElapsed;
	UE_LOG(LogTemp, Error, TEXT("CountdownTime: %f"), CountdownTime);
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 10.f, FColor::Green, FString::Printf(TEXT("%f"), CountdownTime));
	}*/
	if (CountdownTime < -0.f)
	{
		ReadyCountdownFinished();
	}
}

void ALobbyGameMode::ResetTime()
{
	PreveusTimeElapsed = GetWorld()->GetTimeSeconds();
	UE_LOG(LogTemp, Warning, TEXT("ResetTime"));
}

void ALobbyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	ResetTime();
}
