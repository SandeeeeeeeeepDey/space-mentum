/**
 * @file SpaceMaterialPaletteDataAsset.h
 * @brief High-fidelity PBR material palette and sector visual grading configuration.
 *
 * @creator_feel
 * "lets go for quality and hence unreal engine make most sense to me"
 * "abandon realistic space spacing. The levels need to be incredibly dense navigational hazards...
 *  asteroid belt also can be a way."
 *
 * @emotional_intent
 * DeepMind-grade configuration asset: allows technical artists and designers to customize
 * physical materials, metallic/roughness curves, emissive power, and sector atmospheric
 * lighting grades (Perseus Shallows cyan, Kepler Graveyard amber, Tartarus Belt ultraviolet)
 * without touching C++ solvers or recompiling code.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpaceMaterialPaletteDataAsset.generated.h"

class UMaterialInterface;

/** Configuration for an individual PBR surface material. */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FHighFidelityMaterialConfig
{
	GENERATED_BODY()

	/** Surface name identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FString SurfaceName = TEXT("Default");

	/** Base albedo tint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FLinearColor AlbedoTint = FLinearColor::White;

	/** PBR metallic rating [0.0 = dielectric, 1.0 = pure metal]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Metallic = 0.85f;

	/** PBR surface micro-roughness [0.0 = polished mirror, 1.0 = matte chalk]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Roughness = 0.25f;

	/** Baseline emissive color glow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FLinearColor EmissiveColor = FLinearColor(0.0f, 0.95f, 1.0f);

	/** Multiplier for emissive channel into Lumen / Bloom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (ClampMin = "0.0"))
	float EmissiveIntensity = 2.0f;

	/** Reference to master Unreal Engine material interface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	TSoftObjectPtr<UMaterialInterface> MasterMaterial;
};

/** Atmospheric visual grade profile for a specific celestial sector. */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FSectorVisualGradeProfile
{
	GENERATED_BODY()

	/** Sector index (1, 2, 3). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	int32 SectorIndex = 1;

	/** Human-readable sector name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	FString SectorName = TEXT("Perseus Shallows");

	/** Volumetric exponential deep space fog color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	FLinearColor DeepSpaceFogColor = FLinearColor(0.02f, 0.03f, 0.06f);

	/** Fog density multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	float FogDensity = 0.00028f;

	/** Primary key celestial starlight color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	FLinearColor KeyLightColor = FLinearColor(0.0f, 0.95f, 1.0f);

	/** Key light intensity (Lux / Lumens). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	float KeyLightIntensity = 3.5f;

	/** Back-rim starlight color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	FLinearColor RimLightColor = FLinearColor(0.74f, 0.0f, 1.0f);

	/** Cosmic hyperspace dust tint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SectorGrade")
	FLinearColor CosmicDustTint = FLinearColor(0.0f, 0.95f, 1.0f);
};

/**
 * Master Data Asset containing high-fidelity PBR material palettes and sector visual grading.
 */
UCLASS(BlueprintType)
class SPACEMENTUMCORE_API USpaceMaterialPaletteDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	USpaceMaterialPaletteDataAsset();

	/** Astronaut spacesuit PBR configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig SpacesuitMaterial;

	/** Articulated mechanical arm hard-surface configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig MechanicalArmMaterial;

	/** Heavy kinetic mace head PBR configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig MaceHeadMaterial;

	/** Asteroid rocky regolith PBR configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig AsteroidMaterial;

	/** Station wreckage damaged titanium hull configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig StationWreckageMaterial;

	/** Exit jump portal gravitational lens vortex configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig ExitPortalMaterial;

	/** Milestone checkpoint arch neon holo configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Surfaces")
	FHighFidelityMaterialConfig MilestoneGateMaterial;

	/** Sector visual grading profiles. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Sectors")
	TArray<FSectorVisualGradeProfile> SectorProfiles;

	/** Returns the visual grade profile for a specific sector index. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Sectors")
	FSectorVisualGradeProfile GetProfileForSector(int32 InSectorIndex) const;
};
