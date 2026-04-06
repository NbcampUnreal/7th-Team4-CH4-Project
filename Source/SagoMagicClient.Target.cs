// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SagoMagicClientTarget : TargetRules
{
	public SagoMagicClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
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
