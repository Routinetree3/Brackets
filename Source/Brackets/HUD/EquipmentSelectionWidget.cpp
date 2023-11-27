// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentSelectionWidget.h"
#include "Brackets/BracketsCharacter.h"
#include "Brackets/Components/CombatComponent.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Brackets/Weapons/Weapon.h"

bool UEquipmentSelectionWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UEquipmentSelectionWidget::PrimaryButtonClicked(TSubclassOf<AWeapon> SelectedWeapon)
{
	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwningPlayerPawn()) : Character;
	if (Character && Character->GetCombat())
	{
		//later on put a check for match state to insure that this can only be applied during "Round Starts"
		Character->GetCombat()->SpawnWeaponActor(SelectedWeapon, true);
	}
}

void UEquipmentSelectionWidget::SecondaryButtonClicked(TSubclassOf<AWeapon> SelectedWeapon)
{
	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwningPlayerPawn()) : Character;
	if (Character && Character->GetCombat())
	{
		//later on put a check for match state to insure that this can only be applied during "Round Starts"
		Character->GetCombat()->SpawnWeaponActor(SelectedWeapon, false);
	}
}

bool UEquipmentSelectionWidget::LethalButtonClicked(TSubclassOf<class AThrowableProjectile> SelectedThrowable)
{
	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwningPlayerPawn()) : Character;
	if (Character && Character->GetCombat())
	{
		if (Character->GetCurrentLethals() >= Character->GetMaxLethals() || 
			Character->GetCurrentLethals() + Character->GetCurrentNonLethals() >= Character->GetMaxThrowables())
		{
			return false;
		}
		else
		{
			Character->GetCombat()->EquipThrowable(SelectedThrowable, true);
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool UEquipmentSelectionWidget::NonLethalButtonClicked(TSubclassOf<class AThrowableProjectile> SelectedThrowable)
{
	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwningPlayerPawn()) : Character;
	if (Character && Character->GetCombat())
	{
		if (Character->GetCurrentNonLethals() >= Character->GetMaxNonLethals() ||
			Character->GetCurrentLethals() + Character->GetCurrentNonLethals() >= Character->GetMaxThrowables())
		{
			return false;
		}
		else
		{
			Character->GetCombat()->EquipThrowable(SelectedThrowable, false);
			return true;
		}
	}
	else
	{
		return false;
	}
}

void UEquipmentSelectionWidget::ShieldButtonClicked(bool isFull)
{
	//ShieldButton->SetIsEnabled(false);

	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwningPlayerPawn()) : Character;

	if (Character)
	{
		Character->ApplyShield(true);

		if (isFull)
		{
			Character->ApplyShield(true);
		}
		else
		{
			Character->ApplyShield(false);
		}

		Controller = Controller == nullptr ? Cast<ABracketsPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			//later on put a check for match state to insure that this can only be applied during "Round Starts"
		}
	}

}

void UEquipmentSelectionWidget::DeleteSheildClicked()
{
	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwningPlayerPawn()) : Character;
	if (Character)
	{
		Character->DeleteShield();
	}
}



