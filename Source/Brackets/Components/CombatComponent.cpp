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
#include "Brackets/Weapons/ThrowableProjectile.h"
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
	DOREPLIFETIME(UCombatComponent, SelectedWeapon);
	DOREPLIFETIME(UCombatComponent, SelectedWeapon3P);

	DOREPLIFETIME(UCombatComponent, HolsteredPrimaryWeapon);
	DOREPLIFETIME(UCombatComponent, HolsteredPrimaryWeapon3P);
	DOREPLIFETIME(UCombatComponent, HolsteredSecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, HolsteredSecondaryWeapon3P);

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
	if (Character == nullptr || SelectedWeapon == nullptr)
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
	if (Character == nullptr || SelectedWeapon == nullptr)
	{
		return;
	}
	int32 RealoadAmount = AmountToReload();
	if (AmmoCarriedMap.Contains(SelectedWeapon->GetWeaponType()))
	{
		AmmoCarriedMap[SelectedWeapon->GetWeaponType()] -= RealoadAmount;
		AmmoCarried = AmmoCarriedMap[SelectedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoCarried(AmmoCarried);
	}
	SelectedWeapon->AddAmmo(-RealoadAmount);
}

int32 UCombatComponent::AmountToReload()
{
	if (SelectedWeapon == nullptr)
	{
		return 0;
	}
	int32 RoomInMag = SelectedWeapon->GetMagCapacity() - SelectedWeapon->GetAmmo();

	if (AmmoCarriedMap.Contains(SelectedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = AmmoCarriedMap[SelectedWeapon->GetWeaponType()];
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
	case ECombatState::ECS_Throwing:
		if (Character && !Character->IsLocallyControlled())
		{
			//Character->PlayThrowGrenadeMontage();
		}
		break;
	}
}

void UCombatComponent::SpawnWeaponActor(TSubclassOf<class AWeapon> ActortoSpawnClass, bool bIsPrimary)
{
	if (Character->HasAuthority())
	{
		const USkeletalMeshSocket* SkeletalHandSocket1P = Character->GetMesh1P()->GetSocketByName(FName("RightHandSocket"));
		FTransform HandSocketTransform1P = SkeletalHandSocket1P->GetSocketTransform(Character->GetMesh1P());
		const USkeletalMeshSocket* SkeletalHandSocket3P = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		FTransform HandSocketTransform3P = SkeletalHandSocket3P->GetSocketTransform(Character->GetMesh());

		const USkeletalMeshSocket* SecondarySocket = Character->GetMesh1P()->GetSocketByName(FName("SecondaryWeaponSocket"));
		FTransform SecondarySocketTransform = SecondarySocket->GetSocketTransform(Character->GetMesh());

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();

		if (World && Character)
		{
			if (bIsPrimary)
			{
				AWeapon* WeaponToEquip = World->SpawnActor<AWeapon>(ActortoSpawnClass, HandSocketTransform1P, SpawnParams);
				WeaponToEquip->SetMaterialView(true);
				if (WeaponToEquip)
				{
					AWeapon* WeaponToEquip3P = World->SpawnActor<AWeapon>(ActortoSpawnClass, HandSocketTransform3P, SpawnParams);
					WeaponToEquip3P->SetMaterialView(false);
					if (WeaponToEquip3P)
					{
						WeaponToEquip->SetOwner(Character);
						WeaponToEquip3P->SetOwner(Character);
						WeaponToEquip->GetWeaponMesh()->SetOnlyOwnerSee(true);
						WeaponToEquip3P->GetWeaponMesh()->SetOwnerNoSee(true);
						if (SelectedWeapon && SelectedWeapon3P && SelectedWeapon->GetWeaponType() == EWeaponType::EWT_SimiPistol)
						{
							//Swap Weapon
							HolsterSecondaryWeapon();
							EquipActiveWeapon(WeaponToEquip);
							EquipActiveWeapon3P(WeaponToEquip3P);
						}
						else if (SelectedWeapon && SelectedWeapon3P && SelectedWeapon->GetWeaponType() != EWeaponType::EWT_SimiPistol)
						{
							DropEquippedWeapon(bIsPrimary);
							EquipActiveWeapon(WeaponToEquip);
							EquipActiveWeapon3P(WeaponToEquip3P);
						}
						else
						{
							EquipActiveWeapon(WeaponToEquip);
							EquipActiveWeapon3P(WeaponToEquip3P);
						}
					}
				}
			}
			else
			{
				AWeapon* SecondaryWeaponToEquip = World->SpawnActor<AWeapon>(ActortoSpawnClass, SecondarySocketTransform, SpawnParams);
				SecondaryWeaponToEquip->SetMaterialView(true);
				if (SecondaryWeaponToEquip)
				{
					AWeapon* SecondaryWeaponToEquip3P = World->SpawnActor<AWeapon>(ActortoSpawnClass, SecondarySocketTransform, SpawnParams);
					SecondaryWeaponToEquip3P->SetMaterialView(false);
					if (SecondaryWeaponToEquip3P)
					{
						SecondaryWeaponToEquip->SetOwner(Character);
						SecondaryWeaponToEquip3P->SetOwner(Character);
						SecondaryWeaponToEquip->GetWeaponMesh()->SetOnlyOwnerSee(true);
						SecondaryWeaponToEquip3P->GetWeaponMesh()->SetOwnerNoSee(true);
						if (SelectedWeapon == nullptr)
						{
							EquipActiveWeapon(SecondaryWeaponToEquip);
							EquipActiveWeapon3P(SecondaryWeaponToEquip3P);
						}
						else if (HolsteredSecondaryWeapon)
						{
							DropEquippedWeapon(bIsPrimary);//droping secondary
							HolsterSecondaryWeapon(SecondaryWeaponToEquip, SecondaryWeaponToEquip3P);
						}
						else
						{
							HolsterSecondaryWeapon(SecondaryWeaponToEquip, SecondaryWeaponToEquip3P);
						}
					}
				}
			}
		}
	}
	else
	{
		ServerSpawnWeaponActor(ActortoSpawnClass, bIsPrimary);
	}

	if (bIsPrimary)
	{
		AWeapon* Primary = ActortoSpawnClass.GetDefaultObject();
		Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetPrimaryHUDIcon(Primary->GetSilhouette());
		}
	}
	else
	{
		AWeapon* Secondary = ActortoSpawnClass.GetDefaultObject();
		Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetSecondaryHUDIcon(Secondary->GetSilhouette());
		}
	}
}

