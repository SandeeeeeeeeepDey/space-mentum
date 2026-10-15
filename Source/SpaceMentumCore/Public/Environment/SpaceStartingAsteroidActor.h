/**
 * @file SpaceStartingAsteroidActor.h
 * @brief Celestial starting asteroid at the threshold of the sector (X = 0).
 *
 * @creator_feel
 * "the gane starts with me pushing on an asteroid towards the tragactory i want.
 *  al arrow to the direction, a map and speed meter is also needed and the push power too"
 *
 * @emotional_intent
 * Agency & Physical Grounding. Every run begins with muscular intention. The player
 * establishes their initial line by pushing against a tangible celestial rock, giving
 * immediate tactile meaning to Newtonian reaction force before plunging into the gauntlet.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SpaceMentumCoreTypes.h"
#include "SpaceStartingAsteroidActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class SPACEMENTUMCORE_API ASpaceStartingAsteroidActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceStartingAsteroidActor();

	/** Gets the world transform where the astronaut plants the mechanical appendage. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Launch")
	FTransform GetAstronautAnchorTransform() const;

	/** Gets the world position of the astronaut planting point. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Launch")
	FVector GetAstronautAnchorLocation() const;

	/** Radius of the starting asteroid body (cm). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Launch")
	float GetAsteroidRadius() const { return AsteroidRadius; }

	/** Computes the resulting launch impulse and departure velocity from push charge. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Launch")
	FVector ComputeDepartureVelocity(
		float PushChargeRatio,
		const FVector& AimDirection,
		const FLaunchPushOffParameters& LaunchParams) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootScene;

	/** High-mass celestial asteroid visual and physical mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AsteroidMesh;

	/** Spatial socket/transform where the astronaut is planted before push-off. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> AnchorSocket;

	/** Radius of the asteroid body (cm). Default: 320 cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Launch")
	float AsteroidRadius = 320.0f;
};
