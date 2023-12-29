// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "ThrowableProjectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UTexture2D;

UENUM(BlueprintType)
enum class EThrowableType : uint8
{
	ETT_Explosive UMETA(DisplayName = "ExplosiveGranade"),
	ETT_Knife UMETA(DisplayName = "ThrowingKnife"),

	ETT_Smoke UMETA(DisplayName = "SmokeGranade"),
	ETT_Flash UMETA(DisplayName = "FlashGranade"),
	ETT_Decoy UMETA(DisplayName = "DecoyGranade"),

	ETT_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class BRACKETS_API AThrowableProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrowableProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void SpawnLoopSound();
	void RadialDamage();
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;
	UPROPERTY(EditAnywhere)
		float MinimunDamage = 10.f;
	UPROPERTY(EditAnywhere)
		float InnerDamageRadius = 200.f;
	UPROPERTY(EditAnywhere)
		float OuterDamageRadius = 500.f;
	UPROPERTY(EditAnywhere)
		float Falloff = 1.f;

	UPROPERTY(EditAnywhere)
		float Speed = 800.f;

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
		USoundCue* ImpactSound;
	UPROPERTY(EditAnywhere)
		UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		USoundCue* BounceSound;
	UPROPERTY(EditAnywhere)
		USoundCue* ProjectileLoop;
	UPROPERTY()
		UAudioComponent* ProjectileLoopComponent;
	UPROPERTY(EditAnywhere)
		USoundAttenuation* LoopingSoundAttenuation;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;

	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;
	UPROPERTY(EditAnywhere)
		EThrowableType ThrowableType;
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere)
		UTexture2D* Silhouette;

public:	
	// Called every frame
	UPROPERTY(VisibleAnywhere)
		UProjectileMovementComponent* ProjectileMovementComponent;
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE UTexture2D* GetSilhouette() const { return Silhouette; }
	FORCEINLINE EThrowableType GetThrowableType() const { return ThrowableType; }
	FString GetThrowTypeName();

};
