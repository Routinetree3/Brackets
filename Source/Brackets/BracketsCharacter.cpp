// Copyright Epic Games, Inc. All Rights Reserved.

#include "BracketsCharacter.h"
#include "BracketsProjectile.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Brackets/Weapons/Weapon.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "Brackets/Components/CombatComponent.h"
#include "Brackets/Character/BracketsAnimInstance.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Brackets/Player/BracketsPlayerState.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Brackets/HUD/BracketsCharacterHUD.h"
#include "Brackets/GameModes/BracketsSinglesGameMode.h"


//////////////////////////////////////////////////////////////////////////
// ABracketsCharacter

ABracketsCharacter::ABracketsCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false; /* Needs Clean up */
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOnlyOwnerSee(false);
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(GetCapsuleComponent());
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;

	ShieldTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
}

void ABracketsCharacter::BeginPlay()
{

	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;

	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
	if (BracketsPlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BracketsPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(WeaponMappingContext, 1);
			Subsystem->AddMappingContext(InMatchMappingContext, 2);
		}
	}
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABracketsCharacter::ReciveDamage);
	}

	if (Mesh3P && Mesh3P->GetMaterial(GlassMaterialIndex))
	{
		DynamicAimEffectMaterialInstance = UMaterialInstanceDynamic::Create(Mesh3P->GetMaterial(GlassMaterialIndex), this);
		Mesh3P->SetMaterial(GlassMaterialIndex, DynamicAimEffectMaterialInstance);
	}

	if (Mesh1P && Mesh1P->GetMaterial(0))
	{
		DynamicArmMatierialInstance = UMaterialInstanceDynamic::Create(Mesh1P->GetMaterial(0), this);
		Mesh1P->SetMaterial(0, DynamicArmMatierialInstance);
	}

	ShieldTimeline->SetLooping(false);
	ShieldTimeline->SetTimelineLength(4.f);
	onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineCallback") });
	onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });
	ShieldTimeline->AddInterpFloat(TimelineCurve, onTimelineCallback);
	ShieldTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);

	BracketsPlayerState = BracketsPlayerState == nullptr ? Cast<ABracketsPlayerState>(Controller) : BracketsPlayerState;
}

void ABracketsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void ABracketsCharacter::Restart()
{
	Super::Restart();

	BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
	if (BracketsPlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BracketsPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(WeaponMappingContext, 1);
			Subsystem->AddMappingContext(InMatchMappingContext, 2);
		}
	}
}

void ABracketsCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABracketsCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABracketsCharacter, bDisableGameplay);
	DOREPLIFETIME(ABracketsCharacter, Health);
	DOREPLIFETIME(ABracketsCharacter, ShieldHealth);
}

//////////////////////////////////////////////////////////////////////////// Input

void ABracketsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABracketsCharacter::Move);
		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABracketsCharacter::Look);
		//Firing Weapon
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ABracketsCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Canceled, this, &ABracketsCharacter::FireButtonReleased);
		//Aim Weapon
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABracketsCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &ABracketsCharacter::AimButtonReleased);
		//Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ABracketsCharacter::ReloadButtonPressed);
		//InGameEscMenu
		EnhancedInputComponent->BindAction(EscMenuAction, ETriggerEvent::Triggered, this, &ABracketsCharacter::EscMenuButtonPressed);
		//ShowLeaderboard
		EnhancedInputComponent->BindAction(ShowLeaderboard, ETriggerEvent::Triggered, this, &ABracketsCharacter::ShowLeaderboardPressed);
		EnhancedInputComponent->BindAction(ShowLeaderboard, ETriggerEvent::Canceled, this, &ABracketsCharacter::ShowLeaderboardReleased);
	}
} 

void ABracketsCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABracketsCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (BracketsPlayerController != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ABracketsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (BracketsPlayerController != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABracketsCharacter::AimOffset(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		//bUseControllerRotationYaw = false;
	}
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		//bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//Map Pitch from [270, 360) to [-90, 0)//
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABracketsCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABracketsCharacter::FireButtonReleased(const FInputActionValue& Value)
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABracketsCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
	if (bAimEffect)
	{
		if (IsLocallyControlled() && HasAuthority())
		{
			MulticastAim(true);
		}
		else
		{
			ServerAimEffect(true);
		}
		bAimEffect = false;
	}
	//DynamicArmMatierialInstance->SetScalarParameterValue(FName{ TEXT("FOV") }, Combat->ZoomedFOV);
}

void ABracketsCharacter::AimButtonReleased(const FInputActionValue& Value)
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}	
	if (IsLocallyControlled() && HasAuthority())
	{
		MulticastAim(false);
	}
	else
	{
		ServerAimEffect(false);
	}
	bAimEffect = true;
	//DynamicArmMatierialInstance->SetScalarParameterValue(FName{ TEXT("FOV") }, Combat->DefaultFOV);
}

