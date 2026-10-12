/**
 * @file MaceOrbitComponent.h
 * @brief Owns the mace's angular state: winds up spin while A/D is held, integrates the orbit angle.
 *
 * @creator_feel
 * "the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction"
 *
 * @emotional_intent
 * The wind-up must feel heavy and committed — like hauling a wrecking ball around
 * in zero-g. The player should SEE and FEEL the angle sweep so they can time the
 * release with rhythm, not luck.
 *
 * Responsibilities (Step 2 implementation):
 *  - Accumulate angular velocity omega from FMaceOrbitTuning while a spin direction is held.
 *  - Integrate the orbit angle theta at the fixed physics substep (determinism, GDD §2.2).
 *  - Expose the mace head's world position + tangential velocity for VFX, audio, and transfer.
 *  - On release, hand the snapshot to UMomentumTransferComponent and zero the spin.
 *
 * Deliberately NOT responsible for: applying forces to the astronaut (that is
 * UMomentumTransferComponent) or reading input (that is ASpacePlayerController).
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "MaceOrbitComponent.generated.h"

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMPHYSICS_API UMaceOrbitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMaceOrbitComponent();

	/** Read-only access to the current deterministic orbit snapshot. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FMaceOrbitState GetOrbitState() const { return OrbitState; }

	/** Read-only access to the active tuning (normally sourced from a UMaceDefinitionDataAsset). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FMaceOrbitTuning GetOrbitTuning() const { return OrbitTuning; }

protected:
	/** Feel knobs for the wind-up. Overridden at runtime by the equipped mace definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	FMaceOrbitTuning OrbitTuning;

	/** Live orbit state — recorded for replays and ghost runs. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	FMaceOrbitState OrbitState;
};
