// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SagoMagicServerTarget : TargetRules
{
	public SagoMagicServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("SagoMagic");
        RegisterModulesCreatedByRider();
    }

    private void RegisterModulesCreatedByRider()
    {
        ExtraModuleNames.AddRange(new string[] { "GitStatusBranch" });
    }
}
