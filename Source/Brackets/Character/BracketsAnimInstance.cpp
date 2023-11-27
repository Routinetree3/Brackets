// Fill out your copyright notice in the Description page of Project Settings.


#include "BracketsAnimInstance.h"
#include "Brackets/BracketsCharacter.h"
#include "Brackets/Weapons/Weapon.h"
#include "Brackets/Types/CombatState.h"

void UBracketsAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BracketsCharacter = Cast<ABracketsCharacter>(TryGetPawnOwner());
}

void UBracketsAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BracketsCharacter == nullptr)
	{
		BracketsCharacter = Cast<ABracketsCharacter>(TryGetPawnOwner());
	}
	if (BracketsCharacter == nullptr) return;

	bWeaponEquiped = BracketsCharacter->IsWeaponEquipped();

	AO_Yaw = BracketsCharacter->GetAO_Yaw();
	AO_Pitch = BracketsCharacter->GetAO_Pitch();

	bUseFABRIK = BracketsCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bIsCrouched = BracketsCharacter->bIsCrouched;

}
