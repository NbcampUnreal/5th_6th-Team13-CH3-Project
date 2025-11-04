// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Team13_Project : ModuleRules
{
	public Team13_Project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"AIModule",
			"GameplayTasks",
			"NavigationSystem",
			"Niagara",
			"DeveloperSettings",
            "Niagara"
        });
	}
}
