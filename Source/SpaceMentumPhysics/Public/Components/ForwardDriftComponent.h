/**
 * @file ForwardDriftComponent.h
 * @brief Maintains the astronaut's uniform, never-ending forward velocity along +X.
 *
 * @creator_feel
 * "a space person zooming in the forward direction in a uniform velocity"
 *
 * @emotional_intent
 * Relentless. The player can never slow down, never stop, never reverse. That
 * inevitability is what makes every mace release matter — "thrilling vulnerability
 * mixed with heavy kinetic power." Speed boosts may come later as abilities, but
 * the base drift is sacred and uniform.
 *
 * Responsibilities (Step 2 implementation):
 *  - Enforce forward speed = FForwardDriftTuning::ForwardSpeedCmPerSec every physics tick.
 *  - Compose final velocity = forward drift (X) + lateral velocity from UMomentumTransferComponent (Y, Z).
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "ForwardDriftComponent.generated.h"

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMPHYSICS_API UForwardDriftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UForwardDriftComponent();

	/** Read-only access to the drift tuning. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Drift")
	FForwardDriftTuning GetDriftTuning() const { return DriftTuning; }

protected:
	/** Feel knob for the relentless forward speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Drift")
	FForwardDriftTuning DriftTuning;
};