void ABracketsCharacter::ReloadButtonPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void ABracketsCharacter::EscMenuButtonPressed(const FInputActionValue& Value)
{
	BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
	if (BracketsPlayerController)
	{
		BracketsPlayerController->ShowHUDEscMenu(true);
	}
}

void ABracketsCharacter::ShowLeaderboardPressed(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
	BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
	if (BracketsPlayerController)
	{
		BracketsPlayerController->ShowHUDLeaderBoard(true, false);
	}
}

void ABracketsCharacter::ShowLeaderboardReleased(const FInputActionValue& Value)
{
	BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
	if (BracketsPlayerController)
	{
		BracketsPlayerController->ShowHUDLeaderBoard(false, false);
	}
}

void ABracketsCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool ABracketsCharacter::GetHasRifle()
{
	return bHasRifle;
}

void ABracketsCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABracketsCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

FVector ABracketsCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

bool ABracketsCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

void ABracketsCharacter::ApplyShield(bool isFull)
{
	if (isFull)
	{
		ShieldHealth = MaxShield;
	}
	else
	{
		ShieldHealth = HalfShield;
	}
	ServerApplyShield(isFull);
	UpdateHUDHealth();
}

void ABracketsCharacter::ServerApplyShield_Implementation(bool isFull)
{
	if (isFull)
	{
		ShieldHealth = MaxShield;
	}
	else
	{
		ShieldHealth = HalfShield;
	}
}

void ABracketsCharacter::DeleteShield()
{
	ShieldHealth = 0.f;
	ServerDeleteShield();
}

void ABracketsCharacter::ServerDeleteShield_Implementation()
{
	ShieldHealth = 0.f;
}

void ABracketsCharacter::TimelineCallback(float val)
{
	//use the value to larp along the curve to show the desired shield effect
	//Evntualy get all of the min and max values from the Materal itself
	float Opacity = 0.f;
	float Dissolve = 0.f;
	float Emmisive = 0.f;
	float HealthRatio = 0.f;

	if (ShieldMaterialInstance && DynamicShieldMaterialInstance)
	{
		Opacity = val;
		Opacity = FMath::Clamp(Opacity, 0.0f, 1.0f);
		DynamicShieldMaterialInstance->SetScalarParameterValue(TEXT("Opacity"), Opacity);

		Emmisive = FMath::Lerp(MinEmmisive, MaxEmmisive, val);
		DynamicShieldMaterialInstance->SetScalarParameterValue(TEXT("Emmisive"), Emmisive);

		HealthRatio = ShieldHealth / MaxShield;
		Dissolve = MaxDissolve * HealthRatio;
		Dissolve = FMath::Clamp(Dissolve - MaxDissolve, MinDissolve, MaxDissolve);
		DynamicShieldMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), Dissolve);
	}
}

void ABracketsCharacter::TimelineFinishedCallback()
{
	
	GetMesh1P()->SetOverlayMaterial(NULL);
	GetMesh3P()->SetOverlayMaterial(NULL);
}

void ABracketsCharacter::ApplyShieldEffect(bool ShieldDestroyed)
{
	if (ShieldDestroyed)
	{
		MaxEmmisive = 400;
		ShieldTimeline->SetPlayRate(2.f);
	}
	else
	{
		MaxEmmisive = 60;
	}
	MulticastShieldEffect(ShieldDestroyed);
}

