using UnrealBuildTool;

public class Project5Editor : ModuleRules
{
	public Project5Editor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
            	"Core", 
            	"CoreUObject", 
            	"Engine", 
		        "Project5",
                "GameplayTags"
            });

		PrivateDependencyModuleNames.AddRange(new string[] {
           	"UnrealEd",        // Essential for Editor classes
            	"BlueprintGraph",  // Essential for K2Nodes
            	"KismetCompiler",  // Essential for ExpandNode
            	"GraphEditor",
                "Slate",
                "SlateCore",
                "ToolMenus",
                "AssetRegistry"
            });
	}
}