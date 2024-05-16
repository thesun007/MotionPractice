// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DJGame : ModuleRules
{
	public DJGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "DJGame" });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ModularGameplay",
            "GameFeatures",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "ModularGameplayActors",
			"PhysicsCore",
			"UMG",
			"Slate", 
			"SlateCore",
            "AIModule",
            "MotionWarping",
            "AnimationModifiers",
            "AnimationBlueprintLibrary"
        }
		);

		PrivateDependencyModuleNames.AddRange(new string[] {
            "GameplayMessageRuntime",
            "CommonUI",
            "CommonGame",
            "CommonUser",
            "NetCore",
            "EditorScriptingUtilities"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
        
        //¿Ã∞‘ π∫∞°
        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);
    }
}
