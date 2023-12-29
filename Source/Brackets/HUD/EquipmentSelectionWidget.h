// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSelectionWidget.generated.h"

class UTextBlock;
class UButton;
class AWeapon;

UCLASS()
class BRACKETS_API UEquipmentSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectionCountdown;

protected:
	virtual bool Initialize() override;

	UFUNCTION(BlueprintCallable)
	void PrimaryButtonClicked(TSubclassOf<AWeapon>SelectedWeapon);
	UFUNCTION(BlueprintCallable)
	void SecondaryButtonClicked(TSubclassOf<AWeapon>SelectedWeapon);

	UFUNCTION(BlueprintCallable)
		bool LethalButtonClicked(TSubclassOf<class AThrowableProjectile> SelectedThrowable);
	UFUNCTION(BlueprintCallable)
		bool NonLethalButtonClicked(TSubclassOf<class AThrowableProjectile> SelectedThrowable);
	UFUNCTION(BlueprintCallable)
	void DeleteThrowable(TSubclassOf<class AThrowableProjectile> SelectedThrowable);

	UFUNCTION(BlueprintCallable)
	void ShieldButtonClicked(bool isFull);

	UFUNCTION(BlueprintCallable)
	void DeleteSheildClicked();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> BattleRifleClass;

	UPROPERTY()
	class ABracketsCharacter* Character;
	UPROPERTY()
	class ABracketsPlayerController* Controller;

private:

};
