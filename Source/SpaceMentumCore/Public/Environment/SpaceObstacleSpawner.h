/**
 * @file SpaceObstacleSpawner.h
 * @brief Procedural high-density hazard spawner for asteroid belts, orbital debris, and spaceship graveyards.
 *
 * @creator_feel
 * "abandon realistic space spacing. The levels need to be incredibly dense navigational hazards...
 *  asteroid belt also can be a way."
 * "The player is always only seconds away from their next obstacle or anchor point."
 *
 * @emotional_intent
 * Claustrophobia & High APM. Space is cluttered and dangerous. Reachable obstacles appear
 * every 2–5 seconds along the +X corridor, demanding continuous kinetic rhythm to survive.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Solvers/DoomTrajectoryEvaluator.h"
#include "Environment/SpaceObstacleActor.h"
#include "SpaceObstacleSpawner.generated.h"

/**
 * Configuration parameters for procedural hazard belt generation.
 */
USTRUCT(BlueprintType)
struct FSpaceObstacleSpawnerConfig
{
	GENERATED_BODY()

	/** Reference corridor radius (cm). Obstacles spawn within this radial envelope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "200.0"))
	float CorridorRadiusCm = 800.0f;

	/** Minimum time interval between obstacle clusters at reference forward speed (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "0.5"))
	float MinCadenceSeconds = 1.8f;

	/** Maximum time interval between obstacle clusters at reference forward speed (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "1.0"))
	float MaxCadenceSeconds = 4.2f;

	/** Reference forward drift speed used to map cadence to +X spatial spacing (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "500.0"))
	float ReferenceForwardSpeedCmPerSec = 3000.0f;

	/** Total generated corridor distance along +X (cm). Default = 120,000 cm (1.2 km = 40s run). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "10000.0"))
	float TotalCorridorDistanceCm = 150000.0f;

	/** Random seed for deterministic level layout reproduction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 GenerationSeed = 42;

	/** Minimum number of obstacles per cluster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MinObstaclesPerCluster = 1;

	/** Maximum number of obstacles per cluster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxObstaclesPerCluster = 4;
};

UCLASS()
class SPACEMENTUMCORE_API ASpaceObstacleSpawner : public AActor
{
	GENERATED_BODY()

public:
	ASpaceObstacleSpawner();

	virtual void BeginPlay() override;

	/** Procedurally generates dense hazard fields along the forward corridor. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Spawner")
	void GenerateHazardCorridor();

	/** Clears and destroys all spawned obstacles (used on run restart). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Spawner")
	void ClearAllObstacles();

	/**
	 * Queries spatial proxies of active obstacles within a given forward lookahead distance window.
	 * Used directly by UDoomDetectionSubsystem for background 13s–40s reachability analysis.
	 *
	 * @param OriginLocation Current world location of the astronaut.
	 * @param MinForwardDistanceCm Minimum forward distance along +X (e.g. 13s * 30 m/s = 39,000 cm).
	 * @param MaxForwardDistanceCm Maximum forward distance along +X (e.g. 40s * 30 m/s = 120,000 cm).
	 * @return Array of lightweight spatial bounding proxies.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Spawner")
	TArray<FObstacleSpatialProxy> GetObstacleProxiesInLookahead(
		const FVector& OriginLocation,
		float MinForwardDistanceCm,
		float MaxForwardDistanceCm) const;

	/** Returns all currently active spawned obstacle actors. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Spawner")
	const TArray<ASpaceObstacleActor*>& GetSpawnedObstacles() const { return SpawnedObstacles; }

protected:
	/** Procedural belt configuration parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Spawner")
	FSpaceObstacleSpawnerConfig SpawnerConfig;

	/** Obstacle actor class to spawn. Defaults to ASpaceObstacleActor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Spawner")
	TSubclassOf<ASpaceObstacleActor> ObstacleClass;

	/** Collection of active spawned obstacle actors. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Spawner")
	TArray<TObjectPtr<ASpaceObstacleActor>> SpawnedObstacles;

private:
	/** Spawns a cluster of obstacles at a given forward distance along the track. */
	void SpawnClusterAtDistance(float ForwardX, FRandomStream& RandStream);
};