void UCombatComponent::ServerSpawnWeaponActor_Implementation(TSubclassOf<class AWeapon> ActortoSpawnClass, bool bIsPrimary)
{
	SpawnWeaponActor(ActortoSpawnClass, bIsPrimary);
}

//Equip Weapons

void UCombatComponent::EquipActiveWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	SelectedWeapon = WeaponToEquip;
	SelectedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToSocket(SelectedWeapon, true, HandSocket);
	SelectedWeapon->SetHUDAmmo();

	if (AmmoCarriedMap.Contains(SelectedWeapon->GetWeaponType()))
	{
		AmmoCarried = AmmoCarriedMap[SelectedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDAmmoCarried(AmmoCarried);
	}
}

void UCombatComponent::EquipActiveWeapon3P(AWeapon* WeaponToEquip3P)
{
	if (Character == nullptr || WeaponToEquip3P == nullptr)
	{
		return;
	}
	SelectedWeapon3P = WeaponToEquip3P;
	SelectedWeapon3P->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToSocket(SelectedWeapon3P, false, HandSocket3P);
}

void UCombatComponent::HolsterPrimaryWeapon()
{
	if (Character == nullptr || SelectedWeapon->GetWeaponType() == EWeaponType::EWT_SimiPistol /*|| SelectedWeapon->GetWeaponType() == EWeaponType::EWT_Revolver*/)
	{
		return;
	}
	HolsteredPrimaryWeapon = SelectedWeapon;
	HolsteredPrimaryWeapon3P = SelectedWeapon3P;
	HolsteredPrimaryWeapon->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredPrimaryWeapon3P->SetWeaponState(EWeaponState::EWS_Holstered);
	AttachActorToSocket(HolsteredPrimaryWeapon, true, HolsteredPrimaryWeapon->GetHolsterLocation());
	AttachActorToSocket(HolsteredPrimaryWeapon3P, false, HolsteredPrimaryWeapon->GetHolsterLocation());
}

