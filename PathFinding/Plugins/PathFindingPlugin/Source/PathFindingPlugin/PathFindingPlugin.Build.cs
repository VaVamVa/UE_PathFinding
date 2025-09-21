// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PathFindingPlugin : ModuleRules
{
	public PathFindingPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Json",
			"JsonUtilities",
			"AIModule",
			"UMG",
			"LevelSequence"/*250921*/,
			"MovieScene"/*250921*/
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});

		PublicIncludePaths.Add(ModuleDirectory);
	}
}