// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SagoMagicTarget : TargetRules
{
	public SagoMagicTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("SagoMagic");

		bUseLoggingInShipping = true;
		bOverrideBuildEnvironment = true;
	}
}
