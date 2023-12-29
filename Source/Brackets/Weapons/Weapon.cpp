// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Brackets/Brackets.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/WidgetComponent.h"
#include "Brackets/Components/LagCompComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimationAsset.h"
#include "Brackets/BracketsCharacter.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"

#include "DrawDebugHelpers.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	/// <summary>
	/// Set Weapon Mesh from the selected weapon mesh on the Player Instance
	/// Check Player instance to see if Skin Material is applied || else... if not then apply default materal
	/// Apply the player skin Material                           || Apply the selected colors and pattern from the Player Instance to the default materal
	/// </summary>

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RecoilTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;

	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	RecoilTimeline->SetLooping(false);
	onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineCallback") });
	onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });
	RecoilTimeline->AddInterpFloat(TimelineCurve, onTimelineCallback);
	RecoilTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Fire(const FVector& HitTarget, bool IsAiming)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	bIsAiming = IsAiming;
	if (RecoilTimeline && TimelineCurve)
	{
		AddRecoilImpulse();
	}

	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
	if (MuzzleSocket && BracketsOwnerCharacter)
	{
		FVector Start = BracketsOwnerCharacter->GetFirstPersonCameraComponent()->GetComponentLocation();
		FVector End = Start + (HitTarget - Start) * 1.05f;

		FHitResult FireHit;
		UWorld* World = GetWorld();

		struct FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(BracketsOwnerCharacter);
		QueryParams.AddIgnoredComponent(BracketsOwnerCharacter->GetHead());
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_HitBox,
				QueryParams
			);

			DrawDebugLine(World, Start, End, FColor::Red, false, 5.f);

			ABracketsCharacter* HitBracketsCharacter = Cast<ABracketsCharacter>(FireHit.GetActor());
			if (FireHit.bBlockingHit)
			{
				if (HitBracketsCharacter && InstigatorController)
				{
					BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(OwnerPawn) : BracketsOwnerCharacter;
					BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(InstigatorController) : BracketsOwnerController;

					bool bCauseAuthDamage = !bUseServerRewind || OwnerPawn->IsLocallyControlled();

					//UE_LOG(LogTemp, Warning, TEXT("bCauseAuthDamage: %s"), (bCauseAuthDamage ? TEXT("true") : TEXT("false")));
					//UE_LOG(LogTemp, Warning, TEXT("HasAuthority(): %s"), (HasAuthority() ? TEXT("true") : TEXT("false")));

					if (HasAuthority() && bCauseAuthDamage)
					{
						const FName BoxThatWasHit = FireHit.GetComponent()->GetFName();
						UE_LOG(LogTemp, Error, TEXT("BoxThatWasHit: %s"), *BoxThatWasHit.ToString())
						auto BoxHitInfo = BracketsOwnerCharacter->HitBoxes.Find(BoxThatWasHit);
						float HitDamageRatio = 1;
						if (BoxHitInfo)
						{
							HitDamageRatio = BoxHitInfo->DamageRatio; 
						}
						UGameplayStatics::ApplyDamage(    
							HitBracketsCharacter,
							Damage * HitDamageRatio,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
						UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: %f"), Damage * HitDamageRatio)
					}
					if (!HasAuthority() && bUseServerRewind) // for some reason !!! loosing authority when switching Weapons, firing secondary, then switching back
					{
						if (BracketsOwnerCharacter && BracketsOwnerController && BracketsOwnerCharacter->GetLagComp() && BracketsOwnerCharacter->IsLocallyControlled())
						{
							BracketsOwnerCharacter->GetLagComp()->ServerScoreRequest(
								HitBracketsCharacter,
								Start,
								HitTarget,
								BracketsOwnerController->GetServerTime() - BracketsOwnerController->SingleTripTime,
								this
							);
						}
					}
					
				}
			}

			End = Start + (HitTarget - Start) * 1.01f;
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility,
				QueryParams
			);
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint
				);
			}
		}
		SpendRound();
	}
}

