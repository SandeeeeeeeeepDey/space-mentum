// ==============================================================================
// SpaceMentum — Editor build target.
// Identical module set to the game target; used when opening the project in UE 5.8.
// ==============================================================================

using UnrealBuildTool;
using System.Collections.Generic;

public class SpaceMentumEditorTarget : TargetRules
{
	public SpaceMentumEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
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
