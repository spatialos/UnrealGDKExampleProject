// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;
using System.Collections.Generic;

public class GDKShooterTarget : TargetRules
{
	public GDKShooterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("GDKShooter");
		// TODO: UNR-1791 for long-term fix
		GlobalDefinitions.Add("UE_ALLOW_MAP_OVERRIDE_IN_SHIPPING=1");
	}
}
