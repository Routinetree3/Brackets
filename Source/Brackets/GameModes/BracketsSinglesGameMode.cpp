// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsSinglesGameMode.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Brackets/Player/BracketsPlayerState.h"

#include "Brackets/HUD/BracketsCharacterHUD.h"
#include "Brackets/HUD/RoundEndWidget.h"

#include "Brackets/GameStates/BracketsSinglesGameState.h"
#include "Brackets/BracketsCharacter.h"

#include "Brackets/GameInstance/BracketsGameInstance.h"

namespace MatchState
{
	const FName RoundStart = FName("RoundStart"); //Spawn Character, Gear and Weapons Select, Gameplay Disabled
	const FName RoundInProgress = FName("RoundInProgress"); //Game Round is in progress, HUD enabled, Gamplay enabled
	const FName RoundEnd = FName("RoundEnd"); //HUD/Gameplay disabled, Check what Characters are dead, Heal all alive Characters, Organize Next round. //Handles the XP asignment for the previus round
	const FName RoundTransition = FName("RoundTransition");
	const FName MatchEnd = FName("MatchEnd"); //Will Handle the End Podium and the awarding of Prizes
}

ABracketsSinglesGameMode::ABracketsSinglesGameMode()
{
	bDelayedStart = true;
}

void ABracketsSinglesGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABracketsSinglesGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CalculateTime();
}

void ABracketsSinglesGameMode::CalculateTime()
{
	TimeElapsed = GetWorld()->GetTimeSeconds();

	if (MatchState == MatchState::WaitingToStart)
	{
		if (PlayerLimit == NumPlayers && TimeElapsed > 3)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		LoadingTime = TimeElapsed;
	}
	else if (MatchState == MatchState::RoundStart)
	{
		if (bOnce)
		{
			ResetTime();;
		}
		CountdownTime = (PrepaireTime + PreveusTimeElapsed) - (TimeElapsed);
		//UE_LOG(LogTemp, Error, TEXT("CountdownTime: %f"), CountdownTime);
		//UE_LOG(LogTemp, Warning, TEXT("TimeElapsed: %f"), TimeElapsed);
		//UE_LOG(LogTemp, Warning, TEXT("LevelStartingTime: %f"), LevelStartingTime);

		//if (GEngine)
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%f, %f, %f, "), CountdownTime, TimeElapsed, LevelStartingTime));
		if (CountdownTime < 0.f && CountdownTime > -15.f)
		{
			HandleStateSwitch(MatchState::RoundInProgress);
			CountdownTime = 0.1f; 
			bOnce = true;
		}
	}
	else if (MatchState == MatchState::RoundInProgress)
	{
		if (bOnce)
		{
			ResetTime();
		}
		//CountdownTime = (RoundTime + PreveusTimeElapsed) - (TimeElapsed + LevelStartingTime); // old way
		CountdownTime = (RoundTime + PreveusTimeElapsed + Overtime) - (TimeElapsed);
		//UE_LOG(LogTemp, Error, TEXT("CountdownTime: %f"), CountdownTime);
		//UE_LOG(LogTemp, Warning, TEXT("TimeElapsed: %f"), TimeElapsed);
		//UE_LOG(LogTemp, Warning, TEXT("LevelStartingTime: %f"), LevelStartingTime);

		//if (GEngine)
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%f, %f, %f, "), CountdownTime, TimeElapsed, LevelStartingTime));

		if (CountdownTime < 5.f && bCheckOnce && AvalableOvertime > 0)
		{
			CheckPlayersHealth(false);
			bCheckOnce = false;
		}

		if (CountdownTime < 0.f)
		{
			HandleStateSwitch(MatchState::RoundEnd);
			CountdownTime = 0.1f;
			bOnce = true;
		}
	}
	else if (MatchState == MatchState::RoundEnd)
	{
		if (bOnce)
		{
			ResetTime();
		}
		CountdownTime = (CooldownTime + PreveusTimeElapsed) - (TimeElapsed);
		//UE_LOG(LogTemp, Error, TEXT("CountdownTime: %f"), CountdownTime);
		//UE_LOG(LogTemp, Warning, TEXT("TimeElapsed: %f"), TimeElapsed);
		//UE_LOG(LogTemp, Warning, TEXT("PreveusTimeElapsed: %f"), PreveusTimeElapsed);
		if (CountdownTime < -0.f)
		{
			if (RoundCount > 0)
			{
				HandleStateSwitch(MatchState::RoundStart);
				CountdownTime = 0.1f;
				if (bfirstRound) { bfirstRound = false; }
				bOnce = true;
			}
			else
			{
				SetMatchState(MatchState::MatchEnd);
			}
		}
	}
	else if (MatchState == MatchState::MatchEnd)
	{
		if (bOnce)
		{
			ResetTime();
		}
		CountdownTime = (MatchEndTime + PreveusTimeElapsed) - (TimeElapsed);

		if (CountdownTime < 0.f)
		{
			SetMatchState(MatchState::WaitingPostMatch);
			bOnce = true;
		}
	}
}

