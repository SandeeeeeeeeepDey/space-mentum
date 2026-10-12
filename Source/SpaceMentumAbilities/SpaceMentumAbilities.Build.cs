// ==============================================================================
// SpaceMentumAbilities — Build rules.
//
// @creator_feel
//   "ill include abilities and obsticles through time with flavour."
//
// @emotional_intent
//   Flavour over time. This module is the data-driven playground where designers
//   add new maces, abilities, and hazards WITHOUT touching physics code. It sits
//   above SpaceMentumPhysics (it configures it) and below SpaceMentumCore (which
//   composes it into the astronaut).
// ==============================================================================

using UnrealBuildTool;

public class SpaceMentumAbilities : ModuleRules
{
	public SpaceMentumAbilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"SpaceMentumPhysics",
		});

		// Needed because data assets hold reflected soft references to UNiagaraSystem (mace trails, impact bursts).
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Niagara",
		});

		// NOTE: Never add SpaceMentumCore here — that would create a dependency cycle.
	}
}
