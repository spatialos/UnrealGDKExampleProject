using UnrealBuildTool;

public class ExampleProjectTests : ModuleRules
{
    public ExampleProjectTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "Engine",
            "UnrealEd",
            "CoreUObject",
            "GDKShooter"
        });
        PublicDependencyModuleNames.AddRange(new string[] {
            "SpatialGDKEditor",
            "SpatialGDKEditorToolbar",
            "SpatialGDK"
        });
    }
}
