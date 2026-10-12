// ==============================================================================
// SpaceMentumPhysics — Build rules.
//
// @creator_feel
//   "by spinning and stopping the spin in a pertiicular direction, one can stir
//    their direction to move away from obsticles ... it has physics."
//
// @emotional_intent
//   This module is the beating heart: the orbit math, the whiplash kick, the
//   relentless forward drift. It is deliberately the LOWEST layer and depends on
//   nothing else in the game, so the "feel" can be tuned and unit-tested in isolation.
// ==============================================================================

using UnrealBuildTool;

public class SpaceMentumPhysics : ModuleRules
{
	public SpaceMentumPhysics(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		// NOTE: Never add SpaceMentumCore or SpaceMentumAbilities here.
		// Physics must remain the dependency-free foundation (see TECHNICAL_SPECIFICATIONS.md §2).
	}
}