void UCombatComponent::HolsterPrimaryWeapon(AWeapon* WeaponToHolster, AWeapon* WeaponToHolster3P)
{
	
	if (Character == nullptr || WeaponToHolster || WeaponToHolster3P || WeaponToHolster->GetWeaponType() == EWeaponType::EWT_SimiPistol /*|| WeaponToHolster->GetWeaponType() == EWeaponType::EWT_Revolver*/)
	{
		return;
	}
	HolsteredPrimaryWeapon = WeaponToHolster;
	HolsteredPrimaryWeapon3P = WeaponToHolster3P;
	HolsteredPrimaryWeapon->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredPrimaryWeapon3P->SetWeaponState(EWeaponState::EWS_Holstered);
	AttachActorToSocket(HolsteredPrimaryWeapon, true, HolsteredPrimaryWeapon->GetHolsterLocation());
	AttachActorToSocket(HolsteredPrimaryWeapon3P, false, HolsteredPrimaryWeapon->GetHolsterLocation());
	
}

void UCombatComponent::HolsterSecondaryWeapon()
{
	if (Character == nullptr || SelectedWeapon->GetWeaponType() != EWeaponType::EWT_SimiPistol /*|| SelectedWeapon->GetWeaponType() != EWeaponType::EWT_Revolver*/)
	{
		return;
	}
	HolsteredSecondaryWeapon = SelectedWeapon;
	HolsteredSecondaryWeapon3P = SelectedWeapon3P;
	HolsteredSecondaryWeapon->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredSecondaryWeapon3P->SetWeaponState(EWeaponState::EWS_Holstered);
	AttachActorToSocket(HolsteredSecondaryWeapon, true, HolsteredSecondaryWeapon->GetHolsterLocation());
	AttachActorToSocket(HolsteredSecondaryWeapon3P, false, HolsteredSecondaryWeapon->GetHolsterLocation());
}

void UCombatComponent::HolsterSecondaryWeapon(AWeapon* WeaponToHolster, AWeapon* WeaponToHolster3P)
{
	
	if (Character == nullptr || WeaponToHolster == nullptr || WeaponToHolster3P == nullptr || WeaponToHolster->GetWeaponType() != EWeaponType::EWT_SimiPistol /*|| WeaponToHolster->GetWeaponType() != EWeaponType::EWT_Revolver*/)
	{
		return;
	}
	HolsteredSecondaryWeapon = WeaponToHolster;
	HolsteredSecondaryWeapon3P = WeaponToHolster3P;
	HolsteredSecondaryWeapon->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredSecondaryWeapon3P->SetWeaponState(EWeaponState::EWS_Holstered);
	AttachActorToSocket(HolsteredSecondaryWeapon, true, HolsteredSecondaryWeapon->GetHolsterLocation());
	AttachActorToSocket(HolsteredSecondaryWeapon3P, false, HolsteredSecondaryWeapon->GetHolsterLocation());
	
}

