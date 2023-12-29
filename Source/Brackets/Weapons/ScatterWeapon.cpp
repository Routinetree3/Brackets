// Fill out your copyright notice in the Description page of Project Settings.


#include "ScatterWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimationAsset.h"
#include "Brackets/BracketsCharacter.h"
#include "Brackets/Components/LagCompComponent.h"
#include "Brackets/Player/BracketsPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"
#include "Brackets/Brackets.h"

#include "DrawDebugHelpers.h"

void AScatterWeapon::FireScatter(const TArray<FVector_NetQuantize>& HitTargets, bool IsAiming)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) { return; }
	AController* InstigatorController = OwnerPawn->GetController();

	bIsAiming = IsAiming;

	if (RecoilTimeline && TimelineCurve)
	{
		AddRecoilImpulse();
	}

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	BracketsOwnerCharacter = BracketsOwnerCharacter == nullptr ? Cast<ABracketsCharacter>(OwnerPawn) : BracketsOwnerCharacter;
	BracketsOwnerController = BracketsOwnerController == nullptr ? Cast<ABracketsPlayerController>(InstigatorController) : BracketsOwnerController;
	if (MuzzleFlashSocket && BracketsOwnerCharacter && BracketsOwnerController)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//TMap<ABracketsCharacter*, uint32> HitMap;
		TMap<ABracketsCharacter*, FScatterHitInfo> HitMap;

		FName BoxThatWasHit;
		FHitBoxInfo* BoxHitInfo;

		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			FHitResult EffectHit;
			WeaponTraceHit(Start, HitTarget, FireHit, EffectHit);

			ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(FireHit.GetActor());
			if (BracketsCharacter)
			{
				BoxThatWasHit = FireHit.GetComponent()->GetFName();
				BoxHitInfo = BracketsOwnerCharacter->HitBoxes.Find(BoxThatWasHit);
				float HitDamageRatio = 1;
				if (BoxHitInfo)
				{
					HitDamageRatio = BoxHitInfo->DamageRatio;
				}

				if (HitMap.Contains(BracketsCharacter))
				{
					HitMap[BracketsCharacter].Ratios.Add(HitDamageRatio);
				}
				else
				{
					FScatterHitInfo Info;
					Info.Ratios.Add(HitDamageRatio);
					HitMap.Emplace(BracketsCharacter, Info);
				}
			}
			if (EffectHit.bBlockingHit)
			{
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						EffectHit.ImpactPoint,
						EffectHit.ImpactNormal.Rotation()
					);
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						EffectHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}
		}
		TArray<ABracketsCharacter*> HitCharacters;
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					float ShotDamage = 0.f;
					for (auto Ratio : HitPair.Value.Ratios)
					{
						ShotDamage += Damage * Ratio;
						UE_LOG(LogTemp, Warning, TEXT("ShotDamage: %f"), ShotDamage)
					}
					UGameplayStatics::ApplyDamage(
						HitPair.Key, //HitCharacter
						ShotDamage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
					UE_LOG(LogTemp, Warning, TEXT("ApplyDamage: %f"), ShotDamage)
				}
				HitCharacters.Add(HitPair.Key);
			}
		}
		if (!HasAuthority() && bUseServerRewind)
		{
			if (BracketsOwnerCharacter && BracketsOwnerController && BracketsOwnerCharacter->GetLagComp() && BracketsOwnerCharacter->IsLocallyControlled())
			{
				BracketsOwnerCharacter->GetLagComp()->ServerScatterScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BracketsOwnerController->GetServerTime() - BracketsOwnerController->SingleTripTime,
					this
				);
			}
		}
		SpendRound();
	}
}

void AScatterWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit, FHitResult& EffectHit)
{
	
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.05f;
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECC_HitBox
		);

		DrawDebugLine(GetWorld(), TraceStart, End, FColor::Blue, true);

		World->LineTraceSingleByChannel(
			EffectHit,
			TraceStart,
			End,
			ECC_Visibility
		);

		FVector BeamEnd = End;
		if (EffectHit.bBlockingHit)
		{
			BeamEnd = EffectHit.ImpactPoint;
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart, FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

/*FVector AScatterWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleFlashSocket == nullptr) return FVector();

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector TraceStart = SocketTransform.GetLocation();

	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;


	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Blue, true);


	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}*/

void AScatterWeapon::FillScatteredHitTargets(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleFlashSocket == nullptr) return;

	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector TraceStart = SocketTransform.GetLocation();

	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 index = 0; index < NumberOfPellets; index++)
	{
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}