void ABracketsCharacter::MulticastShieldEffect_Implementation(bool ShieldDestroyed)
{
	//play the shield playback
	if (ShieldMaterialInstance)
	{
		DynamicShieldMaterialInstance = UMaterialInstanceDynamic::Create(ShieldMaterialInstance, this);
		GetMesh1P()->SetOverlayMaterial(DynamicShieldMaterialInstance);
		GetMesh3P()->SetOverlayMaterial(DynamicShieldMaterialInstance);	
		if (ShieldTimeline && TimelineCurve)
		{
			if (ShieldDestroyed)
			{
				MaxEmmisive = 400;
				ShieldTimeline->SetPlayRate(10.f);
			}
			else
			{
				MaxEmmisive = 60;
			}
			ShieldTimeline->PlayFromStart();
		}
	}
}

void ABracketsCharacter::ResetHealth()
{
	Health = MaxHealth;
	UpdateHUDHealth();
}

void ABracketsCharacter::UpdateHUDHealth()
{
	BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
	if (BracketsPlayerController)
	{
		BracketsPlayerController->SetHUDHealth(Health, MaxHealth);
		BracketsPlayerController->SetHUDShield(ShieldHealth, MaxShield);
	}
}

void ABracketsCharacter::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorContorller, AActor* DamageCauser)
{
	BracketsPlayerState = BracketsPlayerState == nullptr ? Cast<ABracketsPlayerState>(Controller) : BracketsPlayerState;
	if (BracketsPlayerState && bHasNotTakenDamage)
	{
		BracketsPlayerState->SetHasTakenDamage(true);
		bHasNotTakenDamage = false;
	}

	if (ShieldHealth > 0.f)
	{
		if (Damage > ShieldHealth)
		{
			float OverflowDamage = Damage - ShieldHealth;
			ShieldHealth = FMath::Clamp(ShieldHealth - Damage, 0.f, MaxShield);
			Health = FMath::Clamp(Health - OverflowDamage, 0.f, MaxHealth);
			ApplyShieldEffect(true);

			//play destroy shield, Have the shield fade in quickly then have the opacity fade out
		}
		else
		{
			ShieldHealth = FMath::Clamp(ShieldHealth - Damage, 0.f, MaxShield);
			ApplyShieldEffect(false);

			//Mesh3P->SetOverlayMaterial(); Use this on both meshes with a timeline that will have the shield quickly fade in then fade out over a longer time
		}
		UpdateHUDHealth();
	}
	else
	{
		Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
		UpdateHUDHealth();
		//PlayerHitReactMontage();
	}

	if (Health <= 0.f)
	{
		Eliminated();

		BracketsPlayerController = BracketsPlayerController == nullptr ? Cast<ABracketsPlayerController>(Controller) : BracketsPlayerController;
		ABracketsPlayerController* AttackerController = Cast<ABracketsPlayerController>(InstigatorContorller);

		ABracketsSinglesGameMode* BracketsSinglesGameMode = GetWorld()->GetAuthGameMode<ABracketsSinglesGameMode>();
		if (BracketsSinglesGameMode)
		{
			BracketsSinglesGameMode->PlayerEliminated(this, BracketsPlayerController, AttackerController);
		}
	}
}

void ABracketsCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	//PlayerHitReactMontage();
}

void ABracketsCharacter::Eliminated()
{
	bDisableGameplay = true;
	Combat->DropEquippedWeapon();
	class UBracketsAnimInstance* AnimInstance = Cast<UBracketsAnimInstance>(GetMesh1P()->GetAnimInstance());
	if (AnimInstance)
	{
		AnimInstance->bWeaponEquiped = false;
	}
	MulticastEliminated();
}

void ABracketsCharacter::MulticastEliminated_Implementation()
{
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh3P()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh1P()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh1P()->SetVisibility(false);
	GetMesh3P()->SetVisibility(false);
}

void ABracketsCharacter::ServerAimEffect_Implementation(bool IsEnabled)
{
	MulticastAim(IsEnabled);
}

void ABracketsCharacter::MulticastAim_Implementation(bool IsEnabled)
{
	if (IsEnabled && DynamicAimEffectMaterialInstance)
	{
		DynamicAimEffectMaterialInstance->SetScalarParameterValue(TEXT("Alpha"), 1);
	}
	else if (!IsEnabled && DynamicAimEffectMaterialInstance)
	{
		DynamicAimEffectMaterialInstance->SetScalarParameterValue(TEXT("Alpha"), 0);
	}
}

ECombatState ABracketsCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

int32 ABracketsCharacter::GetRoundsFired() const
{
	if (Combat == nullptr) return 0;
	return Combat->RoundsFired;
}