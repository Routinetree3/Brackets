// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Brackets : ModuleRules
{
	public Brackets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "MultiplayerSession", "AdvancedSessions", "OnlineSubsystem" });
	}
}
