// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LeagueofLegendsTarget : TargetRules
{
	public LeagueofLegendsTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("LeagueofLegends");
		// RegisterModulesCreatedByRider();
	}

	/*
	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "SheetSync" });
	}*/
}
