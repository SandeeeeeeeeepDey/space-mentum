/**
 * @file SpaceObstacleSpawner.cpp
 * @brief Implementation of procedural dense hazard spawner.
 *
 * @emotional_intent High APM, continuous spatial choices, dense hazard gauntlets.
 */

#include "Environment/SpaceObstacleSpawner.h"
#include "Engine/World.h"

ASpaceObstacleSpawner::ASpaceObstacleSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	ObstacleClass = ASpaceObstacleActor::StaticClass();
}

void ASpaceObstacleSpawner::BeginPlay()
{
	Super::BeginPlay();
	GenerateHazardCorridor();
}

void ASpaceObstacleSpawner::GenerateHazardCorridor()
{
	ClearAllObstacles();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FRandomStream RandStream(SpawnerConfig.GenerationSeed);

	// Start spawning 40 meters ahead (4,000 cm = ~1.3s runway at 30 m/s)
	float CurrentX = 4000.0f;

	while (CurrentX < SpawnerConfig.TotalCorridorDistanceCm)
	{
		// Tight cadence spacing: 1.8s to 4.2s (54m to 126m at 30 m/s)
		const float CadenceSeconds = RandStream.FRandRange(
			SpawnerConfig.MinCadenceSeconds,
			SpawnerConfig.MaxCadenceSeconds
		);
		CurrentX += CadenceSeconds * SpawnerConfig.ReferenceForwardSpeedCmPerSec;

		SpawnClusterAtDistance(CurrentX, RandStream);
	}

	UE_LOG(LogTemp, Log, TEXT("[SpaceObstacleSpawner] Generated %d obstacles across %0.1f meters of corridor."),
		SpawnedObstacles.Num(), SpawnerConfig.TotalCorridorDistanceCm / 100.0f);
}

void ASpaceObstacleSpawner::ClearAllObstacles()
{
	for (ASpaceObstacleActor* Obstacle : SpawnedObstacles)
	{
		if (IsValid(Obstacle))
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Empty();
}

void ASpaceObstacleSpawner::SpawnClusterAtDistance(float ForwardX, FRandomStream& RandStream)
{
	UWorld* World = GetWorld();
	if (!World || !ObstacleClass)
	{
		return;
	}

	const int32 ClusterSize = RandStream.RandRange(
		SpawnerConfig.MinObstaclesPerCluster,
		SpawnerConfig.MaxObstaclesPerCluster
	);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < ClusterSize; ++i)
	{
		// Distribute across the cylindrical cross-section (YZ plane)
		const float AngleRad = RandStream.FRandRange(0.0f, 2.0f * PI);
		const float RadialDist = RandStream.FRandRange(
			80.0f,
			SpawnerConfig.CorridorRadiusCm * 0.85f
		);

		const float ClusterJitterX = RandStream.FRandRange(-300.0f, 300.0f);
		const FVector SpawnLocation(
			ForwardX + ClusterJitterX,
			FMath::Cos(AngleRad) * RadialDist,
			FMath::Sin(AngleRad) * RadialDist
		);

		const FRotator SpawnRotation(
			RandStream.FRandRange(-180.0f, 180.0f),
			RandStream.FRandRange(-180.0f, 180.0f),
			RandStream.FRandRange(-180.0f, 180.0f)
		);

		ASpaceObstacleActor* NewObstacle = World->SpawnActor<ASpaceObstacleActor>(
			ObstacleClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (NewObstacle)
		{
			SpawnedObstacles.Add(NewObstacle);
		}
	}
}

TArray<FObstacleSpatialProxy> ASpaceObstacleSpawner::GetObstacleProxiesInLookahead(
	const FVector& OriginLocation,
	float MinForwardDistanceCm,
	float MaxForwardDistanceCm) const
{
	TArray<FObstacleSpatialProxy> Proxies;
	Proxies.Reserve(SpawnedObstacles.Num());

	const float MinX = OriginLocation.X + MinForwardDistanceCm;
	const float MaxX = OriginLocation.X + MaxForwardDistanceCm;

	for (const ASpaceObstacleActor* Obstacle : SpawnedObstacles)
	{
		if (!IsValid(Obstacle) || Obstacle->IsDestroyed())
		{
			continue;
		}

		const FVector ObsLoc = Obstacle->GetActorLocation();
		// Spatial culling along forward corridor axis
		if (ObsLoc.X >= MinX && ObsLoc.X <= MaxX)
		{
			Proxies.Add(Obstacle->GetSpatialProxy());
		}
	}

	return Proxies;
}
