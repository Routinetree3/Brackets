// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Holstered UMETA(DisplayName = "Holstered"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Default Max")
};

class UCurvefloat;
class UCurveVector;
class UTexture2D;

UCLASS()
class BRACKETS_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	UFUNCTION(BlueprintImplementableEvent)
	void LoadCustomizationData();

	virtual void Fire(const FVector& HitTarget, bool IsAiming);
	//void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = .15f;
	UPROPERTY(EditAnywhere, Category = Combat)
		bool bAutomatic = true; // create a get

	// Ammo Public Functions
	virtual void OnRep_Owner() override;

	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);

	//Widget Function
	UFUNCTION()
	void ShowPickupWidget(bool bShowWidget);
	UFUNCTION()
	void Dropped();

	UFUNCTION()
	void AddRecoilImpulse();
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairsBottom;

	UFUNCTION() // dynamic material needed... Maybe wait till every gun/character has one material.
	void ShaderPOV(int32 FOV);
	UFUNCTION()
	void SetMaterialView(bool isFirstPerson);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherbodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);
	UFUNCTION()
		void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherbodyIndex
		);

	UPROPERTY()
		class ABracketsCharacter* BracketsOwnerCharacter;
	UPROPERTY()
		class ABracketsPlayerController* BracketsOwnerController;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;

	UPROPERTY()
	UTimelineComponent* RecoilTimeline;
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	UCurveFloat* TimelineCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	UCurveFloat* YawCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	UCurveFloat* PitchCurve;

	UFUNCTION()
	void TimelineCallback(float val);
	UFUNCTION()
	void TimelineFinishedCallback();

	float RecoilYawImpulse = 0.0f;
	float RecoilPitchImpulse = 0.0f;

	//Range 0 - 1. Strength of final impulse, 0 = Null, 1 = Default, 2 = Dubble.
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float YawAlpha = 1.f;
	//Range 0 - 1. Strength of final impulse, 0 = Null, 1 = Default, 2 = Dubble.
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float PitchAlpha = 1.f;
	//Time duration of the recoil impulse
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float RecoilLength = 0.10f;
	//Additinal spread on each recoil impulse
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float MaxAdditinalRadomizedImpulseYaw = 0.05f;
	//Additinal spread on each recoil impulse
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float MaxAdditinalRadomizedImpulsePitch = 0.05f;
	//Range 0 - 1. Strength of final impulse, 0 = Null, 1 = Default, 2 = Half.
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float AimEffect = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Recoil Properties")
	float MovementEffect = 0.8f;

	float AdditinalRandomizedImpulseYaw = 0.f;
	float AdditinalRandomizedImpulsePitch = 0.f;
	float AdjustedYawAlpha = 0.f;
	float AdjustedPitchAlpha = 0.f;

	bool bIsAiming = false;

	UPROPERTY(EditAnywhere)
		float ZoomedFOV = 45.f;
	UPROPERTY(EditAnywhere)
		float ZoomInterpSpeed = 20.f;

	UPROPERTY()
	FColor PrimaryColor;
	UPROPERTY()
	FColor SecondaryColor;
	UPROPERTY()
	FColor EmmisiveColor;
	UPROPERTY()
	USkeletalMesh* AppliedWeaponMesh;
	UPROPERTY()
	UTexture2D* AppliedPattern;
	UPROPERTY()
	UMaterialInstance* AppliedSkin;
	UPROPERTY(EditAnywhere)
	UTexture2D* Silhouette;

	//"SecondaryWeaponSocket", "FrontPrimaryHolster", "PrimaryHolster"
	UPROPERTY(EditAnywhere)
	FName SelectedHolster = FName("FrontPrimaryHolster");

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;
	UFUNCTION()
		void SpendRound();

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerRewind = true;
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* AreaSphere;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
		EWeaponState WeaponState;
	UFUNCTION()
		void OnRep_WeaponState();
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
		int32 Ammo;
	UPROPERTY(EditAnywhere)
		int32 MagCapacity;


	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		bool bUsePickUp = true;
	UPROPERTY()
		UMaterialInstanceDynamic* DynamicGunMatierialInstance;
	UPROPERTY(EditAnywhere, Category = "Weapon Skin")
		UMaterialInstance* FirstPersonGunMatierialInstance;

	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;

	UFUNCTION(NetMulticast, Reliable)
		void MulticastSetMaterialView(bool isFirstPerson);

public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo()const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetFireDelay() const { return FireDelay; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	FORCEINLINE float GetYawAlpha() const { return YawAlpha; }
	FORCEINLINE float GetPitchAlpha() const { return PitchAlpha; }
	FORCEINLINE float GetRecoilLength() const{ return RecoilLength; }
	FORCEINLINE float GetAimEffect() const { return AimEffect; }
	FORCEINLINE float GetMaxAdditinalRadomizedImpulseYaw() const { return MaxAdditinalRadomizedImpulseYaw; }
	FORCEINLINE float GetMaxAdditinalRadomizedImpulsePitch() const { return MaxAdditinalRadomizedImpulsePitch; }
	FORCEINLINE UCurveFloat* GetTimelineCurve() const { return TimelineCurve; }
	FORCEINLINE	UCurveFloat* GetYawCurve() const { return YawCurve; }
	FORCEINLINE	UCurveFloat* GetPitchCurve() const { return PitchCurve; }
	FORCEINLINE FName GetHolsterLocation() const { return SelectedHolster; }
	FORCEINLINE UTexture2D* GetSilhouette() const { return Silhouette; }
	FORCEINLINE float GetDamage() const { return Damage; }

	bool IsEmpty();
};