void ABracketsSinglesGameMode::ResetTime()
{
	PreveusTimeElapsed = GetWorld()->GetTimeSeconds();
	bOnce = false;
}

void ABracketsSinglesGameMode::HandleStateSwitch(FName NewState)
{
	SetMatchState(NewState);
}

void ABracketsSinglesGameMode::HandleMatchInitilization()
{
	SetMatchState(MatchState::RoundStart);
}

void ABracketsSinglesGameMode::HandleRoundStart()
{
	ABracketsSinglesGameState* BracketsGameState = GetGameState<ABracketsSinglesGameState>();
	if (BracketsGameState)
	{
		BracketsGameState->SetCurrentRound(RoundCount);
	}
	HandleUpdatingLeaderBoard();
	ControllerArrayLength = ControllerArray.Num();
	Overtime = 0.f;
	bCheckOnce = true;
}

void ABracketsSinglesGameMode::HandleRoundInProgress()
{

}

void ABracketsSinglesGameMode::HandleRoundEnd()
{
	CheckPlayersHealth(true);
	HandleUpdatingLeaderBoard();
	AddXP();

	RoundCount--;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Round Count:%d"), RoundCount)
		);
	}
}

void ABracketsSinglesGameMode::HandleRoundTransition()
{

}

void ABracketsSinglesGameMode::HandleMatchEnd()
{

}

