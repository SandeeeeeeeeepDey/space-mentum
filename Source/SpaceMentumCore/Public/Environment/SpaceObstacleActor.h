/**
 * @file SpaceObstacleActor.h
 * @brief Physical obstacle actor in deep space with Continuous Collision Detection (CCD).
 *
 * @creator_feel
 * "abandon realistic space spacing. The levels need to be incredibly dense navigational hazards...
 *  asteroid belt also can be a way."
 * "The ONLY way to permanently change the trajectory of the ship's Center of Mass is to
 *  physically strike the appendage against an environmental obstacle."
 *
 * @emotional_intent
 * Obstacles are menacing, massive cosmic hazards. Striking them with the heavy spinning mace
 * produces a violent clang and pivots the player's momentum. Crashing directly into them
 * with the fragile astronaut body is fatal.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "Solvers/DoomTrajectoryEvaluator.h"
#include "SpaceObstacleActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USphereComponent;
class UObstacleArchetypeDataAsset;
class ASpaceAstronautCharacter;

/** Delegate fired when the mace head impacts the obstacle. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnObstacleMaceStruck,
	class ASpaceObstacleActor*, Obstacle,
	const FExternalStrikeImpulse&, StrikeImpulse
);

/** Delegate fired when the astronaut body collides with the obstacle. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnObstacleAstronautImpact,
	class ASpaceObstacleActor*, Obstacle,
	bool, bIsLethal
);

/** Delegate fired when a destructible obstacle is shattered. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnObstacleDestroyed,
	class ASpaceObstacleActor*, Obstacle
);

UCLASS()
class SPACEMENTUMCORE_API ASpaceObstacleActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceObstacleActor();

	virtual void BeginPlay() override;

	/** Configures obstacle properties from designer-authored archetype data asset. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Obstacle")
	void ApplyArchetypeData(UObstacleArchetypeDataAsset* InArchetypeData);

	/** Handles physical collision with the mace head, transferring external strike momentum. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Obstacle")
	bool ProcessMaceStrike(
		const FVector& MaceVelocityWorld,
		const FVector& HitLocationWorld,
		const FVector& HitNormalWorld,
		float MaceMassKg,
		float TotalSystemMassKg,
		ASpaceAstronautCharacter* StrikingAstronaut);

	/** Handles physical collision with the astronaut body, testing for lethal kinetic impact. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Obstacle")
	bool ProcessAstronautBodyCollision(
		const FVector& RelativeVelocityWorld,
		ASpaceAstronautCharacter* ImpactedAstronaut);

	/** Returns a lightweight spatial proxy used by background doom lookahead evaluators. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Obstacle")
	FObstacleSpatialProxy GetSpatialProxy() const;

	// --- Getters & Status ---

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Obstacle")
	bool IsDestroyed() const { return bIsDestroyed; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Obstacle")
	bool IsAnchored() const { return bIsAnchored; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Obstacle")
	float GetCollisionRadiusCm() const { return CollisionRadiusCm; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Obstacle")
	float GetObstacleMassKg() const { return ObstacleMassKg; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Obstacle")
	float GetRestitution() const { return Restitution; }

	// --- Public Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Obstacle")
	FOnObstacleMaceStruck OnObstacleMaceStruck;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Obstacle")
	FOnObstacleAstronautImpact OnObstacleAstronautImpact;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Obstacle")
	FOnObstacleDestroyed OnObstacleDestroyed;

protected:
	/** Scene root. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Visual static mesh with CCD enabled. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ObstacleMesh;

	/** Bounding sphere collision component with CCD. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** Archetype data asset definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Obstacle")
	TObjectPtr<UObstacleArchetypeDataAsset> ArchetypeData;

	/** Bounding radius of the obstacle (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Obstacle", meta = (ClampMin = "10.0"))
	float CollisionRadiusCm = 160.0f;

	/** Physical mass of the obstacle (kg). Massive asteroids default to 5000 kg. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Obstacle", meta = (ClampMin = "1.0"))
	float ObstacleMassKg = 5000.0f;

	/** Coefficient of restitution for collisions [0, 1]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Obstacle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Restitution = 0.70f;

	/** If true, treated as anchored / infinite-inertia celestial body (e.g. asteroid bulkhead). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Obstacle")
	bool bIsAnchored = true;

	/** Whether this obstacle is shatterable by mace impact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Obstacle")
	bool bIsMaceDestructible = false;

	/** Minimum impact speed (cm/s) required to shatter a destructible obstacle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Obstacle", meta = (ClampMin = "1.0"))
	float ShatterSpeedThresholdCmPerSec = 1500.0f;

	/** Whether the obstacle has already been shattered / removed. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Obstacle")
	bool bIsDestroyed = false;

private:
	/** Internal trigger on lethal body collision. */
	void HandleFatalAstronautCrash(ASpaceAstronautCharacter* Astronaut);

	/** Internal trigger on mace destruction. */
	void HandleObstacleShattered();
};
