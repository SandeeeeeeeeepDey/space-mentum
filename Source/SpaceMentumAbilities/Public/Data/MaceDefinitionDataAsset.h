/**
 * @file MaceDefinitionDataAsset.h
 * @brief Designer-authored definition of a mace: its physical feel and its visual identity.
 *
 * @creator_feel
 * "the person has a mace in his hand."
 * "lets go for quality" — the mace must look and feel genuinely heavy.
 *
 * @emotional_intent
 * Each mace is a different personality: a light whip that kicks often but softly, or a
 * colossal wrecking ball that winds up slowly and launches you across the corridor.
 * Designers create DA_Mace_* assets from this class and tune feel without recompiling.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "MaceDefinitionDataAsset.generated.h"

class UStaticMesh;
class UNiagaraSystem;

UCLASS(BlueprintType)
class SPACEMENTUMABILITIES_API UMaceDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Human-readable name shown in UI and debug overlays. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Wind-up feel: tether length, mass, spin acceleration, speed cap. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
	FMaceOrbitTuning OrbitTuning;

	/** Kick feel: efficiency, direction model, damping, caps. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Physics")
	FMomentumTransferTuning TransferTuning;

	/** Mace head mesh. Soft reference so unused maces never bloat memory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> HeadMesh;

	/** Orbital ribbon trail (NS_MacePlasmaTrail): cyan -> ultraviolet as omega rises (GDD §3.1.4). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UNiagaraSystem> TrailSystem;
};