void ABracketsSinglesGameMode::CheckPlayersHealth(bool IsEndRound)
{
	bool bNeedOvertime = false;
	int32 Length = ControllerArray.Num();
	int32 Itteration = 0;
	int32 Controller1 = 0;
	int32 Controller2 = 1;

	while (Itteration < Length / 2)
	{
		//UE_LOG(LogTemp, Error, TEXT("%d"),Itteration);
		//UE_LOG(LogTemp, Warning, TEXT("%d"), Controller1);
		//UE_LOG(LogTemp, Warning, TEXT("%d"), Controller2);
		ABracketsPlayerController* BracketsController1 = Cast<ABracketsPlayerController>(ControllerArray[Controller1]);
		ABracketsPlayerController* BracketsController2 = Cast<ABracketsPlayerController>(ControllerArray[Controller2]);
		ABracketsSinglesGameState* BracketsGameState = GetGameState<ABracketsSinglesGameState>();
		if (BracketsController1 && BracketsController2 && BracketsGameState)
		{
			ABracketsCharacter* BracketsCharacter1 = Cast<ABracketsCharacter>(BracketsController1->GetCharacter());
			ABracketsCharacter* BracketsCharacter2 = Cast<ABracketsCharacter>(BracketsController2->GetCharacter());
			ABracketsPlayerState* BracketsPlayerState1 = Cast<ABracketsPlayerState>(BracketsController1->PlayerState);
			ABracketsPlayerState* BracketsPlayerState2 = Cast<ABracketsPlayerState>(BracketsController2->PlayerState);

			if (BracketsCharacter1->GetHealth() > BracketsCharacter2->GetHealth())
			{
				if (IsEndRound)
				{
					BracketsController1->SetHUDRoundEnd(true);
					BracketsController2->SetIsDead();
					BracketsController2->SetHUDRoundEnd(false);
					BracketsCharacter2->Eliminated();

					//BracketsCharacter1->GetCombat()->DropEquippedWeapon();
					//BracketsCharacter2->GetCombat()->DropEquippedWeapon();
					ControllerArray.RemoveAt(Controller2);
				}
				else if (BracketsPlayerState1->GetIsDead() == false && BracketsPlayerState2->GetIsDead() == false && !IsEndRound)
				{
					bNeedOvertime = true;
				}
				
			}
			else if (BracketsCharacter1->GetHealth() < BracketsCharacter2->GetHealth())
			{
				if (IsEndRound)
				{
					BracketsController1->SetHUDRoundEnd(false);
					BracketsController2->SetHUDRoundEnd(true);
					BracketsController1->SetIsDead();
					BracketsCharacter1->Eliminated();

					ControllerArray.RemoveAt(Controller1);
				}
				else if (BracketsPlayerState1->GetIsDead() == false && BracketsPlayerState2->GetIsDead() == false && !IsEndRound)
				{
					bNeedOvertime = true;
				}

			}
			else if (BracketsCharacter1->GetHealth() == BracketsCharacter2->GetHealth())
			{
				bNeedOvertime = true;

				if (IsEndRound)
				{
					bool Coinflip = FMath::RandBool();

					if (Coinflip)
					{
						BracketsController1->SetHUDRoundEnd(false);
						BracketsController2->SetHUDRoundEnd(true);
						BracketsController1->SetIsDead();
						BracketsCharacter1->Eliminated();

						ControllerArray.RemoveAt(Controller1);
						UE_LOG(LogTemp, Error, TEXT("Heads"));
					}
					else
					{
						BracketsController1->SetHUDRoundEnd(true);
						BracketsController2->SetIsDead();
						BracketsController2->SetHUDRoundEnd(false);
						BracketsCharacter2->Eliminated();

						ControllerArray.RemoveAt(Controller2);
						UE_LOG(LogTemp, Error, TEXT("Tails"));
					}
				}

				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						15.f,
						FColor::Red,
						FString::Printf(TEXT("Draw"))
					);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Default"));
			}
		}
		Controller1 = Controller1 + 1;
		Controller2 = Controller2 + 1;
		Controller1 = FMath::Clamp(Controller1, 0, Length - 1);
		Controller2 = FMath::Clamp(Controller2, 0, Length);
		Itteration++;
	}

	if (!IsEndRound && bNeedOvertime)
	{
		SetOvertime();
	}
}

void ABracketsSinglesGameMode::SetOvertime()
{
	Overtime = AvalableOvertime;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABracketsPlayerController* BracketsPlayer = Cast<ABracketsPlayerController>(*It);
		if (BracketsPlayer)
		{
			BracketsPlayer->AddOvertime(Overtime);
		}
	}
}

//Take the left over Contoller array (With all dead Controllers removed) and itterate though them adding the necissary XP
void ABracketsSinglesGameMode::AddXP()
{
	int32 Length = ControllerArray.Num();
	int32 Itteration = 0;

	for (Itteration = 0; Itteration < Length; Itteration++)
	{
		ABracketsPlayerController* BracketsController = Cast<ABracketsPlayerController>(ControllerArray[Itteration]);
		if (BracketsController)
		{
			int32 XPThatWasSet = 0;
			ABracketsPlayerState* BracketsPlayerState = Cast<ABracketsPlayerState>(BracketsController->PlayerState);
			if (BracketsPlayerState && BracketsController)
			{
				switch (RoundCount)
				{
				case 0: // End of game

					break;
				case 1: // last round
					BracketsPlayerState->AddToScore(RoundXP * 4);
					XPThatWasSet = RoundXP * 4;
					break;
				case 2:
					BracketsPlayerState->AddToScore(RoundXP * 2);
					XPThatWasSet = RoundXP * 2;
					break;
				case 3:
					BracketsPlayerState->AddToScore(RoundXP);
					XPThatWasSet = RoundXP;
					break;
				case 4:
					BracketsPlayerState->AddToScore(RoundXP);
					XPThatWasSet = RoundXP;
					break;
				default:
					break;
				}
			}
			UBracketsGameInstance* GameInstance = Cast<UBracketsGameInstance>(BracketsController->GetGameInstance());
			if (GameInstance)
			{
				UE_LOG(LogTemp, Error, TEXT("GameInstance"));
				GameInstance->SetXP(XPThatWasSet);
			}
		}
	}
}

