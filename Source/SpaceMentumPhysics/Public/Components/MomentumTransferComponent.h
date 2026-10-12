/**
 * @file MomentumTransferComponent.h
 * @brief Translates mace centrifugal deceleration into astronaut steering vector.
 *
 * @creator_feel
 * "by spinning and stopping the spin in a pertiicular direction, one can stir their direction..."
 * "moment of stopping is the moment of release."
 *
 * @emotional_intent
 * The player should feel a sudden, satisfying "whiplash" kick. It must feel like grabbing
 * onto a physical pivot in zero-g, transferring raw rotational violence into clean evasion.
 * Avoid mushy linear interpolation; prioritize a crisp, punchy kinetic impulse.
 *
 * Responsibilities (Step 2 implementation):
 *  - On spin release, compute v_t = omega x r from the UMaceOrbitComponent snapshot.
 *  - Convert to lateral impulse J = (+/-) alpha * m_m * v_t per EMomentumKickModel.
 *  - Apply J / m_p as an instantaneous change to lateral (Y, Z) velocity — never to forward X.
 *  - Exponentially damp lateral velocity afterwards so turns can be chained.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "MomentumTransferComponent.generated.h"

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMPHYSICS_API UMomentumTransferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMomentumTransferComponent();

	/** Read-only access to the active transfer tuning. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	FMomentumTransferTuning GetTransferTuning() const { return TransferTuning; }

	/** Current lateral velocity (cm/s) in the astronaut's local YZ plane. X is always 0. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	FVector GetLateralVelocity() const { return LateralVelocity; }

protected:
	/** Feel knobs for the kick. Overridden at runtime by the equipped mace definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Transfer")
	FMomentumTransferTuning TransferTuning;

	/** Lateral velocity produced by kicks, decaying via LateralDampingPerSec. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Transfer")
	FVector LateralVelocity = FVector::ZeroVector;
};
