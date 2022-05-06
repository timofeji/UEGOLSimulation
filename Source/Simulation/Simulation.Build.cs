// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Simulation : ModuleRules
{
	public Simulation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject",
			"Engine", "InputCore", "Niagara", "NiagaraEditor", 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

	}
}