void ABracketsSinglesGameMode::OnMatchStateSet()
{
	FGameModeEvents::OnGameModeMatchStateSetEvent().Broadcast(MatchState);
	
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchIsWaitingToStart();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::RoundStart)
	{
		HandleRoundStart();
	}
	else if (MatchState == MatchState::RoundInProgress)
	{
		HandleRoundInProgress();
	}
	else if (MatchState == MatchState::RoundEnd)
	{
		HandleRoundEnd();
	}
	else if (MatchState == MatchState::RoundTransition)
	{
		HandleRoundTransition();
	}
	else if (MatchState == MatchState::MatchEnd)
	{
		HandleMatchEnd();
	}
	else if (MatchState == MatchState::WaitingPostMatch)
	{
		HandleMatchHasEnded();
	}
	else if (MatchState == MatchState::LeavingMap)
	{
		HandleLeavingMap();
	}
	else if (MatchState == MatchState::Aborted)
	{
		HandleMatchAborted();
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABracketsPlayerController* BracketsPlayer = Cast<ABracketsPlayerController>(*It);
		if (BracketsPlayer)
		{
			BracketsPlayer->OnMatchSateSet(MatchState);
		}
	}
}

void ABracketsSinglesGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);
	ControllerArray.Add(C);
	int32 TempLength = ControllerArray.Num();
	UE_LOG(LogTemp, Error, TEXT("Length: %d"), TempLength);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Length: %d"), TempLength)
		);
	}
	ABracketsSinglesGameState* BracketsGameState = GetGameState<ABracketsSinglesGameState>();
	if (BracketsGameState)
	{
		BracketsGameState->SetCurrentRound(RoundCount); // keep this line
	}
}

void ABracketsSinglesGameMode::PlayerEliminated(ABracketsCharacter* ElimmedCharacter, ABracketsPlayerController* VictimController, ABracketsPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr)
	{
		return;
	}
	if (VictimController == nullptr || VictimController->PlayerState == nullptr)
	{
		return;
	}
	ABracketsPlayerState* AttackerPlayerState = AttackerController ? Cast<ABracketsPlayerState>(AttackerController->PlayerState) : nullptr;
	ABracketsPlayerState* VictumPlayerState = VictimController ? Cast<ABracketsPlayerState>(VictimController->PlayerState) : nullptr;

	ABracketsSinglesGameState* BracketsSinglesGameState = GetGameState<ABracketsSinglesGameState>();
	ControllerArrayLength--;
	if (bFirstKill)
	{
		BracketsSinglesGameState->FirstPlayerKill(AttackerPlayerState);
		bFirstKill = false;
	}

	if (AttackerPlayerState && AttackerPlayerState != VictumPlayerState && BracketsSinglesGameState)
	{
		///AttackerPlayerState->AddToScore(1.f); //Kinda works for now
	}
	if (VictumPlayerState)
	{
		//VictumPlayerState->AddToDefeats(1);
		VictumPlayerState->SetIsDead(true);
	}
	if (AttackerController)
	{
		AttackerController->SetHUDRoundEnd(true);
	}
	if (VictimController)
	{
		VictimController->SetHUDRoundEnd(false);
	}
	HandleUpdatingLeaderBoard();
	ControllerArrayLength--;

	UE_LOG(LogTemp, Warning, TEXT("ControllerArrayLength: %d"), ControllerArrayLength)
	UE_LOG(LogTemp, Warning, TEXT("ControllerArray.Num(): %d"), ControllerArray.Num()/2)

	if (ControllerArrayLength < ControllerArray.Num() / 2)
	{
		SetMatchState(MatchState::RoundEnd);
		bOnce = true;
	}
}

void ABracketsSinglesGameMode::HandleUpdatingLeaderBoard()
{
	ABracketsSinglesGameState* BracketsSinglesGameState = GetGameState<ABracketsSinglesGameState>();
	if (BracketsSinglesGameState)
	{
		BracketsSinglesGameState->UpdateLeaderboard();
	}
}