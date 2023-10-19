#pragma once

UENUM(BlueprintType)
enum class EHealthState : uint8
{
	EHS_Alive UMETA(DisplayName = "Alive"),
	EHS_Dead UMETA(DisplayName = "Dead"),

	EHS_MAX UMETA(DisplayName = "DefaultMAX"),
};