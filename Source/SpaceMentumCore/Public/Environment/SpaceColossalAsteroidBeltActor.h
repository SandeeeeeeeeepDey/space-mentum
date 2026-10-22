/**
 * @file SpaceColossalAsteroidBeltActor.h
 * @brief Sprawling celestial asteroid belt extending far outside the player's view,
 *        generating dynamic colossal asteroids approaching from variable forward-facing angles.
 *
 * @creator_feel
 * "the game needs a screen where the asteroids are much bigger too.
 *  the belt of the asteroid needs to be much bigger and outside the view.
 *  the asteroids should be comming from any angle which has atleast one component from the forward."
 *
 * @emotional_intent
 * Cosmic majesty, claustrophobic expanse, and relentless kinetic hazard flow.
 * The asteroid belt is not a narrow hallway; it is a gargantuan planetary ring system
 * stretching thousands of meters across the void. Towering celestial monoliths drift in the
 * deep periphery while aggressive ballistic asteroids hurtle forward from oblique trajectories.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Environment/SpaceColossalAsteroidActor.h"
#include "SpaceColossalAsteroidBeltActor.generated.h"

/**
 * Tuning parameters for procedural colossal belt generation and dynamic incoming spawns.
 */
USTRUCT(BlueprintType)
struct FColossalAsteroidBeltConfig
{
	GENERATED_BODY()

	/** Outer belt lateral radius envelope (cm). Default = 35,000 cm (350 meters). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "10000.0"))
	float BeltOuterRadiusCm = 35000.0f;

	/** Inner belt clear clearance radius around player centerline (cm). Default = 1500 cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "800.0"))
	float BeltInnerClearanceCm = 1500.0f;

	/** Number of static deep-background monolithic perimeter asteroids outside the view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "10"))
	int32 BackgroundPerimeterAsteroidCount = 45;

	/** Minimum radius of incoming colossal asteroids (cm). Default = 450 cm (9m diameter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "300.0"))
	float MinIncomingAsteroidRadiusCm = 450.0f;

	/** Maximum radius of incoming colossal asteroids (cm). Default = 1800 cm (36m diameter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "600.0"))
	float MaxIncomingAsteroidRadiusCm = 1800.0f;

	/** Cadence interval between oncoming asteroid encounters (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "1.0"))
	float SpawnIntervalSeconds = 3.2f;

	/** Base approach speed along -X (cm/s). Default = 1600 cm/s (57.6 km/h). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "400.0"))
	float BaseIncomingApproachSpeedCms = 1600.0f;

	/** Maximum lateral velocity component (Y, Z) for incoming trajectory angles (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "100.0"))
	float MaxLateralVelocityCms = 900.0f;

	/** Forward lookahead distance along +X where incoming asteroids materialize (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Belt", meta = (ClampMin = "5000.0"))
	float ForwardSpawnDistanceCm = 14000.0f;
};

UCLASS()
class SPACEMENTUMCORE_API ASpaceColossalAsteroidBeltActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceColossalAsteroidBeltActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Procedurally generates the vast background belt outside the view. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Belt")
	void GenerateColossalBackgroundBelt();

	/** Clears all background and dynamic active asteroids. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Belt")
	void ClearAllBeltAsteroids();

	/** Spawns a single dynamic colossal asteroid with trajectory containing a forward component. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Belt")
	ASpaceColossalAsteroidActor* SpawnIncomingColossalAsteroid(const FVector& PlayerPositionWorld);

	/** Queries all active candidate asteroids for targeting acquisition. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Belt")
	const TArray<ASpaceColossalAsteroidActor*>& GetActiveDynamicAsteroids() const { return ActiveDynamicAsteroids; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Belt")
	FColossalAsteroidBeltConfig BeltConfig;

	/** Subclass of asteroid to instantiate. Defaults to ASpaceColossalAsteroidActor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Belt")
	TSubclassOf<ASpaceColossalAsteroidActor> AsteroidClass;

	/** Collection of active dynamic oncoming asteroids. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Belt")
	TArray<TObjectPtr<ASpaceColossalAsteroidActor>> ActiveDynamicAsteroids;

	/** Static deep-background perimeter monoliths outside view. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Belt")
	TArray<TObjectPtr<ASpaceColossalAsteroidActor>> BackgroundPerimeterMonoliths;

private:
	float SpawnTimerAccumulator = 0.0f;

	/** Generates a random approach velocity vector guaranteed to possess a forward component (Vx < 0). */
	FVector GenerateIncomingTrajectoryVector(FRandomStream& RandStream) const;
};
