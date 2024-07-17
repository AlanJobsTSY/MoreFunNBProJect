// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TSY_Project : ModuleRules
{
	public TSY_Project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
