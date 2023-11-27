// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"
#include "Brackets/Types/CombatState.h"
#include "Brackets/Types/HealthState.h"
#include "BracketsCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

class ABracketsPlayerController;
class ABracketsPlayerState;
class ABracketsGameMode;

class UCurvefloat;
class UCurveVector;

UCLASS(config=Game)
class ABracketsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* WeaponMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* InMatchMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;
	
public:
	ABracketsCharacter();
	virtual void PostInitializeComponents() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Restart() override;
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* CrouchAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	/** EquipAction*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* EquipAction;

	/** PrimarySwitch*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* PrimarySwitch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* SecondarySwitch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* CycleSwitch;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* SwitchLethal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* SwitchNonLethal;

	/** Menu Action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* EscMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ShowLeaderboard;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* FireAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* AimAction;
	/** Throw Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* ThrowLethalAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* ThrowNonLethalAction;

	void FireButtonPressed(const FInputActionValue& Value);
	void FireButtonReleased(const FInputActionValue& Value);
	void AimButtonPressed(const FInputActionValue& Value);
	void AimButtonReleased(const FInputActionValue& Value);
	void ReloadButtonPressed(const FInputActionValue& Value);
	void EscMenuButtonPressed(const FInputActionValue& Value);
	void ShowLeaderboardPressed(const FInputActionValue& Value);
	void ShowLeaderboardReleased(const FInputActionValue& Value);
	void CrouchPressed(const FInputActionValue& Value);
	void CrouchReleased(const FInputActionValue& Value);
	void ThrowLethalPressed(const FInputActionValue& Value);
	void ThrowNonLethalPressed(const FInputActionValue& Value);

	void PrimarySwitchButtonPressed(const FInputActionValue& Value);
	void SecondarySwitchButtonPressed(const FInputActionValue& Value);
	void SwitchLethalButtonPressed(const FInputActionValue& Value);
	void SwitchNonLethalButtonPressed(const FInputActionValue& Value);
	void CycleSwitchButtonPressed(const FInputActionValue& Value);

	void ResetHealth();
	void ApplyShield(bool isFull);   //// Redo the entire Shield System... not good that client has tells the server to apply shield
	void DeleteShield();
	void ApplyShieldEffect(bool ShieldDestroyed);

	void Eliminated();

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
		bool bHasRifle;

	UPROPERTY(VisibleAnywhere, Category = MatierialInstance)
		UMaterialInstanceDynamic* DynamicArmMatierialInstance;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
		void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
		bool GetHasRifle();

	void SetOverlappingWeapon(AWeapon* Weapon);

protected:	
	
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorContorller, AActor* DamageCauser);

	void AimOffset(float DeltaTime);
	UPROPERTY(EditDefaultsOnly)
	int32 GlassMaterialIndex = 1;

	UFUNCTION(Server, Reliable)
		void ServerApplyShield(bool isFull);
	UFUNCTION(Server, Reliable)
		void ServerDeleteShield();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastShieldEffect(bool ShieldDestroyed);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastEliminated();

	UFUNCTION(Server, Reliable)
		void ServerAimEffect(bool IsEnabled);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastAim(bool IsEnabled);

private:

	UPROPERTY()
		ABracketsPlayerController* BracketsPlayerController;
	UPROPERTY()
	ABracketsPlayerState* BracketsPlayerState;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	FVector ThirdMeshLocation;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Shield Stats")
		float MaxShield = 25.f;
	UPROPERTY(EditAnywhere, Category = "Shield Stats")
		float HalfShield = 12.5f;
	UPROPERTY(EditAnywhere, Category = "Shield Stats")
	float MinEmmisive = 10.f;
	UPROPERTY(EditAnywhere, Category = "Shield Stats")
	float MaxEmmisive = 60.f;
	UPROPERTY(EditAnywhere, Category = "Shield Stats")
	float MaxDissolve = 20.f;
	UPROPERTY(EditAnywhere, Category = "Shield Stats")
	float MinDissolve = 4.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats", meta = (AllowPrivateAccess = "true"))
		float Health = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, BlueprintReadOnly, Category = "Shield Stats", meta = (AllowPrivateAccess = "true"))
		float ShieldHealth = 0.f;

	UPROPERTY()
		UTimelineComponent* ShieldTimeline;
	UPROPERTY(EditDefaultsOnly, Category = "Shield Stats")
		UCurveFloat* TimelineCurve;

	// Dynamic instance that we can change at run time.
	UPROPERTY(VisibleAnywhere, Category = ShieldMaterial)
		UMaterialInstanceDynamic* DynamicShieldMaterialInstance;
	//Material instance set on the blueprint, used with the dynamic material instance.
	UPROPERTY(EditAnywhere, Category = ShieldMaterial)
		UMaterialInstance* ShieldMaterialInstance;

	UFUNCTION()
	void TimelineCallback(float val);
	UFUNCTION()
	void TimelineFinishedCallback();

	UFUNCTION()
	void OnRep_Health();
	void UpdateHUDHealth();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;

	FRotator StartingAimRotation;
	UPROPERTY(VisibleAnywhere, Category = MatierialInstance)
	UMaterialInstanceDynamic* DynamicAimEffectMaterialInstance;


	bool bFireButtonPressed = false;
	bool bHasNotTakenDamage = true;
	bool bAimEffect = true;


public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	FVector GetHitTarget() const;
	bool IsWeaponEquipped();
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShieldHealth() const { return ShieldHealth; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }

	int32 GetCurrentLethals() const;
	int32 GetCurrentNonLethals() const;
	int32 GetMaxLethals() const;
	int32 GetMaxNonLethals() const;
	int32 GetMaxThrowables() const;
	int32 GetRoundsFired() const;
	ECombatState GetCombatState() const;
};