void UCombatComponent::OnRep_EquipActiveWeapon()
{
	if (SelectedWeapon && SelectedWeapon3P && Character)
	{
		SelectedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		SelectedWeapon3P->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToSocket(SelectedWeapon, true, HandSocket);
		AttachActorToSocket(SelectedWeapon3P, false, HandSocket3P);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		SelectedWeapon->GetWeaponMesh()->SetOnlyOwnerSee(true);
		SelectedWeapon3P->GetWeaponMesh()->SetOwnerNoSee(true);
		SelectedWeapon->SetHUDAmmo();

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

void UCombatComponent::EquipThrowable(TSubclassOf<AThrowableProjectile> ThrowableToEquip, bool bIsLeathal)
{
	if (Character == nullptr || ThrowableToEquip == nullptr) { return; }
	if (CurrentNonLethals + CurrentLethals >= MaxThowables) { return; }
	Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;

	UE_LOG(LogTemp, Error, TEXT("EquipThrowable"));

	if (bIsLeathal && CurrentLethals <= MaxLethals)
	{
		switch (LethalSlotArrayIndex)
		{
		case 0:
			LethalSlotArray.Insert(ThrowableToEquip, LethalSlotArrayIndex);
			CurrentLethals++;
			if (Controller)
			{
				Controller->SetLethalHUDIcon(LethalSlotArray);
			}
			break;
		case 1:
			LethalSlotArray.Insert(ThrowableToEquip, LethalSlotArrayIndex);
			CurrentLethals++;
			if (Controller)
			{
				Controller->SetLethalHUDIcon(LethalSlotArray);
			}
			break;
		default:
			break;
		}
		if (LethalSlotArrayIndex >= MaxLethals - 1)
		{
			LethalSlotArrayIndex = 0;
		}
		else
		{
			LethalSlotArrayIndex++;
		}
	}
	else if (!bIsLeathal && CurrentNonLethals <= MaxNonLethals)
	{
		switch (NonLethalSlotArrayIndex)
		{
		case 0:
			NonLethalSlotArray.Insert(ThrowableToEquip, NonLethalSlotArrayIndex);
			CurrentNonLethals++;
			if (Controller)
			{
				Controller->SetNonLethalHUDIcon(NonLethalSlotArray);
			}
			break;
		case 1:
			NonLethalSlotArray.Insert(ThrowableToEquip, NonLethalSlotArrayIndex);
			CurrentNonLethals++;
			if (Controller)
			{
				Controller->SetNonLethalHUDIcon(NonLethalSlotArray);
			}
			break;
		default:
			break;
		}
		if (NonLethalSlotArrayIndex >= MaxNonLethals - 1)
		{
			NonLethalSlotArrayIndex = 0;
		}
		else
		{
			NonLethalSlotArrayIndex++;
		}
	}
}

void UCombatComponent::OnRep_HolsterPrimaryWeapon()
{
	HolsteredPrimaryWeapon->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredPrimaryWeapon3P->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredPrimaryWeapon->SetOwner(Character);
	HolsteredPrimaryWeapon3P->SetOwner(Character);
	HolsteredPrimaryWeapon->GetWeaponMesh()->SetOnlyOwnerSee(true);
	HolsteredPrimaryWeapon3P->GetWeaponMesh()->SetOwnerNoSee(true);

	AttachActorToSocket(HolsteredPrimaryWeapon, true, HolsteredPrimaryWeapon->GetHolsterLocation());
	AttachActorToSocket(HolsteredPrimaryWeapon3P, false, HolsteredPrimaryWeapon->GetHolsterLocation());
}

void UCombatComponent::OnRep_HolsterSecondaryWeapon()
{
	HolsteredSecondaryWeapon->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredSecondaryWeapon3P->SetWeaponState(EWeaponState::EWS_Holstered);
	HolsteredSecondaryWeapon->SetOwner(Character);
	HolsteredSecondaryWeapon3P->SetOwner(Character);
	HolsteredSecondaryWeapon->GetWeaponMesh()->SetOnlyOwnerSee(true);
	HolsteredSecondaryWeapon3P->GetWeaponMesh()->SetOwnerNoSee(true);

	AttachActorToSocket(HolsteredSecondaryWeapon, true, HolsteredSecondaryWeapon->GetHolsterLocation());
	AttachActorToSocket(HolsteredSecondaryWeapon3P, false, HolsteredSecondaryWeapon->GetHolsterLocation());
}

void UCombatComponent::SwapToPrimaryWeapon()
{
	if (SelectedWeapon == nullptr || HolsteredPrimaryWeapon == nullptr) { return; }
	if (Character && Character->HasAuthority() || SelectedWeapon->GetWeaponType() != EWeaponType::EWT_SimiPistol /*|| SelectedWeapon->GetWeaponType() != EWeaponType::EWT_Revolver*/)
	{
		//AWeapon* TempWeapon = SelectedWeapon;
		//AWeapon* TempWeapon3P = SelectedWeapon3P;
		HolsterSecondaryWeapon();
		EquipActiveWeapon(HolsteredPrimaryWeapon);
		EquipActiveWeapon3P(HolsteredPrimaryWeapon3P);
	}
	else
	{
		ServerSwapToPrimaryWeapon();
	}
}

void UCombatComponent::ServerSwapToPrimaryWeapon_Implementation()
{
	SwapToPrimaryWeapon();
}

void UCombatComponent::SwapToSecondaryWeapon()
{
	if (SelectedWeapon == nullptr || HolsteredSecondaryWeapon == nullptr) { return; }
	// if SelectedWeapon == EWeaponType::EWT_Knife // holster knife
	if (Character && Character->HasAuthority() || SelectedWeapon->GetWeaponType() == EWeaponType::EWT_SimiPistol /*|| SelectedWeapon->GetWeaponType() == EWeaponType::EWT_Revolver*/)
	{
		//AWeapon* TempWeapon = SelectedWeapon;
		//AWeapon* TempWeapon3P = SelectedWeapon3P;
		HolsterPrimaryWeapon();
		EquipActiveWeapon(HolsteredSecondaryWeapon);
		EquipActiveWeapon3P(HolsteredSecondaryWeapon3P);
	}
	else
	{
		ServerSwapToSecondaryWeapon();
	}
}

void UCombatComponent::ServerSwapToSecondaryWeapon_Implementation()
{
	SwapToSecondaryWeapon();
}

void UCombatComponent::SwapLethals()
{
	if (LethalSlotArray.Num() > 1)
	{
		if (LethalSlotArray.IsValidIndex(0) && LethalSlotArray.IsValidIndex(1))
		{
			TSubclassOf<AThrowableProjectile> TempGranade = LethalSlotArray[0];
			LethalSlotArray[0] = LethalSlotArray[1];
			LethalSlotArray[1] = TempGranade;
		}

		Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetLethalHUDIcon(LethalSlotArray);
		}
		UE_LOG(LogTemp, Warning, TEXT("SwapLethals"));
	}
}

void UCombatComponent::SwapNonLethals()
{
	if (NonLethalSlotArray.Num() > 1)
	{
		if (NonLethalSlotArray.IsValidIndex(0) && NonLethalSlotArray.IsValidIndex(1))
		{
			TSubclassOf<AThrowableProjectile> TempGranade = NonLethalSlotArray[0];
			NonLethalSlotArray[0] = NonLethalSlotArray[1];
			NonLethalSlotArray[1] = TempGranade;
		}

		Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetNonLethalHUDIcon(NonLethalSlotArray);
		}
		UE_LOG(LogTemp, Warning, TEXT("SwapNonLethals"));
	}
}

