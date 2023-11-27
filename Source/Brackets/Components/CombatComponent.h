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
class AThrowableProjectile;

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

	UFUNCTION()
	void SpawnWeaponActor(TSubclassOf<class AWeapon> ActortoSpawnClass, bool bIsPrimary);

	UFUNCTION()
	void DropEquippedWeapon(bool bIsPrimary);
	void ClearThrowables();
	void SetAiming(bool bIsAiming);
	void InitializeCarriedAmmo();

	void ThrowLethal();
	void ThrowNonLethal();
	void EquipThrowable(TSubclassOf<class AThrowableProjectile> ThrowableToEquip, bool bIsLeathal);

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
	UFUNCTION(Server, Reliable)
		void ServerThrow(TSubclassOf<AThrowableProjectile> ThrowableToThrow, bool bIsLethal);

	void Throw(TSubclassOf<AThrowableProjectile> ThrowableToThrow);

	//equip
	UFUNCTION(BlueprintCallable)
		void EquipActiveWeapon(AWeapon* WeaponToEquip);
	UFUNCTION(BlueprintCallable)
		void EquipActiveWeapon3P(AWeapon* WeaponToEquip3P);

		void HolsterPrimaryWeapon();
		void HolsterPrimaryWeapon(AWeapon* WeaponToHolster, AWeapon* WeaponToHolster3P);
		void HolsterSecondaryWeapon();
		void HolsterSecondaryWeapon(AWeapon* WeaponToHolster, AWeapon* WeaponToHolster3P);

	UFUNCTION()
		void OnRep_HolsterPrimaryWeapon();
	UFUNCTION()
		void OnRep_HolsterSecondaryWeapon();

	UFUNCTION(BlueprintCallable)
		void SwapToPrimaryWeapon();
	UFUNCTION(Server, Reliable)
		void ServerSwapToPrimaryWeapon();

	UFUNCTION(BlueprintCallable)
		void SwapToSecondaryWeapon();
	UFUNCTION(Server, Reliable)
		void ServerSwapToSecondaryWeapon();

	UFUNCTION(BlueprintCallable)
		void SwapLethals();
	UFUNCTION(BlueprintCallable)
		void SwapNonLethals();

	UFUNCTION()
		void OnRep_EquipActiveWeapon();

	UFUNCTION()
	void CycleEquipment(int32 InputValue);

	UFUNCTION(Server, Reliable)
		void ServerSpawnWeaponActor(TSubclassOf<class AWeapon> ActortoSpawnClass, bool bIsPrimary);

	// True for FirstPerson || False for ThridPerson
	UFUNCTION()
		void AttachActorToSocket(AActor* ActorToAttach, bool bMeshPerspective, FName SocketName);

	UFUNCTION(Server, Reliable)
		void ServerAttachActorToSocket(AActor* ActorToAttach, bool bMeshPerspective, FName SocketName);

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
	int32 StartingCarbineAmmo = 30;
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 30;
	UPROPERTY(EditAnywhere)
	int32 StartingBattleRifleAmmo = 40;
	UPROPERTY(EditAnywhere)
	int32 StartingSimiPistolAmmo = 12;
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 16;
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 20;

	//Firing Functions Extra
	bool bFireButtonPressed;
	FHUDPackage HUDPackage;

	int32 CycleIndex = 0;
	UPROPERTY(ReplicatedUsing = OnRep_EquipActiveWeapon)
		AWeapon* SelectedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_EquipActiveWeapon)
		AWeapon* SelectedWeapon3P;
	UPROPERTY(ReplicatedUsing = OnRep_HolsterPrimaryWeapon)
		AWeapon* HolsteredPrimaryWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_HolsterPrimaryWeapon)
		AWeapon* HolsteredPrimaryWeapon3P;
	UPROPERTY(ReplicatedUsing = OnRep_HolsterSecondaryWeapon)
		AWeapon* HolsteredSecondaryWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_HolsterSecondaryWeapon)
		AWeapon* HolsteredSecondaryWeapon3P;

	int32 LethalSlotArrayIndex = 0;
	int32 NonLethalSlotArrayIndex = 0;

	int32 CurrentLethals = 0;
	int32 CurrentNonLethals = 0;
	UPROPERTY(EditAnywhere)
		int32 MaxLethals = 2;
	UPROPERTY(EditAnywhere)
		int32 MaxNonLethals = 2;
	UPROPERTY(EditAnywhere)
		int32 MaxThowables = 3;

	UPROPERTY()
	TArray<TSubclassOf<AThrowableProjectile>>LethalSlotArray;
	UPROPERTY()
	TArray<TSubclassOf<AThrowableProjectile>>NonLethalSlotArray;

	FName HandSocket = FName("RightHandSocket");
	FName HandSocket3P = FName("RightHandSocket3P");

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
