using UnrealBuildTool;

public class SheetSync : ModuleRules
{
    public SheetSync(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        if (Target.Type != TargetRules.TargetType.Editor)
        {
            throw new System.Exception("SheetSync is Editor only.");
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "HTTP",
                "Json",
                "DataTableEditor",
                "Slate",
                "SlateCore",
                "PropertyEditor"
            }
        );
    }
}