void AWeapon::AddRecoilImpulse()
{
	BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
	if (BracketsOwnerCharacter)
	{
		
		FVector Speed = BracketsOwnerCharacter->GetVelocity();

		if (YawCurve, PitchCurve)
		{
			RecoilYawImpulse = YawCurve->GetFloatValue(BracketsOwnerCharacter->GetRoundsFired());
			RecoilPitchImpulse = PitchCurve->GetFloatValue(BracketsOwnerCharacter->GetRoundsFired());

			AdditinalRandomizedImpulseYaw = FMath::FRandRange(MaxAdditinalRadomizedImpulseYaw * -1, MaxAdditinalRadomizedImpulseYaw);
			AdditinalRandomizedImpulsePitch = FMath::FRandRange(0.00f, MaxAdditinalRadomizedImpulsePitch);

			if (bIsAiming && Speed.X == 0.f && Speed.Y == 0.f && Speed.Z == 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Is Aiming but not moving"))
				AdjustedYawAlpha = YawAlpha / AimEffect;
				AdjustedPitchAlpha = PitchAlpha / AimEffect;
				AdditinalRandomizedImpulseYaw = 0.f;
				AdditinalRandomizedImpulsePitch = 0.f;
			}
			else if (bIsAiming && Speed.X > 0.f || Speed.Y > 0.f || Speed.Z > 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Is Aiming and is moving"))
				AdjustedYawAlpha = (YawAlpha / MovementEffect) / AimEffect;
				AdjustedPitchAlpha = (PitchAlpha / MovementEffect) / AimEffect;
				AdditinalRandomizedImpulseYaw = FMath::FRandRange((MaxAdditinalRadomizedImpulseYaw / MovementEffect) / AimEffect * -1, (MaxAdditinalRadomizedImpulseYaw / MovementEffect) / AimEffect);
				AdditinalRandomizedImpulsePitch = FMath::FRandRange(0.00f, (MaxAdditinalRadomizedImpulsePitch / MovementEffect) / AimEffect);
			}
			else if (Speed.X > 0.f || Speed.Y > 0.f || Speed.Z > 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Is not Aiming but is moving"))
				AdjustedYawAlpha = YawAlpha / MovementEffect;
				AdjustedPitchAlpha = PitchAlpha / MovementEffect;
				AdditinalRandomizedImpulseYaw = FMath::FRandRange((MaxAdditinalRadomizedImpulseYaw / MovementEffect) * -1, MaxAdditinalRadomizedImpulseYaw / MovementEffect);
				AdditinalRandomizedImpulsePitch = FMath::FRandRange(0.00f, MaxAdditinalRadomizedImpulsePitch / MovementEffect);
			}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("Is not Aiming and is not moving"))
				AdjustedYawAlpha = YawAlpha;
				AdjustedPitchAlpha = PitchAlpha;
			}
			//UE_LOG(LogTemp, Error, TEXT("RecoilPitchImpulse: %f, AdjustedPitchAlpha: %f, AdditinalRandomizedImpulsePitch: %f "), RecoilPitchImpulse, AdjustedPitchAlpha, AdditinalRandomizedImpulsePitch)

			RecoilTimeline->PlayFromStart();
		}
	}
}

void AWeapon::TimelineCallback(float val)
{
	BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
	if (BracketsOwnerCharacter)
	{
		BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(BracketsOwnerCharacter->Controller) : BracketsOwnerController;
		if (BracketsOwnerController)
		{
			BracketsOwnerController->AddYawInput(FMath::Lerp(0.00f, (RecoilYawImpulse + AdditinalRandomizedImpulseYaw) * AdjustedYawAlpha, val));
			BracketsOwnerController->AddPitchInput(FMath::Lerp(0.00f, (RecoilPitchImpulse - AdditinalRandomizedImpulsePitch) * AdjustedPitchAlpha, val));
			//UE_LOG(LogTemp, Warning, TEXT("AddYawInput: %f"), FMath::Lerp(0.00f, (RecoilYawImpulse + AdditinalRandomizedImpulseYaw) * AdjustedYawAlpha, val))
			//UE_LOG(LogTemp, Warning, TEXT("AddPitchInput: %f"), FMath::Lerp(0.00f, (RecoilPitchImpulse - AdditinalRandomizedImpulsePitch) * AdjustedPitchAlpha, val))
		}
	}
}

