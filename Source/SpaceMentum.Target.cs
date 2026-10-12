// ==============================================================================
// SpaceMentum — Game build target.
//
// @creator_feel
//   "lets go for quality and hence unreal engine make most sense to me"
//
// Module order mirrors the dependency graph (low -> high):
//   SpaceMentumPhysics  ->  SpaceMentumAbilities  ->  SpaceMentumCore
// See Docs/TECHNICAL_SPECIFICATIONS.md §2 for the rationale.
// ==============================================================================

using UnrealBuildTool;
using System.Collections.Generic;

public class SpaceMentumTarget : TargetRules
{
	public SpaceMentumTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[]
		{
			"SpaceMentumPhysics",
			"SpaceMentumAbilities",
			"SpaceMentumCore",
		});
	}
}
