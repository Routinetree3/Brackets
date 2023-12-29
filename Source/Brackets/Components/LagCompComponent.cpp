// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompComponent.h"
#include "Brackets/BracketsCharacter.h"
#include "Brackets/Weapons/Weapon.h"
#include "Brackets/Weapons/ScatterWeapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GamePlayStatics.h"
#include "DrawDebugHelpers.h"
#include "Brackets/Brackets.h"


ULagCompComponent::ULagCompComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void ULagCompComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULagCompComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) { return; }
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
}

void ULagCompComponent::SaveFramePackage(FFramePackage& outPackage)
{
	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : Character;
	if (Character)
	{
		outPackage.Time = GetWorld()->GetTimeSeconds();
		outPackage.Character = Character;
		/*for (auto& BoxPair : Character->HitBoxes)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.Size = BoxPair.Value->GetScaledBoxExtent();
			//BoxInfo.DamageRatio = BoxPair.Value->Get
			outPackage.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}*/
		for (auto& BoxPair : Character->HitBoxes)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = BoxPair.Value.HitBoxComponent->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value.HitBoxComponent->GetComponentRotation();
			BoxInfo.Size = BoxPair.Value.HitBoxComponent->GetScaledBoxExtent();
			BoxInfo.DamageRatio = BoxPair.Value.DamageRatio;
			outPackage.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}
	}
}

FFramePackage ULagCompComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInfo& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInfo& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInfo InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.Size = YoungerBox.Size;
		InterpBoxInfo.DamageRatio = YoungerBox.DamageRatio;
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

void ULagCompComponent::CacheBoxPositions(ABracketsCharacter* HitCharacter, FFramePackage& outFramePackage)
{
	if (HitCharacter == nullptr) { return; }
	/*for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.Size = HitBoxPair.Value->GetScaledBoxExtent();
			outFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}*/
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		if (HitBoxPair.Value.HitBoxComponent != nullptr)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = HitBoxPair.Value.HitBoxComponent->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value.HitBoxComponent->GetComponentRotation();
			BoxInfo.Size = HitBoxPair.Value.HitBoxComponent->GetScaledBoxExtent();
			outFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompComponent::MoveHitBoxes(ABracketsCharacter* HitCharacter, const FFramePackage& Package, bool bResetCollision)
{
	if (HitCharacter == nullptr) { return; }
	for (auto& HitBoxPair : HitCharacter->HitBoxes)
	{
		/*if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].Size);
			if (bResetCollision)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}*/
		if (HitBoxPair.Value.HitBoxComponent != nullptr)
		{
			HitBoxPair.Value.HitBoxComponent->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value.HitBoxComponent->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value.HitBoxComponent->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].Size);
			if (bResetCollision)
			{
				HitBoxPair.Value.HitBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void ULagCompComponent::EnableCharacterMeshCollision(ABracketsCharacter* HitCharacter, ECollisionEnabled::Type Collision)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(Collision);
	}
}

void ULagCompComponent::ShowFramePackage(const FFramePackage& Package, const FColor Color)
{
	/*for (auto& BoxInfo : Package.HitBoxInfo)
	{

	}*/
}

FServerSideRewindResult ULagCompComponent::ConfirmHit(const FFramePackage& Package, ABracketsCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveHitBoxes(HitCharacter, Package, false);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.10f;
	UWorld* World = GetWorld();
	if (World)
	{
		for (auto& HitBoxPair : HitCharacter->HitBoxes)
		{
			if (HitBoxPair.Value.HitBoxComponent != nullptr)
			{
				HitBoxPair.Value.HitBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				HitBoxPair.Value.HitBoxComponent->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

		Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : Character;
		struct FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);
		QueryParams.AddIgnoredComponent(Character->GetHead());

		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox,
			QueryParams
		);

		if (ConfirmHitResult.bBlockingHit)
		{
			const FName BoxThatWasHit = ConfirmHitResult.GetComponent()->GetFName();
			UE_LOG(LogTemp, Error, TEXT("BoxThatWasHit: %s"), *BoxThatWasHit.ToString())
			auto BoxHitInfo = Package.HitBoxInfo.Find(BoxThatWasHit); 
			float HitDamageRatio = 1;
			if (BoxHitInfo)
			{
				HitDamageRatio = BoxHitInfo->DamageRatio;
				UE_LOG(LogTemp, Error, TEXT("HitDamageRatio: %f"), HitDamageRatio)
			}

			if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, true);
				}
			}

			MoveHitBoxes(HitCharacter, CurrentFrame, true);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, HitDamageRatio };
		}
	}
	MoveHitBoxes(HitCharacter, CurrentFrame, true);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, 1.f };
}