void UCombatComponent::CycleEquipment(int32 InputValue)
{
	CycleIndex += InputValue;
	CycleIndex = FMath::Clamp(CycleIndex, 0, 3);

	if (CycleIndex == 0 && InputValue == -1)
	{
		CycleIndex = 3;
	}
	switch (CycleIndex)
	{
	case 1:
		SwapToPrimaryWeapon();
		break;
	case 2:
		SwapToSecondaryWeapon();
		break;
	case 3:
		UE_LOG(LogTemp, Warning, TEXT("Switch To Knife"));
		break;
	default:
		break;
	}
	if (CycleIndex == 3 && InputValue == 1)
	{
		CycleIndex = 0;
	}

}

void UCombatComponent::DropEquippedWeapon(bool bIsPrimary) ////// REDU!!!!
{
	if (SelectedWeapon && SelectedWeapon3P && bIsPrimary)
	{
		SelectedWeapon->Dropped();
		SelectedWeapon3P->Dropped();
	}
	if (HolsteredSecondaryWeapon && HolsteredSecondaryWeapon3P && !bIsPrimary)
	{
		HolsteredSecondaryWeapon->Dropped();
		HolsteredSecondaryWeapon3P->Dropped();
	}
}

void UCombatComponent::ClearThrowables()
{
	LethalSlotArray.Empty();
	NonLethalSlotArray.Empty();

	LethalSlotArrayIndex = 0;
	NonLethalSlotArrayIndex = 0;
	CurrentLethals = 0;
	CurrentNonLethals = 0;

	if (Controller)
	{
		Controller->RemoveHUDWeaponIcons();
	}
}

void UCombatComponent::AttachActorToSocket(AActor* ActorToAttach, bool bMeshPerspective, FName SocketName)
{
	if (Character)
	{
		if (bMeshPerspective)
		{
			if (ActorToAttach)
			{ 
				const FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false);
				ActorToAttach->AttachToComponent(Character->GetMesh1P(), AttachmentTransformRules, SocketName);
			}
		}
		else if (!bMeshPerspective)
		{
			if (ActorToAttach)
			{
				const FAttachmentTransformRules AttachmentTransformRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false);
				ActorToAttach->AttachToComponent(Character->GetMesh(), AttachmentTransformRules, SocketName);
			}
		}
	}
	else
	{
		ServerAttachActorToSocket(ActorToAttach, bMeshPerspective, SocketName);
	}
}

void UCombatComponent::ServerAttachActorToSocket_Implementation(AActor* ActorToAttach, bool bMeshPerspective, FName SocketName)
{
	AttachActorToSocket(ActorToAttach, bMeshPerspective, SocketName);
}

