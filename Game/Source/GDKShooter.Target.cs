// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;
using System.Collections.Generic;

public class GDKShooterTarget : TargetRules
{
	public GDKShooterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("GDKShooter");

		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			GlobalDefinitions.Add("FORCE_ANSI_ALLOCATOR=1");
		}
	}
}
