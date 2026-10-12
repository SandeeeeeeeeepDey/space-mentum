/**
 * @file ObstacleArchetypeDataAsset.h
 * @brief Designer-authored hazard archetype: what it is, how often it spawns, how it reacts.
 *
 * @creator_feel
 * "one can stir their direction to move away from obsticles. ill include abilities and
 *  obsticles through time with flavour."
 *
 * @emotional_intent
 * Obstacles are the reason the kick matters. They should escalate in "flavour" over a
 * run — dormant debris first, then drifting comets and laser grids, then crystal barriers
 * that reward the bravest players for smashing through with the mace (GDD §2.1 F-3).
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObstacleArchetypeDataAsset.generated.h"

class UStaticMesh;
class UNiagaraSystem;

/** How a hazard behaves in the corridor (GDD §2.1 F-3 "Hazard Types"). */
UENUM(BlueprintType)
enum class EObstacleBehaviour : uint8
{
	/** Asteroids, debris, dormant satellites. */
	Static          UMETA(DisplayName = "Static"),
	/** Drifting comets, laser grids, gravitational anomalies. */
	Kinetic         UMETA(DisplayName = "Reactive / Kinetic"),
	/** Crystalline barriers — breakable only by a high-velocity mace impact. */
	MaceDestructible UMETA(DisplayName = "Mace-Destructible"),
};

UCLASS(BlueprintType)
class SPACEMENTUMABILITIES_API UObstacleArchetypeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Human-readable name for designers and debug overlays. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Behavioural family of this hazard. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour")
	EObstacleBehaviour Behaviour = EObstacleBehaviour::Static;

	/** Relative spawn weight within its difficulty band. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0"))
	float SpawnWeight = 1.0f;

	/** Earliest run time (seconds) at which this hazard may appear — "flavour through time". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", Units = "Seconds"))
	float MinRunTimeSeconds = 0.0f;

	/** For MaceDestructible: minimum mace head speed (cm/s) required to shatter it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behaviour", meta = (ClampMin = "0.0", EditCondition = "Behaviour == EObstacleBehaviour::MaceDestructible"))
	float BreakSpeedThresholdCmPerSec = 1500.0f;

	/** Visual mesh. Soft reference — loaded only when the archetype enters the spawn pool. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Burst played on destruction (NS_KineticImpactBurst). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UNiagaraSystem> DestructionBurst;
};
