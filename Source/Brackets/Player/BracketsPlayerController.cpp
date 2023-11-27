// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsPlayerController.h"
#include "Brackets/HUD/BracketsCharacterHUD.h"
#include "Brackets/HUD/CharacterHUDWidget.h"
#include "Brackets/HUD/EquipmentSelectionWidget.h"
#include "Brackets/HUD/RoundEndWidget.h"
#include "Brackets/HUD/InGameMenuWidget.h"
#include "Brackets/HUD/LeaderBoardWidget.h"
#include "Brackets/HUD/MatchStartWidget.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

#include "Kismet/GameplayStatics.h"
#include "Brackets/Components/CombatComponent.h"
#include "Brackets/Weapons/Weapon.h"
#include "Brackets/Weapons/ThrowableProjectile.h"

#include "Brackets/BracketsCharacter.h"
#include "Brackets/Player/BracketsPlayerState.h"
#include "Brackets/GameModes/BracketsSinglesGameMode.h"
#include "Brackets/GameStates/BracketsSinglesGameState.h"
#include "Brackets/Character/BracketsAnimInstance.h"
#include "Gameframework/CharacterMovementComponent.h"

#include "Net/UnrealNetwork.h"


void ABracketsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BracketsCharacterHUD = Cast<ABracketsCharacterHUD>(GetHUD());
	if (BracketsCharacterHUD)
	{
		HandleMatchHasStarted();
	}
	ServerCheckMatchState();
}

void ABracketsPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckTimeSync(DeltaTime);
	SetHUDTime();
	PollInit();
}

void ABracketsPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABracketsPlayerController, MatchState);
	DOREPLIFETIME(ABracketsPlayerController, OverTime);
	DOREPLIFETIME(ABracketsPlayerController, bIsDeadCPP);
}

void ABracketsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
	if (BracketsCharacter)
	{
		HUDHealth = BracketsCharacter->GetHealth();
		HUDMaxHealth = BracketsCharacter->GetMaxHealth();
		SetHUDHealth(HUDHealth, HUDMaxHealth);
	}
}

void ABracketsPlayerController::PollInit()
{
	if (CharacterHUDWidget == nullptr)
	{
		if (BracketsCharacterHUD && BracketsCharacterHUD->CharacterHUDWidget)
		{
			CharacterHUDWidget = BracketsCharacterHUD->CharacterHUDWidget;
			if (CharacterHUDWidget && MatchState == MatchState::RoundInProgress)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				//SetHUDScore(HUDScore);
				//SetHUDDefeats(HUDDefeats);
			}
		}
	}

}

void ABracketsPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABracketsPlayerController::ServerCheckMatchState_Implementation()
{
	BracketsSinglesGameMode = BracketsSinglesGameMode == nullptr ? Cast<ABracketsSinglesGameMode>(UGameplayStatics::GetGameMode(this)) : BracketsSinglesGameMode;

	if (BracketsSinglesGameMode)
	{
		PrepaireTime = BracketsSinglesGameMode->GetPrepaireTime();
		RoundTime = BracketsSinglesGameMode->GetRoundTime();
		CooldownTime = BracketsSinglesGameMode->GetCooldownTime();		
		MatchEndTime = BracketsSinglesGameMode->GetMatchEndTime();
		LevelStartingTime = BracketsSinglesGameMode->GetLevelStartingTime();

		MatchState = BracketsSinglesGameMode->GetMatchState();
		ClientJoinMidgame(MatchState, PrepaireTime, RoundTime, CooldownTime, MatchEndTime, LevelStartingTime);
	}
}

void ABracketsPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Prepaire, float Round, float Cooldown, float MatchEnd, float StartingTime)
{
	PrepaireTime = Prepaire;
	RoundTime = Round;
	CooldownTime = Cooldown;
	MatchEndTime = 
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
}

void ABracketsPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);

}

void ABracketsPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerRecivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABracketsPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABracketsPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ABracketsPlayerController::HandleMatchHasStarted()
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	if (BracketsCharacterHUD)
	{
		if (BracketsCharacterHUD->CharacterHUDWidget == nullptr)
		{
			BracketsCharacterHUD->AddCharacterHUDWidget();
			BracketsCharacterHUD->CharacterHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		if (BracketsCharacterHUD->InGameMenuWidget == nullptr)
		{
			BracketsCharacterHUD->AddInGameMenuWidget();
			if (BracketsCharacterHUD->InGameMenuWidget)
			{
				BracketsCharacterHUD->InGameMenuWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		if (BracketsCharacterHUD->LeaderBoardWidget == nullptr)
		{
			BracketsCharacterHUD->AddLeaderBoardWidget();
			if (BracketsCharacterHUD->LeaderBoardWidget)
			{
				BracketsCharacterHUD->LeaderBoardWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		if (BracketsCharacterHUD->RoundEndWidget == nullptr)
		{
			BracketsCharacterHUD->AddRoundEndWidget();
			if (BracketsCharacterHUD->RoundEndWidget)
			{
				BracketsCharacterHUD->RoundEndWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void ABracketsPlayerController::HandleControllerRoundStart()
{
	OverTime = 0.f;
	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
	ABracketsPlayerState* BracketsPlayerState = GetPlayerState<ABracketsPlayerState>();

	if (BracketsCharacter)
	{
		BracketsCharacter->GetCharacterMovement()->DisableMovement();

		if (BracketsCharacter->GetCombat())
		{
			BracketsCharacter->GetCombat()->DropEquippedWeapon(true);
			BracketsCharacter->GetCombat()->DropEquippedWeapon(false);
			BracketsCharacter->GetCombat()->ClearThrowables();
		}
	}

	if (BracketsCharacter && !BracketsPlayerState->GetIsDead())
	{
		BracketsCharacter->bDisableGameplay = true;
	}
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	if (BracketsCharacterHUD && BracketsCharacterHUD->CharacterHUDWidget)
	{
		BracketsCharacterHUD->CharacterHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (BracketsCharacterHUD && BracketsCharacterHUD->RoundEndWidget)
	{
		BracketsCharacterHUD->RoundEndWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (BracketsCharacterHUD && BracketsCharacterHUD->LeaderBoardWidget)
	{
		ShowHUDLeaderBoard(false, false);
		if (bfirstRound)
		{
			BracketsCharacterHUD->LeaderBoardWidget->SetPlayerNameAndPlace();
		}
	}
	if (BracketsCharacterHUD && !BracketsPlayerState->GetIsDead())
	{
		BracketsCharacterHUD->AddEquipmentSelectWidget();
		SetShowMouseCursor(true);
	}
}

void ABracketsPlayerController::HandleControllerRoundInProgress()
{
	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
	if (BracketsCharacter)
	{
		BracketsCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		BracketsCharacter->bDisableGameplay = false;
	}

	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;

	if (BracketsCharacterHUD)
	{
		if (BracketsCharacterHUD->EquipmentSelectionWidget)
		{
			BracketsCharacterHUD->RemoveEquipmentSelectWidget();
		}
		ABracketsPlayerState* BracketsPlayerState = GetPlayerState<ABracketsPlayerState>();
		if (BracketsCharacterHUD->CharacterHUDWidget && !BracketsPlayerState->GetIsDead())
		{
			BracketsCharacterHUD->CharacterHUDWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ABracketsPlayerController::HandleControllerRoundEnd()
{
	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
	if (BracketsCharacter)
	{
		BracketsCharacter->bDisableGameplay = true;
	}
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;

	if (BracketsCharacterHUD)
	{
		if (BracketsCharacterHUD->CharacterHUDWidget)
		{
			BracketsCharacterHUD->CharacterHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABracketsPlayerController::HandleControllerMatchEnd()
{

}

void ABracketsPlayerController::SetHUDTime()
{
	if (HasAuthority())
	{
		BracketsSinglesGameMode = BracketsSinglesGameMode == nullptr ? Cast<ABracketsSinglesGameMode>(UGameplayStatics::GetGameMode(this)) : BracketsSinglesGameMode;
		if (BracketsSinglesGameMode)
		{
			LevelStartingTime = BracketsSinglesGameMode->GetLevelStartingTime();
		}
	}

	uint32 SecondsLeft = 0;

	if (MatchState == MatchState::InProgress)
	{
		LoadingTime = GetServerTime();
	}
	else if (MatchState == MatchState::RoundStart)
	{
		if (bOnce)
		{
			ResetTime();
		}
		//PrepaireCountdown = (PrepaireTime + PreveusTimeElapsed ) - (GetServerTime() + LevelStartingTime); //old way
		PrepaireCountdown = (PrepaireTime + PreveusTimeElapsed) - GetServerTime();
		//UE_LOG(LogTemp, Error, TEXT("PrepaireCountdown: %f"), PrepaireCountdown);
		//UE_LOG(LogTemp, Warning, TEXT("LevelStartingTime: %f"), LevelStartingTime);
		//UE_LOG(LogTemp, Warning, TEXT("PreveusTimeElapsed: %f"), PreveusTimeElapsed);
		//UE_LOG(LogTemp, Warning, TEXT("LoadingTime: %f"), LoadingTime);

		//if (GEngine)
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("%f, %f, %f, %f, %f"), PrepaireCountdown, LevelStartingTime, PreveusTimeElapsed, LoadingTime, GetServerTime()));
		SecondsLeft = FMath::CeilToInt(PrepaireCountdown);
	}
	else if (MatchState == MatchState::RoundInProgress)
	{
		if (bOnce)
		{
			ResetTime();
		}
		//RoundCountdown = (RoundTime + PreveusTimeElapsed + LevelStartingTime + LoadingTime) - GetServerTime(); older way
		RoundCountdown = (RoundTime + PreveusTimeElapsed + OverTime) - GetServerTime();
		//UE_LOG(LogTemp, Error, TEXT("RoundCountdown: %f"), RoundCountdown);
		//UE_LOG(LogTemp, Warning, TEXT("LevelStartingTime: %f"), LevelStartingTime);
		//UE_LOG(LogTemp, Warning, TEXT("PreveusTimeElapsed: %f"), PreveusTimeElapsed);
		//UE_LOG(LogTemp, Warning, TEXT("LoadingTime: %f"), LoadingTime);
		
		//if (GEngine)
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("%f, %f, %f, %f, %f"), RoundCountdown, LevelStartingTime, PreveusTimeElapsed, LoadingTime, GetServerTime()));
		SecondsLeft = FMath::CeilToInt(RoundCountdown);
	}
	else if (MatchState == MatchState::RoundEnd)
	{
		if (bOnce)
		{
			ResetTime();
		}
		CoolCountdown = (CooldownTime + PreveusTimeElapsed) - GetServerTime();
		//CoolCountdown = (CooldownTime + PreveusTimeElapsed) - GetServerTime() + LevelStartingTime + LoadingTime;
		//UE_LOG(LogTemp, Error, TEXT("CoolCountdown: %f"), CoolCountdown);
		//UE_LOG(LogTemp, Warning, TEXT("TimeElapsed: %f"), GetServerTime());
		//UE_LOG(LogTemp, Warning, TEXT("PreveusTimeElapsed: %f"), PreveusTimeElapsed);
		SecondsLeft = FMath::CeilToInt(CoolCountdown);
	}
	else if (MatchState == MatchState::MatchEnd)
	{
		if (bOnce)
		{
			ResetTime();
		}
		MatchEndCountdown = (MatchEndTime + PreveusTimeElapsed) - GetServerTime();
		SecondsLeft = FMath::CeilToInt(MatchEndCountdown);
	}

	if (HasAuthority())
	{
		BracketsSinglesGameMode = BracketsSinglesGameMode == nullptr ? Cast<ABracketsSinglesGameMode>(UGameplayStatics::GetGameMode(this)) : BracketsSinglesGameMode;
		if (BracketsSinglesGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BracketsSinglesGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::RoundStart)
		{
			SetHUDSelectionCountdown(PrepaireCountdown);
		}
		if (MatchState == MatchState::RoundInProgress)
		{
			SetHUDMatchCountdown(RoundCountdown);
		}
		if (MatchState == MatchState::RoundEnd)
		{

		}
		if (MatchState == MatchState::MatchEnd)
		{

		}
	}
}

void ABracketsPlayerController::ResetTime()
{
	PreveusTimeElapsed = GetServerTime();
	bOnce = false;
}

void ABracketsPlayerController::ResetStats()
{
	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
	if (BracketsCharacter)
	{
		class UBracketsAnimInstance* AnimInstance = Cast<UBracketsAnimInstance>(BracketsCharacter->GetMesh1P()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->bWeaponEquiped = false;
		}
		ABracketsPlayerState* BracketsPlayerState = GetPlayerState<ABracketsPlayerState>();
		if (!BracketsPlayerState->GetIsDead())
		{
			BracketsCharacter->ResetHealth();
			float Health = BracketsCharacter->GetHealth();
			float MaxHealth = BracketsCharacter->GetMaxHealth();
			Health = MaxHealth;
			SetHUDHealth(Health, MaxHealth);
		}
		//float ResetAmmo = 0.f;
		//SetHUDAmmoCarried(ResetAmmo);
		//SetHUDWeaponAmmo(ResetAmmo);
		if (BracketsCharacter->HasAuthority())
		{
			BracketsCharacter->GetCombat()->InitializeCarriedAmmo();
		}
	}
}

void ABracketsPlayerController::SetHUDMatchCountdown(float Countdown)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->RoundCountdownText;
	if (bHUDValid)
	{
		if (Countdown < 0.f)
		{
			BracketsCharacterHUD->CharacterHUDWidget->RoundCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(Countdown / 60.f);
		int32 Seconds = Countdown - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BracketsCharacterHUD->CharacterHUDWidget->RoundCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABracketsPlayerController::SetHUDSelectionCountdown(float Countdown)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->EquipmentSelectionWidget &&
		BracketsCharacterHUD->EquipmentSelectionWidget->SelectionCountdown;
	if (bHUDValid)
	{
		if (Countdown < 0.f)
		{
			BracketsCharacterHUD->EquipmentSelectionWidget->SelectionCountdown->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(Countdown / 60.f);
		int32 Seconds = Countdown - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BracketsCharacterHUD->EquipmentSelectionWidget->SelectionCountdown->SetText(FText::FromString(CountdownText));
	}
}

void ABracketsPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->WeaponAmmo;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BracketsCharacterHUD->CharacterHUDWidget->WeaponAmmo->SetText(FText::FromString(AmmoText));
	}
}

void ABracketsPlayerController::SetHUDAmmoCarried(int32 Ammo)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->AmmoCarried;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BracketsCharacterHUD->CharacterHUDWidget->AmmoCarried->SetText(FText::FromString(AmmoText));
	}
}

void ABracketsPlayerController::SetLethalHUDIcon(TArray<TSubclassOf<AThrowableProjectile>> LethalArray)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1 &&
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2;
	if (bHUDValid)
	{
		if (LethalArray.IsValidIndex(0))
		{
			AThrowableProjectile* Lethal = LethalArray[0].GetDefaultObject();
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetBrushFromTexture(Lethal->GetSilhouette());
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetOpacity(1);
		}
		else
		{
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetBrushFromTexture(nullptr);
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetOpacity(0);
		}
		if (LethalArray.IsValidIndex(1))
		{
			AThrowableProjectile* Lethal = LethalArray[1].GetDefaultObject();
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetBrushFromTexture(Lethal->GetSilhouette());
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetOpacity(1);
		}
		else
		{
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetBrushFromTexture(nullptr);
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetOpacity(0);
		}
	}

	/*if (LethalIconArray.Num() < 1)
	{
		UTexture2D* TempTexture = LethalIconArray[0];
		LethalIconArray[0] = LethalIconArray[1];
		LethalIconArray[1] = TempTexture;
	}
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1 &&
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2;
	if (bHUDValid)
	{
		if (LethalIconArray.IsValidIndex(0))
		{
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetBrushFromTexture(LethalIconArray[0]);
		}
		if (LethalIconArray.IsValidIndex(1))
		{
			BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetBrushFromTexture(LethalIconArray[1]);
		}
	}*/
}

void ABracketsPlayerController::SetNonLethalHUDIcon(TArray<TSubclassOf<AThrowableProjectile>> NonLethalArray)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1 &&
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2;
	if (bHUDValid)
	{
		if (NonLethalArray.IsValidIndex(0))
		{
			AThrowableProjectile* NonLethal = NonLethalArray[0].GetDefaultObject();
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1->SetBrushFromTexture(NonLethal->GetSilhouette());
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1->SetOpacity(1);
		}
		else
		{
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1->SetBrushFromTexture(nullptr);
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1->SetOpacity(0);
		}
		if (NonLethalArray.IsValidIndex(1))
		{
			AThrowableProjectile* NonLethal = NonLethalArray[1].GetDefaultObject();
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2->SetBrushFromTexture(NonLethal->GetSilhouette());
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2->SetOpacity(1);
		}
		else
		{
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2->SetBrushFromTexture(nullptr);
			BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2->SetOpacity(0);
		}
	}
}

void ABracketsPlayerController::SetPrimaryHUDIcon(UTexture2D* Silhouette)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon;
	if (bHUDValid)
	{
		if (Silhouette)
		{
			BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon->SetBrushFromTexture(Silhouette);
			BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon->SetOpacity(1);
		}
		else
		{
			BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon->SetBrushFromTexture(nullptr);
			BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon->SetOpacity(0);
		}
	}
}

void ABracketsPlayerController::SetSecondaryHUDIcon(UTexture2D* Silhouette)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon;
	if (bHUDValid)
	{
		if (Silhouette)
		{
			BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon->SetBrushFromTexture(Silhouette);
			BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon->SetOpacity(1);
		}
		else
		{
			BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon->SetBrushFromTexture(nullptr);
			BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon->SetOpacity(0);
		}
	}
}

void ABracketsPlayerController::RemoveHUDWeaponIcons()
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon &&
		BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon &&
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1 &&
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2 &&
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1 &&
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2;
	if (bHUDValid)
	{
		BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon->SetBrushFromTexture(nullptr);
		BracketsCharacterHUD->CharacterHUDWidget->PrimaryIcon->SetOpacity(0);
		BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon->SetBrushFromTexture(nullptr);
		BracketsCharacterHUD->CharacterHUDWidget->SecondaryIcon->SetOpacity(0);
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetBrushFromTexture(nullptr);
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot1->SetOpacity(0);
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetBrushFromTexture(nullptr);
		BracketsCharacterHUD->CharacterHUDWidget->LethalSlot2->SetOpacity(0);
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1->SetBrushFromTexture(nullptr);
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot1->SetOpacity(0);
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2->SetBrushFromTexture(nullptr);
		BracketsCharacterHUD->CharacterHUDWidget->NonLethalSlot2->SetOpacity(0);
	}

}

void ABracketsPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->HealthBar &&
		BracketsCharacterHUD->CharacterHUDWidget->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BracketsCharacterHUD->CharacterHUDWidget->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		BracketsCharacterHUD->CharacterHUDWidget->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterHUDWidget = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
	if (Health <= 0.0f)
	{
		SetIsDead();
	}
}

void ABracketsPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->CharacterHUDWidget &&
		BracketsCharacterHUD->CharacterHUDWidget->ShieldBar;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BracketsCharacterHUD->CharacterHUDWidget->ShieldBar->SetPercent(ShieldPercent);
	}
}

void ABracketsPlayerController::SetHUDRoundEnd(bool bHasWon)
{	
	/*bIsDead = !bHasWon;
	bIsDeadCPP = !bHasWon;*/

	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
	if (BracketsCharacter)
	{
		BracketsCharacter->bDisableGameplay = true;
	}

	ClientSetHUDRoundEnd(bHasWon);
}

void ABracketsPlayerController::ClientSetHUDRoundEnd_Implementation(bool bHasWon)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;

	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->RoundEndWidget &&
		BracketsCharacterHUD->RoundEndWidget->EndRoundText;

	if (bHUDValid && !BracketsCharacterHUD->RoundEndWidget->IsVisible())
	{
		bool bLeaderBoardWidgetValid = BracketsCharacterHUD &&
			BracketsCharacterHUD->LeaderBoardWidget;

		if (bLeaderBoardWidgetValid && !BracketsCharacterHUD->LeaderBoardWidget->IsVisible())
		{
			BracketsCharacterHUD->RoundEndWidget->SetVisibility(ESlateVisibility::Visible);

			if (bHasWon)
			{
				BracketsCharacterHUD->RoundEndWidget->EndRoundText->SetText(FText::FromString(VictoryText));
				BracketsCharacterHUD->RoundEndWidget->PlayAnimation(BracketsCharacterHUD->RoundEndWidget->TextAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward);
			}
			if (!bHasWon)
			{
				BracketsCharacterHUD->RoundEndWidget->EndRoundText->SetText(FText::FromString(DefeatText));
				BracketsCharacterHUD->RoundEndWidget->PlayAnimation(BracketsCharacterHUD->RoundEndWidget->TextAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward);
			}
		}
	}
}

void ABracketsPlayerController::ShowHUDEscMenu(bool Pressed)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD &&
		BracketsCharacterHUD->InGameMenuWidget;
	if (bHUDValid)
	{
		ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(GetPawn());
		if (!BracketsCharacterHUD->InGameMenuWidget->IsVisible())
		{
			BracketsCharacterHUD->InGameMenuWidget->SetVisibility(ESlateVisibility::Visible);
			BracketsCharacterHUD->InGameMenuWidget->SetFocus();
			//BracketsCharacter->bDisableGameplay = true;
			//replace with show over all?
			if (BracketsCharacterHUD->EquipmentSelectionWidget->IsVisible())
			{
				BracketsCharacterHUD->EquipmentSelectionWidget->SetVisibility(ESlateVisibility::Hidden);
			}

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(BracketsCharacterHUD->InGameMenuWidget->TakeWidget());
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
		else
		{
			BracketsCharacterHUD->InGameMenuWidget->SetVisibility(ESlateVisibility::Hidden);
			//BracketsCharacter->bDisableGameplay = false;
			//replace with show over all?
			if (BracketsCharacterHUD->EquipmentSelectionWidget)
			{
				BracketsCharacterHUD->EquipmentSelectionWidget->SetVisibility(ESlateVisibility::Visible);
			}

			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
			SetShowMouseCursor(false);
		}
	}
}

void ABracketsPlayerController::ShowHUDLeaderBoard(bool Show, bool ShowMouse)
{
	BracketsCharacterHUD = BracketsCharacterHUD == nullptr ? Cast<ABracketsCharacterHUD>(GetHUD()) : BracketsCharacterHUD;
	bool bHUDValid = BracketsCharacterHUD && BracketsCharacterHUD->LeaderBoardWidget;
	if (bHUDValid)
	{
		if (ShowMouse)
		{
			if (!BracketsCharacterHUD->LeaderBoardWidget->IsVisible() && Show)
			{
				BracketsCharacterHUD->LeaderBoardWidget->SetVisibility(ESlateVisibility::Visible);
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(BracketsCharacterHUD->LeaderBoardWidget->TakeWidget());
				SetInputMode(InputMode);
				SetShowMouseCursor(true);
			}
			else
			{
				BracketsCharacterHUD->LeaderBoardWidget->SetVisibility(ESlateVisibility::Hidden);
				FInputModeGameOnly InputMode;
				SetInputMode(InputMode);
				SetShowMouseCursor(false);
			}
		}
		else if (!ShowMouse)
		{
			if (!BracketsCharacterHUD->LeaderBoardWidget->IsVisible() && Show)
			{
				BracketsCharacterHUD->LeaderBoardWidget->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				BracketsCharacterHUD->LeaderBoardWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

}

void ABracketsPlayerController::AddOvertime(float OvertimeAmount)
{
	OverTime = OvertimeAmount;
}

void ABracketsPlayerController::SetIsDead()
{
	ABracketsPlayerState* BracketsPlayerState = GetPlayerState<ABracketsPlayerState>();
	if (BracketsPlayerState)
	{
		BracketsPlayerState->SetIsDead(true);
	}

}

void ABracketsPlayerController::OnMatchSateSet(FName State)
{
	MatchState = State;
	bOnce = true;
	if (MatchState == MatchState::RoundStart)
	{
		HandleControllerRoundStart();
	}
	else if (MatchState == MatchState::RoundInProgress)
	{
		HandleControllerRoundInProgress();
	}
	else if (MatchState == MatchState::RoundEnd)
	{
		HandleControllerRoundEnd();
	}
	else if (MatchState == MatchState::MatchEnd)
	{
		HandleControllerMatchEnd();
	}
}

void ABracketsPlayerController::OnRep_MatchState()
{
	bOnce = true;
	if (MatchState == MatchState::RoundStart)
	{
		HandleControllerRoundStart();
	}
	else if (MatchState == MatchState::RoundInProgress)
	{
		HandleControllerRoundInProgress();
	}
	else if (MatchState == MatchState::RoundEnd)
	{
		HandleControllerRoundEnd();
	}
	else if (MatchState == MatchState::MatchEnd)
	{
		HandleControllerMatchEnd();
	}
}