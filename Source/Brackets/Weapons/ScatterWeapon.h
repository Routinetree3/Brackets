// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ScatterWeapon.generated.h"

UCLASS()
class BRACKETS_API AScatterWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	//virtual void Fire(const FVector& HitTarget, bool IsAiming) override;
	virtual void FireScatter(const TArray<FVector_NetQuantize>& HitTargets, bool IsAiming);
	//FVector TraceEndWithScatter(const FVector& HitTarget);
	void FillScatteredHitTargets(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

protected:



	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit, FHitResult& EffectHit);

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		uint32 NumberOfPellets = 10;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		UParticleSystem* BeamParticles;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseScatter = false;

};
