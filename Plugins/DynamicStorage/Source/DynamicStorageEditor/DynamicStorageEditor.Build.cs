using UnrealBuildTool;

public class DynamicStorageEditor : ModuleRules
{
    public DynamicStorageEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", "CoreUObject", "Engine",
            "DynamicStorage", "GameplayTags"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "BlueprintGraph", "KismetCompiler", "GraphEditor",
            "Slate", "SlateCore", "ToolMenus",
            "AssetRegistry", "GameplayTagsEditor"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}