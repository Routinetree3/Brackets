// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Brackets/HUD/BracketsCharacterHUD.h"
#include "Brackets/Types/CombatState.h"
#include "Brackets/Weapons/WeaponTypes.h"
#include "CombatComponent.generated.h"


class AWeapon;
class ABracketsCharacter;
class ABracketsPlayerController;
class ABracketsCharacterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BRACKETS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	friend class ABracketsCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FireButtonPressed(bool bPressed);

	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(AWeapon* WeaponToEquip);
	UFUNCTION(BlueprintCallable)
	void EquipWeapon3P(AWeapon* WeaponToEquip);

	UFUNCTION()
	void SpawnActor(TSubclassOf<class AWeapon> ActortoSpawnClass);

	UFUNCTION()
	void DropEquippedWeapon();
	void SetAiming(bool bIsAiming);
	void InitializeCarriedAmmo();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();

	int32 AmountToReload();

	//Crosshairs
	void TraceUnderCrosshairs(FHitResult& TranceHitResult);
	void SetHUDCrosshairs(float DeltaTime);

	//Firing Functions
	UFUNCTION()
		void Fire();
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION()
		void OnRep_EquippedWeapon();

	UFUNCTION(Server, Reliable)
	void ServerSpawnActor(TSubclassOf<class AWeapon> ActortoSpawnClass);

	// True for FirstPerson || False for ThridPerson
	UFUNCTION()
	void AttachActorToRightHand(AActor* ActorToAttach, bool bMeshPerspective);

	UFUNCTION(Server, Reliable)
	void ServerAttachActorToRightHand(AActor* ActorToAttach, bool bMeshPerspective);

private:
	UPROPERTY()
		ABracketsCharacter* Character;

	UPROPERTY()
		ABracketsPlayerController* Controller;

	UPROPERTY()
		ABracketsCharacterHUD* HUD;
	
	UPROPERTY(Replicated,EditAnywhere)
	bool bAiming = false;

	UPROPERTY(EditAnywhere)
		float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
		float AimWalkSpeed;

	float CrosshairVelocityFactor;
	float CrosshiarInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	float DefaultFOV = 90.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 45.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;
	float CurrentFOV;
	bool AimFOVOnce = false;

	void InterpFOV(float DeltaTime);
	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool bIsAiming);

	TMap<EWeaponType, int32> AmmoCarriedMap;
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	//Firing Functions Extra
	bool bFireButtonPressed;
	FHUDPackage HUDPackage;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon3P;

	FVector HitTarget;
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();
	int32 RoundsFired = 0;

	void UpdateAmmoValues();
	UPROPERTY(ReplicatedUsing = OnRep_AmmoCarried)
	int32 AmmoCarried;
	UFUNCTION()
	void OnRep_AmmoCarried();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
		void OnRep_CombatState();
};
