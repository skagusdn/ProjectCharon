using UnrealBuildTool;

public class ProjectCharonEditor : ModuleRules
{
    public ProjectCharonEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ProjectCharon",
                
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "Water",
                "UnrealEd",
                "DeveloperSettings",
                "EditorSubsystem"
                
            }
        );
    }
}