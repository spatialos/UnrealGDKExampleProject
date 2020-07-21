// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

using System.IO;
using UnrealBuildTool;

public class GDKShooter : ModuleRules
{
	public GDKShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // bLegacyPublicIncludePaths = false;

        string engine_path = Path.GetFullPath(Target.RelativeEnginePath);

        PrivateIncludePaths.AddRange(
                        new string[] {
                    Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Source/")),
                    Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Source/BlastRuntime/Public/extensions/shaders/include")),
                        }
                    );


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
                "HTTP",
                "AIModule",
                "Blast",
                "BlastRuntime"
			});

        PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                "Blast",
                "BlastRuntime"
                    });

    }
}
