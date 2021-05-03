// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;
using System.Collections.Generic;

public class GDKShooterTarget : TargetRules
{
	public GDKShooterTarget(TargetInfo Target) : base(Target)
	{
        // bUseMallocProfiler = true;
        // bOmitFramePointers = false;

		Type = TargetType.Game;
		ExtraModuleNames.Add("GDKShooter");
    }
}