FScatterServerSideRewindResult ULagCompComponent::ScatterConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages){if (Frame.Character == nullptr) { FScatterServerSideRewindResult(); }}
	FScatterServerSideRewindResult ScatterResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveHitBoxes(Frame.Character, Frame, false);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitBoxes)
		{
			if (HitBoxPair.Value.HitBoxComponent != nullptr)
			{
				HitBoxPair.Value.HitBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				HitBoxPair.Value.HitBoxComponent->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
	}

	Character = Character == nullptr ? Cast<ABracketsCharacter>(GetOwner()) : Character;
	struct FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.AddIgnoredComponent(Character->GetHead());

	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.05f;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox,
				QueryParams
			);

			float HitDamageRatio = 1;
			if (ConfirmHitResult.bBlockingHit)
			{
				const FName BoxThatWasHit = ConfirmHitResult.GetComponent()->GetFName();
				auto BoxHitInfo = FramePackages[0].HitBoxInfo.Find(BoxThatWasHit);
				if (BoxHitInfo)
				{
					HitDamageRatio = BoxHitInfo->DamageRatio; 
				}
			}

			ABracketsCharacter* BracketsCharacter = Cast<ABracketsCharacter>(ConfirmHitResult.GetActor());
			if (BracketsCharacter)
			{
				if (ScatterResult.HitsInfo.Contains(BracketsCharacter))
				{
					ScatterResult.HitsInfo[BracketsCharacter].Ratios.Add(HitDamageRatio);
				}
				else
				{
					FScatterHitInfo HitInfo;
					HitInfo.Ratios.Add(HitDamageRatio);
					ScatterResult.HitsInfo.Emplace(BracketsCharacter, HitInfo);
				}
			}

			if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, true);
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		MoveHitBoxes(Frame.Character, Frame, true);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	return ScatterResult;
}

FServerSideRewindResult ULagCompComponent::ServerSideRewind(ABracketsCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FScatterServerSideRewindResult ULagCompComponent::ScatterServerSideRewind(const TArray<ABracketsCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (ABracketsCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ScatterConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

void ULagCompComponent::ServerScoreRequest_Implementation(ABracketsCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamage() * Confirm.DamageRatio,
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamage UseServerRewind: %f"), DamageCauser->GetDamage() * Confirm.DamageRatio)
	}
}

void ULagCompComponent::ServerScatterScoreRequest_Implementation(const TArray<ABracketsCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	FScatterServerSideRewindResult Confirm = ScatterServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || Character == nullptr || DamageCauser == nullptr) { continue; }
		float ShotDamage = 0.f;
		if (Confirm.HitsInfo.Contains(HitCharacter))
		{
			for (auto Ratio : Confirm.HitsInfo[HitCharacter].Ratios)
			{
				ShotDamage += DamageCauser->GetDamage() * Ratio;
				UE_LOG(LogTemp, Warning, TEXT("ShotDamage: %f"), ShotDamage)
			}
			UGameplayStatics::ApplyDamage(
				HitCharacter,
				ShotDamage,
				Character->Controller,
				DamageCauser,
				UDamageType::StaticClass()
			);
		}
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamage UseServerRewind: %f"), ShotDamage)
	}
}

FFramePackage ULagCompComponent::GetFrameToCheck(ABracketsCharacter* HitCharacter, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagComp() == nullptr ||
		HitCharacter->GetLagComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) { return FFramePackage(); }

	FFramePackage FrameToCheck;
	bool bShouldInterp = true;
	//History on the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagComp()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewistHistoryTime = History.GetHead()->GetValue().Time;

	if (OldestHistoryTime > HitTime) { return FFramePackage(); }
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterp = false;
	}
	if (NewistHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while (Older->GetValue().Time > HitTime)
	{
		if (Older->GetNextNode() == nullptr) { break; }
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime) { Younger = Older; }
	}
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
	}
	else if (bShouldInterp)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
}

