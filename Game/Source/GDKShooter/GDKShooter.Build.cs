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

        //PublicIncludePaths.AddRange(
        //                new string[] {
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Source/")),
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Source/BlastRuntime/Public/extensions/shaders/include")),
        //                }
        //            );


        //PrivateIncludePaths.AddRange(
        //                new string[] {
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Source/")),
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Source/BlastRuntime/Public/extensions/shaders/include")),
        //                }
        //            );


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
                "AIModule"
                // "Blast",
                // "BlastRuntime"
			});

        PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                // "Blast",
                // "BlastRuntime"
                    });

        // yunjie: without adding library paths, server worker and client worker will be failed to start due to can not find the blast libraries
        //PublicLibraryPaths.AddRange(
        //            new string[] {
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Libraries/Win64")),
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Libraries/Linux")),
        //                }
        //    );
        //PublicRuntimeLibraryPaths.AddRange(
        //                new string[] {
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Libraries/Win64")),
        //            Path.GetFullPath(Path.Combine(engine_path, "Plugins/GameWorks/Blast/Libraries/Linux")),
        //                }

        //    );


        //if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
        //{
        //    PublicDelayLoadDLLs.AddRange(
        //        new string[]
        //        {
        //            "NvBlast_x64.dll",
        //            "NvBlastExtSerialization_x64.dll",
        //            "NvBlastExtShaders_x64.dll",
        //            "NvBlastExtStress_x64.dll",
        //            "NvBlastGlobals_x64.dll",
        //        });
        //}
    }
}

