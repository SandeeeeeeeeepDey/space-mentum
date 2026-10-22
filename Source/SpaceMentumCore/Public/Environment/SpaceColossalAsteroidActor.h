/**
 * @file SpaceColossalAsteroidActor.h
 * @brief Monumental / colossal celestial asteroid actor tumbling through deep space.
 *
 * @creator_feel
 * "the game needs a screen where the asteroids are much bigger too.
 *  the belt of the asteroid needs to be much bigger and outside the view.
 *  the asteroids should be comming from any angle which has atleast one component from the forward."
 *
 * @emotional_intent
 * Intimidation and sublime scale. Unlike smaller slalom hazards, these asteroids are towering
 * monolithic space boulders (radii from 400cm to 3500cm+). They approach from variable angles
 * bearing down on the player with high inertia, demanding intentional kinetic manipulation.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SpaceMentumKineticManipulationTypes.h"
#include "SpaceColossalAsteroidActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

/** Delegate fired when this colossal asteroid receives a kinetic push impulse. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnColossalAsteroidPushed,
	class ASpaceColossalAsteroidActor*, Asteroid,
	const FVector&, TransferredImpulseNs,
	const FVector&, ResultingDeltaVelocityCms
);

UCLASS()
class SPACEMENTUMCORE_API ASpaceColossalAsteroidActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceColossalAsteroidActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/**
	 * Initializes the colossal asteroid with scale, velocity, mass, and tumble.
	 *
	 * @param InRadiusCm Bounding radius of the asteroid (cm).
	 * @param InVelocityWorld World linear velocity vector (cm/s).
	 * @param InTumbleRateRot Rotational tumble rates (deg/s).
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Asteroid")
	void InitializeColossalAsteroid(
		float InRadiusCm,
		const FVector& InVelocityWorld,
		const FRotator& InTumbleRateRot);

	/**
	 * Receives a kinetic push impulse from the astronaut's mechanical manipulator.
	 *
	 * @param ImpulseVectorNs Linear impulse applied to the asteroid (N*s).
	 * @return True if impulse was successfully absorbed.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Asteroid")
	bool ReceiveKineticPushImpulse(const FVector& ImpulseVectorNs);

	/** Highlights this asteroid with target lock holographic shading. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Asteroid")
	void SetTargetLockHighlight(bool bLocked, bool bCriticalProximity);

	// --- Getters ---

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Asteroid")
	float GetAsteroidRadiusCm() const { return AsteroidRadiusCm; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Asteroid")
	float GetAsteroidMassKg() const { return AsteroidMassKg; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Asteroid")
	FVector GetAsteroidVelocity() const { return LinearVelocityWorld; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Asteroid")
	bool IsTargetLocked() const { return bIsCurrentlyLocked; }

	// --- Public Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Asteroid")
	FOnColossalAsteroidPushed OnColossalAsteroidPushed;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** High-fidelity cratered celestial mesh with Continuous Collision Detection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AsteroidMesh;

	/** Primary collision volume. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	/** Bounding collision radius (cm). Default = 650 cm (6.5m radius = 13m across). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Asteroid", meta = (ClampMin = "300.0"))
	float AsteroidRadiusCm = 650.0f;

	/** Physical mass of the celestial boulder (kg). Default = 45,000 kg. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Asteroid", meta = (ClampMin = "1000.0"))
	float AsteroidMassKg = 45000.0f;

	/** Instantaneous linear velocity in world space (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Asteroid")
	FVector LinearVelocityWorld = FVector::ZeroVector;

	/** Rotational tumbling rate (degrees/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Asteroid")
	FRotator TumbleRateDegPerSec = FRotator::ZeroRotator;

	/** Whether this asteroid is currently target-locked by the player. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Asteroid")
	bool bIsCurrentlyLocked = false;
};
