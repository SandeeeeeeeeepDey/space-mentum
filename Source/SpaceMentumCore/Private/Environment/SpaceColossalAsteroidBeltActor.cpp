/**
 * @file SpaceColossalAsteroidBeltActor.cpp
 * @brief Implementation of sprawling celestial asteroid belt generator and incoming hazard spawner.
 *
 * @creator_feel
 * "the game needs a screen where the asteroids are much bigger too.
 *  the belt of the asteroid needs to be much bigger and outside the view.
 *  the asteroids should be comming from any angle which has atleast one component from the forward."
 *
 * @emotional_intent
 * Delivers awe-inspiring visual scale and intense kinetic anticipation.
 * Sprawls vast monolithic asteroids far outside the camera frustum while launching
 * massive tumbling boulders on ballistic trajectories that intersect the corridor.
 */

#include "Environment/SpaceColossalAsteroidBeltActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ASpaceColossalAsteroidBeltActor::ASpaceColossalAsteroidBeltActor()
{
	PrimaryActorTick.bCanEverTick = true;
	AsteroidClass = ASpaceColossalAsteroidActor::StaticClass();
}

void ASpaceColossalAsteroidBeltActor::BeginPlay()
{
	Super::BeginPlay();
	GenerateColossalBackgroundBelt();
}

void ASpaceColossalAsteroidBeltActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// 1. Dynamic Spawner Timer Accumulator
	SpawnTimerAccumulator += DeltaTime;
	if (SpawnTimerAccumulator >= BeltConfig.SpawnIntervalSeconds)
	{
		SpawnTimerAccumulator = 0.0f;

		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		const FVector PlayerPos = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

		SpawnIncomingColossalAsteroid(PlayerPos);
	}

	// 2. Prune asteroids that have drifted far past the player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	const float PlayerX = PlayerPawn ? PlayerPawn->GetActorLocation().X : 0.0f;

	for (int32 i = ActiveDynamicAsteroids.Num() - 1; i >= 0; --i)
	{
		ASpaceColossalAsteroidActor* Asteroid = ActiveDynamicAsteroids[i];
		if (!IsValid(Asteroid))
		{
			ActiveDynamicAsteroids.RemoveAt(i);
			continue;
		}

		if (Asteroid->GetActorLocation().X < (PlayerX - 4500.0f))
		{
			Asteroid->Destroy();
			ActiveDynamicAsteroids.RemoveAt(i);
		}
	}
}

void ASpaceColossalAsteroidBeltActor::GenerateColossalBackgroundBelt()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ClearAllBeltAsteroids();

	FRandomStream RandStream(1337);

	// Generate colossal background monoliths located far outside the primary view
	for (int32 i = 0; i < BeltConfig.BackgroundPerimeterAsteroidCount; ++i)
	{
		const float AngleRad = RandStream.FRandRange(0.0f, 2.0f * PI);
		const float RadialDistCm = RandStream.FRandRange(BeltConfig.BeltInnerClearanceCm + 3500.0f, BeltConfig.BeltOuterRadiusCm);
		const float ForwardX = RandStream.FRandRange(-5000.0f, 160000.0f);

		const FVector SpawnLocation(
			ForwardX,
			FMath::Cos(AngleRad) * RadialDistCm,
			FMath::Sin(AngleRad) * RadialDistCm
		);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASpaceColossalAsteroidActor* Monolith = World->SpawnActor<ASpaceColossalAsteroidActor>(
			AsteroidClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Monolith)
		{
			// Background monoliths are colossal (12m to 50m across) with slow ambient drift
			const float MonolithRadiusCm = RandStream.FRandRange(1200.0f, 3500.0f);
			const FVector DriftVel(
				RandStream.FRandRange(-80.0f, 80.0f),
				RandStream.FRandRange(-60.0f, 60.0f),
				RandStream.FRandRange(-60.0f, 60.0f)
			);
			const FRotator Tumble(
				RandStream.FRandRange(-3.0f, 3.0f),
				RandStream.FRandRange(-4.0f, 4.0f),
				RandStream.FRandRange(-2.0f, 2.0f)
			);

			Monolith->InitializeColossalAsteroid(MonolithRadiusCm, DriftVel, Tumble);
			BackgroundPerimeterMonoliths.Add(Monolith);
		}
	}
}

