/**
 * @file SpaceMaterialPaletteDataAsset.cpp
 * @brief Implementation and default values for high-fidelity PBR material palettes and sector visual grading.
 */

#include "Data/SpaceMaterialPaletteDataAsset.h"

USpaceMaterialPaletteDataAsset::USpaceMaterialPaletteDataAsset()
{
	// 1. Spacesuit PBR Defaults
	SpacesuitMaterial.SurfaceName = TEXT("Spacesuit");
	SpacesuitMaterial.AlbedoTint = FLinearColor(0.88f, 0.91f, 0.95f);
	SpacesuitMaterial.Metallic = 0.85f;
	SpacesuitMaterial.Roughness = 0.20f;
	SpacesuitMaterial.EmissiveColor = FLinearColor(0.0f, 0.95f, 1.0f);
	SpacesuitMaterial.EmissiveIntensity = 2.0f;
	SpacesuitMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_Astronaut_Suit_PBR.M_Astronaut_Suit_PBR"))
	);

	// 2. Mechanical Arm Hard-Surface Defaults
	MechanicalArmMaterial.SurfaceName = TEXT("MechanicalArm");
	MechanicalArmMaterial.AlbedoTint = FLinearColor(0.28f, 0.33f, 0.41f);
	MechanicalArmMaterial.Metallic = 0.92f;
	MechanicalArmMaterial.Roughness = 0.22f;
	MechanicalArmMaterial.EmissiveColor = FLinearColor(0.0f, 0.95f, 1.0f);
	MechanicalArmMaterial.EmissiveIntensity = 1.5f;
	MechanicalArmMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_MechanicalArm_HardSurface.M_MechanicalArm_HardSurface"))
	);

	// 3. Mace Head Heavy Kinetic Ram Defaults
	MaceHeadMaterial.SurfaceName = TEXT("MaceHead");
	MaceHeadMaterial.AlbedoTint = FLinearColor(0.12f, 0.16f, 0.23f);
	MaceHeadMaterial.Metallic = 0.95f;
	MaceHeadMaterial.Roughness = 0.18f;
	MaceHeadMaterial.EmissiveColor = FLinearColor(0.97f, 0.15f, 0.52f);
	MaceHeadMaterial.EmissiveIntensity = 3.5f;
	MaceHeadMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_MaceHead_Heavy.M_MaceHead_Heavy"))
	);

	// 4. Asteroid PBR Defaults
	AsteroidMaterial.SurfaceName = TEXT("Asteroid");
	AsteroidMaterial.AlbedoTint = FLinearColor(0.22f, 0.20f, 0.18f);
	AsteroidMaterial.Metallic = 0.25f;
	AsteroidMaterial.Roughness = 0.85f;
	AsteroidMaterial.EmissiveColor = FLinearColor::Black;
	AsteroidMaterial.EmissiveIntensity = 0.0f;
	AsteroidMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_Asteroid_PBR.M_Asteroid_PBR"))
	);

	// 5. Station Wreckage Defaults
	StationWreckageMaterial.SurfaceName = TEXT("StationWreckage");
	StationWreckageMaterial.AlbedoTint = FLinearColor(0.40f, 0.38f, 0.35f);
	StationWreckageMaterial.Metallic = 0.80f;
	StationWreckageMaterial.Roughness = 0.35f;
	StationWreckageMaterial.EmissiveColor = FLinearColor(1.0f, 0.45f, 0.05f);
	StationWreckageMaterial.EmissiveIntensity = 1.8f;
	StationWreckageMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_StationWreckage_PBR.M_StationWreckage_PBR"))
	);

	// 6. Exit Jump Portal Defaults
	ExitPortalMaterial.SurfaceName = TEXT("ExitPortal");
	ExitPortalMaterial.AlbedoTint = FLinearColor(0.0f, 0.95f, 1.0f);
	ExitPortalMaterial.Metallic = 0.95f;
	ExitPortalMaterial.Roughness = 0.05f;
	ExitPortalMaterial.EmissiveColor = FLinearColor(0.0f, 0.95f, 1.0f);
	ExitPortalMaterial.EmissiveIntensity = 8.0f;
	ExitPortalMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_ExitPortal_WarpVortex.M_ExitPortal_WarpVortex"))
	);

	// 7. Milestone Checkpoint Gate Defaults
	MilestoneGateMaterial.SurfaceName = TEXT("MilestoneGate");
	MilestoneGateMaterial.AlbedoTint = FLinearColor(0.0f, 0.95f, 1.0f);
	MilestoneGateMaterial.Metallic = 0.90f;
	MilestoneGateMaterial.Roughness = 0.10f;
	MilestoneGateMaterial.EmissiveColor = FLinearColor(0.0f, 0.95f, 1.0f);
	MilestoneGateMaterial.EmissiveIntensity = 4.0f;
	MilestoneGateMaterial.MasterMaterial = TSoftObjectPtr<UMaterialInterface>(
		FSoftObjectPath(TEXT("/Game/Environment/Materials/M_MilestoneGate_NeonHolo.M_MilestoneGate_NeonHolo"))
	);

	// 8. Sector Visual Grading Profiles
	FSectorVisualGradeProfile Perseus;
	Perseus.SectorIndex = 1;
	Perseus.SectorName = TEXT("Perseus Shallows");
	Perseus.DeepSpaceFogColor = FLinearColor(0.02f, 0.03f, 0.06f);
	Perseus.FogDensity = 0.00028f;
	Perseus.KeyLightColor = FLinearColor(0.0f, 0.95f, 1.0f); // Key Cyan
	Perseus.KeyLightIntensity = 3.5f;
	Perseus.RimLightColor = FLinearColor(0.74f, 0.0f, 1.0f); // Rim Magenta
	Perseus.CosmicDustTint = FLinearColor(0.0f, 0.95f, 1.0f);
	SectorProfiles.Add(Perseus);

	FSectorVisualGradeProfile Kepler;
	Kepler.SectorIndex = 2;
	Kepler.SectorName = TEXT("Kepler Graveyard");
	Kepler.DeepSpaceFogColor = FLinearColor(0.06f, 0.03f, 0.01f);
	Kepler.FogDensity = 0.00034f;
	Kepler.KeyLightColor = FLinearColor(1.0f, 0.55f, 0.1f); // Key Amber
	Kepler.KeyLightIntensity = 3.8f;
	Kepler.RimLightColor = FLinearColor(0.0f, 0.85f, 1.0f); // Rim Cyan
	Kepler.CosmicDustTint = FLinearColor(1.0f, 0.6f, 0.15f);
	SectorProfiles.Add(Kepler);

	FSectorVisualGradeProfile Tartarus;
	Tartarus.SectorIndex = 3;
	Tartarus.SectorName = TEXT("Tartarus Belt");
	Tartarus.DeepSpaceFogColor = FLinearColor(0.04f, 0.01f, 0.05f);
	Tartarus.FogDensity = 0.00042f;
	Tartarus.KeyLightColor = FLinearColor(0.85f, 0.15f, 0.95f); // Key Violet
	Tartarus.KeyLightIntensity = 4.2f;
	Tartarus.RimLightColor = FLinearColor(1.0f, 0.75f, 0.0f); // Rim Gold
	Tartarus.CosmicDustTint = FLinearColor(0.85f, 0.15f, 0.95f);
	SectorProfiles.Add(Tartarus);
}

FSectorVisualGradeProfile USpaceMaterialPaletteDataAsset::GetProfileForSector(int32 InSectorIndex) const
{
	for (const FSectorVisualGradeProfile& Profile : SectorProfiles)
	{
		if (Profile.SectorIndex == InSectorIndex)
		{
			return Profile;
		}
	}

	// Fallback to Sector 1 if not found
	if (SectorProfiles.Num() > 0)
	{
		return SectorProfiles[0];
	}

	FSectorVisualGradeProfile DefaultProfile;
	DefaultProfile.SectorIndex = InSectorIndex;
	return DefaultProfile;
}