//Fire Functions
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed && SelectedWeapon)
	{
		if (SelectedWeapon->bAutomatic)
		{
			Fire();
		}
		else
		{
			Fire();
			bFireButtonPressed = false;
		}
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
		if (SelectedWeapon)
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
	if (SelectedWeapon == nullptr || Character == nullptr)
	{
		return;
	}
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		SelectedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (SelectedWeapon == nullptr)
	{
		return;
	}
	bCanFire = true;
	if (bFireButtonPressed && SelectedWeapon->bAutomatic)
	{
		Fire();
	}
}

bool UCombatComponent::CanFire()
{
	if (SelectedWeapon == nullptr)
	{
		return false;
	}
	return !SelectedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (SelectedWeapon == nullptr)
	{
		return;
	}
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		//Character->PlayFireMontage(bAiming);
		SelectedWeapon->Fire(TraceHitTarget, bAiming);
	}
}

void UCombatComponent::ThrowLethal()
{
	//CombatState = ECombatState::ECS_Throwing;

	if (LethalSlotArray.IsValidIndex(0))
	{
		ServerThrow(LethalSlotArray[0], true);
		LethalSlotArray.RemoveAt(0);
		Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetLethalHUDIcon(LethalSlotArray);
		}
	}
}

void UCombatComponent::ThrowNonLethal()
{
	//CombatState = ECombatState::ECS_Throwing;

	if (NonLethalSlotArray.IsValidIndex(0))
	{
		ServerThrow(NonLethalSlotArray[0], true);
		if (NonLethalSlotArray.IsValidIndex(0))
		{
			NonLethalSlotArray.RemoveAt(0);
			Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->Controller) : Controller;
			if (Controller)
			{
				Controller->SetNonLethalHUDIcon(NonLethalSlotArray);
			}
		}
	}
}

void UCombatComponent::ServerThrow_Implementation(TSubclassOf<AThrowableProjectile> ThrowableToThrow, bool bIsLethal)
{
	//CombatState = ECombatState::ECS_Throwing;
	if (Character)
	{
		//Character->PlayThrowGrenadeMontage();
	}
	Throw(ThrowableToThrow);
	if (bIsLethal)
	{
		CurrentLethals--;
	}
	else if (!bIsLethal)
	{
		CurrentNonLethals--;
	}
}

void UCombatComponent::Throw(TSubclassOf<AThrowableProjectile> ThrowableToThrow)
{
	TSubclassOf<AThrowableProjectile> CurrentQucikThrowable = ThrowableToThrow;

	UE_LOG(LogTemp, Warning, TEXT("Throw"));

	if (Character && Character->HasAuthority())
	{
		const USkeletalMeshSocket* SkeletalLeftHandSocket = Character->GetMesh1P()->GetSocketByName(FName("ThrowSocket"));
		const FTransform StartingLocation = SkeletalLeftHandSocket->GetSocketTransform(Character->GetMesh1P());

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();

		if (CurrentQucikThrowable)
		{
			if (World)
			{
				World->SpawnActor<AThrowableProjectile>(
					CurrentQucikThrowable,
					StartingLocation,
					SpawnParams
					);
			}
		}
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
			if (SelectedWeapon)
			{
				HUDPackage.CrosshairsCenter = SelectedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = SelectedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = SelectedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = SelectedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = SelectedWeapon->CrosshairsBottom;
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
	if (SelectedWeapon == nullptr)
	{
		return;
	}
	if (bAiming) // Fix Zoom In problem
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, SelectedWeapon->GetZoomedFOV(), DeltaTime, SelectedWeapon->GetZoomInterpSpeed());
		SelectedWeapon->ShaderPOV(CurrentFOV);
		if (Character->DynamicArmMatierialInstance)
		{
			Character->DynamicArmMatierialInstance->SetScalarParameterValue(FName{ TEXT("FOV") }, CurrentFOV);
		}
		AimFOVOnce = true;
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
		SelectedWeapon->ShaderPOV(CurrentFOV);
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
	AmmoCarriedMap.Emplace(EWeaponType::EWT_Carbine, StartingCarbineAmmo);
	AmmoCarriedMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	AmmoCarriedMap.Emplace(EWeaponType::EWT_BattleRifle, StartingBattleRifleAmmo);
	AmmoCarriedMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	AmmoCarriedMap.Emplace(EWeaponType::EWT_SimiPistol, StartingSimiPistolAmmo);
	AmmoCarriedMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || SelectedWeapon == nullptr)
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