void ASpaceColossalAsteroidBeltActor::ClearAllBeltAsteroids()
{
	for (ASpaceColossalAsteroidActor* Asteroid : ActiveDynamicAsteroids)
	{
		if (IsValid(Asteroid))
		{
			Asteroid->Destroy();
		}
	}
	ActiveDynamicAsteroids.Empty();

	for (ASpaceColossalAsteroidActor* Monolith : BackgroundPerimeterMonoliths)
	{
		if (IsValid(Monolith))
		{
			Monolith->Destroy();
		}
	}
	BackgroundPerimeterMonoliths.Empty();
}

ASpaceColossalAsteroidActor* ASpaceColossalAsteroidBeltActor::SpawnIncomingColossalAsteroid(const FVector& PlayerPositionWorld)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FRandomStream RandStream(FPlatformTime::Cycles());

	// Spawn ahead of the player along +X
	const float SpawnX = PlayerPositionWorld.X + BeltConfig.ForwardSpawnDistanceCm + RandStream.FRandRange(-1200.0f, 1500.0f);

	// Radial offset from corridor centerline
	const float AngleRad = RandStream.FRandRange(0.0f, 2.0f * PI);
	const float RadialDistCm = RandStream.FRandRange(250.0f, 2400.0f);

	const FVector SpawnLocation(
		SpawnX,
		PlayerPositionWorld.Y + FMath::Cos(AngleRad) * RadialDistCm,
		PlayerPositionWorld.Z + FMath::Sin(AngleRad) * RadialDistCm
	);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASpaceColossalAsteroidActor* NewAsteroid = World->SpawnActor<ASpaceColossalAsteroidActor>(
		AsteroidClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewAsteroid)
	{
		const float RadiusCm = RandStream.FRandRange(
			BeltConfig.MinIncomingAsteroidRadiusCm,
			BeltConfig.MaxIncomingAsteroidRadiusCm
		);

		const FVector TrajectoryVelocity = GenerateIncomingTrajectoryVector(RandStream);
		const FRotator Tumble(
			RandStream.FRandRange(-18.0f, 18.0f),
			RandStream.FRandRange(-24.0f, 24.0f),
			RandStream.FRandRange(-15.0f, 15.0f)
		);

		NewAsteroid->InitializeColossalAsteroid(RadiusCm, TrajectoryVelocity, Tumble);
		ActiveDynamicAsteroids.Add(NewAsteroid);
	}

	return NewAsteroid;
}

FVector ASpaceColossalAsteroidBeltActor::GenerateIncomingTrajectoryVector(FRandomStream& RandStream) const
{
	// MANDATE: "the asteroids should be comming from any angle which has atleast one component from the forward."
	// Approach velocity must have Vx < 0 (heading toward smaller X / toward the player from ahead).
	// Lateral components (Vy, Vz) can be from ANY radial angle.

	const float ForwardComponentSpeed = RandStream.FRandRange(
		BeltConfig.BaseIncomingApproachSpeedCms * 0.85f,
		BeltConfig.BaseIncomingApproachSpeedCms * 1.55f
	);

	// Negative Vx means approaching from the forward corridor toward the player
	const float Vx = -ForwardComponentSpeed;

	// Oblique approach angle in the lateral YZ plane
	const float LateralAngleRad = RandStream.FRandRange(0.0f, 2.0f * PI);
	const float LateralMagnitude = RandStream.FRandRange(0.0f, BeltConfig.MaxLateralVelocityCms);

	const float Vy = FMath::Cos(LateralAngleRad) * LateralMagnitude;
	const float Vz = FMath::Sin(LateralAngleRad) * LateralMagnitude;

	return FVector(Vx, Vy, Vz);
}
