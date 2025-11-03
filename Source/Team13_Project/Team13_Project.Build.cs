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
<<<<<<< HEAD
			"Niagara",
			"DeveloperSettings"
		});
=======
            "Niagara"
        });
>>>>>>> c93b057877c6b775a465bb999e8d2d5e4c4d851d
	}
}
