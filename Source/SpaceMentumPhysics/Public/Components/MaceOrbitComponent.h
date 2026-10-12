/**
 * @file MaceOrbitComponent.h
 * @brief Owns the mace's angular state: winds up spin while A/D is held, integrates the orbit angle.
 *
 * @creator_feel
 * "the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction ... moment of stopping is the moment of release."
 *
 * @emotional_intent
 * The wind-up must feel heavy and committed — like hauling a wrecking ball around
 * in zero-g. The player should SEE and FEEL the angle sweep so they can time the
 * release with rhythm, not luck.
 *
 * Responsibilities:
 *  - Accumulate angular velocity omega from FMaceOrbitTuning while a spin direction is held.
 *  - Integrate the orbit angle theta at fixed TG_PrePhysics tick.
 *  - Expose the mace head's world position + tangential velocity for VFX, audio, and transfer.
 *  - On release, dispatch snapshot to UMomentumTransferComponent and arrest angular velocity.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "MaceOrbitComponent.generated.h"

/**
 * Event broadcast when the player releases the spin input button.
 * Dispatches the instantaneous kinematic snapshot needed to calculate the steering kick.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnMaceOrbitReleased,
	float, OrbitAngleRad,
	float, AngularVelocityRadPerSec,
	float, TetherLengthCm,
	float, MaceMassKg
);

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMPHYSICS_API UMaceOrbitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMaceOrbitComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Updates the active spin direction requested by the player controller.
	 * Transition from CounterClockwise/Clockwise to None represents the exact moment of release.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Mace")
	void SetHeldSpinDirection(EMaceSpinDirection Direction);

	/** Applies new tuning parameters (e.g. from an equipped mace data asset). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Mace")
	void SetOrbitTuning(const FMaceOrbitTuning& InTuning);

	/** Read-only access to current deterministic orbit snapshot. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FMaceOrbitState GetOrbitState() const { return OrbitState; }

	/** Read-only access to active tuning. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FMaceOrbitTuning GetOrbitTuning() const { return OrbitTuning; }

	/** Position of mace head in owner local space (YZ plane, X = 0). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FVector GetMaceHeadLocalPosition() const;

	/** Position of mace head in world coordinates. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FVector GetMaceHeadWorldPosition() const;

	/** Tangential linear velocity of mace head in world coordinates (cm/s). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FVector GetTangentialVelocityWorld() const;

	/** Rotational cadence in revolutions per minute. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	float GetCurrentRPM() const;

	/** Linear tangential speed of the mace head (cm/s). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	float GetCurrentTangentialSpeedCmPerSec() const;

	/** Resets orbit angle and velocity to initial rest state. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Mace")
	void ResetOrbit(float InitialAngleRad = 0.0f);

	/** Gathers full real-time telemetry snapshot for debug HUD, replays, or AI agents. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Mace")
	FMaceTelemetry GetTelemetry() const;

	/** Fired on the exact frame the player releases the spin button. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Mace")
	FOnMaceOrbitReleased OnMaceOrbitReleased;

protected:
	/** Feel knobs for the wind-up. Overridden at runtime by the equipped mace definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	FMaceOrbitTuning OrbitTuning;

	/** Live orbit state — recorded for replays and ghost runs. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	FMaceOrbitState OrbitState;
};
