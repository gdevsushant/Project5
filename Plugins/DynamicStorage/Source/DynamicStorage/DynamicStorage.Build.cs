using UnrealBuildTool;

public class DynamicStorage : ModuleRules
{
	public DynamicStorage(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependencies are available to other modules/projects using your plugin
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"GameplayTags" 
		});

		// Private dependencies are only used inside this module
		PrivateDependencyModuleNames.AddRange(new string[] {
			"DeveloperSettings" // Necessary if you use UDeveloperSettings for your project settings
		});
	}
}