void AWeapon::TimelineFinishedCallback()
{
	
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerRewind, COND_OwnerOnly);
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerRewind = !bPingTooHigh;
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherbodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bUsePickUp)
	{
		ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(OtherActor);
		if (BracketsCharacter && PickupWidget)
		{
			BracketsCharacter->SetOverlappingWeapon(this);
		}
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherbodyIndex)
{
	ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(OtherActor);
	if (BracketsCharacter && PickupWidget)
	{
		BracketsCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
	if (BracketsOwnerCharacter)
	{
		BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(BracketsOwnerCharacter->Controller) : BracketsOwnerController;
		if (BracketsOwnerController)
		{
			BracketsOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BracketsOwnerCharacter = nullptr;
		BracketsOwnerController = nullptr;
	}
	else
	{
		//SetHUDAmmo();
		//UE_LOG(LogTemp, Error, TEXT("OnRep_Owner"))
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//UE_LOG(LogTemp, Warning, TEXT("EWS_Equipped"))

		ShowPickupWidget(false);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
		if (BracketsOwnerCharacter && bUseServerRewind)
		{
			BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(BracketsOwnerCharacter->Controller) : BracketsOwnerController;
			if (BracketsOwnerController && HasAuthority() && !BracketsOwnerController->HighPingDelegate.IsBound())
			{
				BracketsOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}

		break;
	case EWeaponState::EWS_Dropped:
		//UE_LOG(LogTemp, Warning, TEXT("EWS_Dropped"))
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
		if (BracketsOwnerCharacter && bUseServerRewind)
		{
			BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(BracketsOwnerCharacter->Controller) : BracketsOwnerController;
			if (BracketsOwnerController && HasAuthority() && BracketsOwnerController->HighPingDelegate.IsBound())
			{
				BracketsOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}
		break;
	case EWeaponState::EWS_Holstered:
		//UE_LOG(LogTemp, Warning, TEXT("EWS_Holstered"))
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : BracketsOwnerCharacter;
		if (BracketsOwnerCharacter && bUseServerRewind)
		{
			BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(BracketsOwnerCharacter->Controller) : BracketsOwnerController;
			if (BracketsOwnerController && HasAuthority() && BracketsOwnerController->HighPingDelegate.IsBound())
			{
				BracketsOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
			}
		}
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::Dropped()
{
	SetOwner(nullptr);
	BracketsOwnerCharacter = nullptr;
	BracketsOwnerController = nullptr;
	Destroy();
}

void AWeapon::ShaderPOV(int32 FOV)
{
	if (DynamicGunMatierialInstance)
	{
		DynamicGunMatierialInstance->SetScalarParameterValue(FName{ TEXT("FOV") }, FOV);
	}
}

void AWeapon::SetMaterialView(bool isFirstPerson) // keep just in case
{
	if (GetOwner()->HasAuthority())
	{
		MulticastSetMaterialView(isFirstPerson);
	}
	else
	{
		if (isFirstPerson)
		{
			if (FirstPersonGunMatierialInstance)
			{
				if (WeaponMesh && WeaponMesh->GetMaterial(0))
				{
					DynamicGunMatierialInstance = UMaterialInstanceDynamic::Create(FirstPersonGunMatierialInstance, this);
					WeaponMesh->SetMaterial(0, DynamicGunMatierialInstance);
				}
			}
		}
		else
		{
			if (WeaponMesh && WeaponMesh->GetMaterial(0))
			{
				DynamicGunMatierialInstance = UMaterialInstanceDynamic::Create(WeaponMesh->GetMaterial(0), this);
				WeaponMesh->SetMaterial(0, DynamicGunMatierialInstance);
			}
		}
	}
}

void AWeapon::MulticastSetMaterialView_Implementation(bool isFirstPerson)
{
	if (isFirstPerson)
	{
		if (FirstPersonGunMatierialInstance)
		{
			if (WeaponMesh && WeaponMesh->GetMaterial(0))
			{
				DynamicGunMatierialInstance = UMaterialInstanceDynamic::Create(FirstPersonGunMatierialInstance, this);
				WeaponMesh->SetMaterial(0, DynamicGunMatierialInstance);
			}
		}
	}
	else
	{
		if (WeaponMesh && WeaponMesh->GetMaterial(0))
		{
			DynamicGunMatierialInstance = UMaterialInstanceDynamic::Create(WeaponMesh->GetMaterial(0), this);
			WeaponMesh->SetMaterial(0, DynamicGunMatierialInstance);
		}
	}
}