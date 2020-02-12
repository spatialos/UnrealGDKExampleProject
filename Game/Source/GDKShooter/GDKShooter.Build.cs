// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using UnrealBuildTool;

public class GDKShooter : ModuleRules
{
	public GDKShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Sockets",
				"OnlineSubsystemUtils",
				"PhysXVehicles",
				"UMG",
				"Slate",
				"SlateCore",
				"SpatialGDK",
                "Json",
                "JsonUtilities",
                "HTTP",
                "AIModule"
			});
	}
}
