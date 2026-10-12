// ==============================================================================
// SpaceMentumCore — Build rules (PRIMARY game module).
//
// @creator_feel
//   "the concept is a space person zooming in the forward direction in a uniform
//    velocity and the person has a mace in his hand."
//
// @emotional_intent
//   This is where the space person is born. Core composes the physics (Physics
//   module) and the flavour (Abilities module) into one astronaut the player
//   inhabits through a cinematic chase camera. It is the TOP of the dependency graph.
// ==============================================================================

using UnrealBuildTool;

public class SpaceMentumCore : ModuleRules
{
	public SpaceMentumCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"SpaceMentumPhysics",
			"SpaceMentumAbilities",
		});
	}
}
