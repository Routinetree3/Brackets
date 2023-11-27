#pragma once

#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Carbine UMETA(DisplayName = "Carbine"),
	EWT_SMG UMETA(DisplayName = "SMG"),
	EWT_BattleRifle UMETA(DisplayName = "Battle Rifle"),
	EWT_SimiPistol UMETA(DisplayName = "Simi Pistol"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),

	EWT_LethalGranade UMETA(DisplayName = "LethalGranade"),
	EWT_NonLethalGranade UMETA(DisplayName = "NonLethalGranade"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};
