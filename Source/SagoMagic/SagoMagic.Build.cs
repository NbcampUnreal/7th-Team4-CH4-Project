// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SagoMagic : ModuleRules
{
	public SagoMagic(ReadOnlyTargetRules Target) : base(Target)
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
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "GameplayMessageRuntime",
            "OnlineSubsystem",
            "AnimGraphRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
        {

        });

		PublicIncludePaths.AddRange(new string[]
        {
			"SagoMagic",
			"SagoMagic/SampleProject",
			"SagoMagic/SampleProject/Variant_Strategy",
			"SagoMagic/SampleProject/Variant_Strategy/UI",
			"SagoMagic/SampleProject/Variant_TwinStick",
			"SagoMagic/SampleProject/Variant_TwinStick/AI",
			"SagoMagic/SampleProject/Variant_TwinStick/Gameplay",
			"SagoMagic/SampleProject/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
