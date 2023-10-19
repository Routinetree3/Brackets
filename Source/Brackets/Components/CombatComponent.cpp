// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Brackets/Weapons/Weapon.h"
#include "Brackets/BracketsCharacter.h"
#include "Brackets/Player/BracketsPlayerController.h"

#include "Engine/EngineTypes.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon3P);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, AmmoCarried, COND_OwnerOnly);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( Character && Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
	if (Character)
	{
		DefaultFOV = Character->GetFirstPersonCameraComponent()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);

	}
}

//Reload Functions
void UCombatComponent::Reload()
{
	if (AmmoCarried > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	CombatState = ECombatState::ECS_Reloading;
	//HandleReload();
	FinishReloading();
	//UpdateAmmoValues();
}

void UCombatComponent::HandleReload()
{
	//FinishReloading();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr)
	{
		return;
	}
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	int32 RealoadAmount = AmountToReload();
	if (AmmoCarriedMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		AmmoCarriedMap[EquippedWeapon->GetWeaponType()] -= RealoadAmount;
		AmmoCarried = AmmoCarriedMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoCarried(AmmoCarried);
	}
	EquippedWeapon->AddAmmo(-RealoadAmount);
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)
	{
		return 0;
	}
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (AmmoCarriedMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = AmmoCarriedMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::OnRep_AmmoCarried()
{
	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoCarried(AmmoCarried);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::SpawnActor(TSubclassOf<class AWeapon> ActortoSpawnClass)
{
	if (Character->HasAuthority())
	{
		const USkeletalMeshSocket* HandSocket1P = Character->GetMesh1P()->GetSocketByName(FName("RightHandSocket"));
		FTransform HandSocketTransform1P = HandSocket1P->GetSocketTransform(Character->GetMesh1P());
		const USkeletalMeshSocket* HandSocket3P = Character->GetMesh3P()->GetSocketByName(FName("RightHandSocket"));
		FTransform HandSocketTransform3P = HandSocket3P->GetSocketTransform(Character->GetMesh3P());
		DropEquippedWeapon();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World && Character)
		{
			AWeapon* WeaponToEquip = World->SpawnActor<AWeapon>(ActortoSpawnClass, HandSocketTransform1P, SpawnParams);
			WeaponToEquip->SetMaterialView(true);
			if (WeaponToEquip)
			{
				EquipWeapon(WeaponToEquip);
				AWeapon* WeaponToEquip3P = World->SpawnActor<AWeapon>(ActortoSpawnClass, HandSocketTransform3P, SpawnParams);
				WeaponToEquip3P->SetMaterialView(false);
				if (WeaponToEquip3P)
				{
					EquipWeapon3P(WeaponToEquip3P);
				}
			}
		}
	}
	else
	{
		ServerSpawnActor(ActortoSpawnClass);
	}
}

void UCombatComponent::ServerSpawnActor_Implementation(TSubclassOf<class AWeapon> ActortoSpawnClass)
{
	SpawnActor(ActortoSpawnClass);
}

//Equip Weapons
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	//DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon, true);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->GetWeaponMesh()->SetOnlyOwnerSee(true);
	EquippedWeapon->SetHUDAmmo();

	if (AmmoCarriedMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		AmmoCarried = AmmoCarriedMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoCarried(AmmoCarried);
	}
}

void UCombatComponent::EquipWeapon3P(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	//DropEquippedWeapon();
	EquippedWeapon3P = WeaponToEquip;
	EquippedWeapon3P->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon3P, false);
	EquippedWeapon3P->SetOwner(Character);
	EquippedWeapon3P->GetWeaponMesh()->SetOwnerNoSee(true);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && EquippedWeapon3P && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		EquippedWeapon3P->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon, true);
		AttachActorToRightHand(EquippedWeapon3P, false);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		EquippedWeapon->GetWeaponMesh()->SetOnlyOwnerSee(true);
		EquippedWeapon3P->GetWeaponMesh()->SetOwnerNoSee(true);
		/*if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquipSound,
				Character->GetActorLocation()
			);
		}*/
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon && EquippedWeapon3P)
	{
		EquippedWeapon->Dropped();
		EquippedWeapon3P->Dropped();
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach, bool bMeshPerspective)
{
	if (Character)
	{
		if (bMeshPerspective)
		{
			if (ActorToAttach)
			{ 

				FName SocketName = (FName("RightHandSocket"));
			
				const FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false);
				ActorToAttach->AttachToComponent(Character->GetMesh1P(), AttachmentTransformRules, SocketName);
			}
		}
		else if (!bMeshPerspective)
		{
			if (ActorToAttach)
			{

				FName SocketName = (FName("RightHandSocket3P"));

				const FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false);
				ActorToAttach->AttachToComponent(Character->GetMesh3P(), AttachmentTransformRules, SocketName);
			}
		}

	}
	else
	{
		ServerAttachActorToRightHand(ActorToAttach, bMeshPerspective);
	}
}

void UCombatComponent::ServerAttachActorToRightHand_Implementation(AActor* ActorToAttach, bool bMeshPerspective)
{
	AttachActorToRightHand(ActorToAttach, bMeshPerspective);
}

//Fire Functions
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
	if (!bFireButtonPressed)
	{
		RoundsFired = 0;
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		RoundsFired++;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			bCanFire = false;
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
		if (Controller)
		{
			Controller->StartCameraShake();
		}
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		//Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget, bAiming);
	}
}

//Crosshairs
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TranceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld
	(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistacneToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistacneToCharacter + 100.f);

		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel
		(
			TranceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		/*if (TranceHitResult.GetActor()  && TranceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}*/
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)
	{
		return;
	}

	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABracketsCharacterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//Calculate crosshair spread
			//[0,600] -> [0,1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplyerRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplyerRange, Velocity.Size());

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	if (bAiming) // Fix Zoom In problem
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
		EquippedWeapon->ShaderPOV(CurrentFOV);
		if (Character->DynamicArmMatierialInstance)
		{
			Character->DynamicArmMatierialInstance->SetScalarParameterValue(FName{ TEXT("FOV") }, CurrentFOV);
		}
		AimFOVOnce = true;
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
		EquippedWeapon->ShaderPOV(CurrentFOV);
		if (Character->DynamicArmMatierialInstance)
		{
			Character->DynamicArmMatierialInstance->SetScalarParameterValue(FName{ TEXT("FOV") }, CurrentFOV);
		}
	}
	if (Character && Character->GetFirstPersonCameraComponent())
	{
		Character->GetFirstPersonCameraComponent()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	AmmoCarriedMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	bAiming = bIsAiming;
	ServerSetIsAiming(bIsAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetIsAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
