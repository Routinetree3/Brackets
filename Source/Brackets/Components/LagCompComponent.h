// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
		FVector Location;
	UPROPERTY()
		FRotator Rotation;
	UPROPERTY()
		FVector Size;
	UPROPERTY()
		float DamageRatio;
};

USTRUCT(BlueprintType)
struct FScatterHitInfo
{
	GENERATED_BODY()

	UPROPERTY()
		TArray<float> Ratios;    
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
		float Time;
	UPROPERTY()
		TMap<FName, FBoxInfo> HitBoxInfo;
	UPROPERTY()
		ABracketsCharacter* Character;

};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
		bool bHitConfirmed;

	UPROPERTY()
		float DamageRatio;	

};

USTRUCT(BlueprintType)
struct FScatterServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
		TMap<ABracketsCharacter*, FScatterHitInfo> HitsInfo;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BRACKETS_API ULagCompComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompComponent();
	friend class ABracketsCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);
	FServerSideRewindResult ServerSideRewind(class ABracketsCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	FScatterServerSideRewindResult ScatterServerSideRewind(const TArray<ABracketsCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(
			ABracketsCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime,
			class AWeapon* DamageCauser
		);
	UFUNCTION(Server, Reliable)
		void ServerScatterScoreRequest(
			const TArray<ABracketsCharacter*>& HitCharacters,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations,
			float HitTime,
			AWeapon* DamageCauser
		);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage();
	void SaveFramePackage(FFramePackage& outPackage);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABracketsCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
	);
	FScatterServerSideRewindResult ScatterConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
	void CacheBoxPositions(ABracketsCharacter* HitCharacter, FFramePackage& outFramePackage);
	void MoveHitBoxes(ABracketsCharacter* HitCharacter, const FFramePackage& Package, bool bResetCollision);
	void EnableCharacterMeshCollision(ABracketsCharacter* HitCharacter, ECollisionEnabled::Type Collision);
	FFramePackage GetFrameToCheck(ABracketsCharacter* HitCharacter, float HitTime);

private:
	UPROPERTY()
	ABracketsCharacter* Character;
	UPROPERTY()
	class ABracketsPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;
	UPROPERTY(EditAnywhere)
		float MaxRecordTime = 2.f;
		
};
