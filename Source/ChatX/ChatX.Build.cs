// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChatX : ModuleRules
{
	public ChatX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",

		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"ChatX",

			//UI
			"UMG", "Slate", "SlateCore",
		});
	}
}
