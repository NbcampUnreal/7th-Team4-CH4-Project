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
		GlobalDefinitions.Add("UE_PROJECT_STEAMPRODUCTNAME=\"480\"");
		GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDIR=\"SagoMagic\"");
		GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDESC=\"SagoMagic\"");
		GlobalDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=480");
		ExtraModuleNames.Add("SagoMagic");
    }
}
