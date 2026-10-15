/**
 * @file MomentumTransferComponent.h
 * @brief Translates mace centrifugal deceleration into astronaut steering impulse vector.
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
 * Responsibilities:
 *  - On spin release, compute v_t = omega x r from mace snapshot.
 *  - Convert to lateral impulse J = (+/-) alpha * m_m * v_t per EMomentumKickModel.
 *  - Apply J / m_p as an instantaneous change to lateral (Y, Z) velocity — never to forward X.
 *  - Exponentially damp lateral velocity afterwards so turns can be chained.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "MomentumTransferComponent.generated.h"

/**
 * Event broadcast immediately after a momentum kick is applied.
 * Feeds camera whiplash shake, audio bass drops, and thruster burst VFX.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMomentumKickFired,
	FVector, ImpartedImpulse,
	FVector, ResultingLateralVelocity,
	EMomentumKickModel, KickModel
);

/**
 * Event broadcast when the mace head physically impacts an obstacle,
 * transferring external collision momentum to the entire system Center of Mass.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnExternalStrikeLanded,
	const FExternalStrikeImpulse&, StrikeImpulse,
	AActor*, HitObstacle
);

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMPHYSICS_API UMomentumTransferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMomentumTransferComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Executes the instantaneous momentum transfer calculation upon mace release.
	 *
	 * @param ReleaseAngleRad           Mace orbit angle at moment of release.
	 * @param ReleaseAngularVelocityRadPerSec Signed angular velocity at moment of release.
	 * @param TetherLengthCm          Tether radius (cm).
	 * @param MaceMassKg              Mace head mass (kg).
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	void ExecuteMomentumTransfer(
		float ReleaseAngleRad,
		float ReleaseAngularVelocityRadPerSec,
		float TetherLengthCm,
		float MaceMassKg);

	/** Sets new transfer tuning parameters (e.g. from data asset). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	void SetTransferTuning(const FMomentumTransferTuning& InTuning);

	/** Flips between ReactionRecoil (GDD default) and CarryAlong (yank) for rapid feel testing. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	void ToggleKickModel();

	/** Sets the active kick model directly. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	void SetKickModel(EMomentumKickModel NewModel);

	/** Current lateral velocity (cm/s) in astronaut's local YZ plane. X is always 0. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	FVector GetLateralVelocity() const { return LateralVelocity; }

	/** Current lateral velocity transformed into world space (cm/s). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	FVector GetLateralVelocityWorld() const;

	/** Read-only access to the active transfer tuning. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	FMomentumTransferTuning GetTransferTuning() const { return TransferTuning; }

	/** Active kick model being evaluated. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	EMomentumKickModel GetKickModel() const { return TransferTuning.KickModel; }

	/**
	 * Executes an external strike momentum transfer when the mace head hits an obstacle.
	 * Shifts the system Center of Mass velocity vector permanently.
	 *
	 * @return True if a valid collision impulse was calculated and applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	bool ExecuteExternalStrike(
		const FVector& MaceVelocityWorld,
		const FVector& ObstacleVelocityWorld,
		const FVector& HitLocationWorld,
		const FVector& HitNormalWorld,
		float MaceMassKg,
		float ObstacleMassKg,
		float Restitution,
		bool bIsAnchored,
		AActor* HitObstacle);

	/** World velocity of the system Center of Mass (cm/s). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Transfer")
	FVector GetSystemCoMVelocity() const { return SystemCoMVelocity; }

	/** Overrides the system Center of Mass velocity vector. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	void SetSystemCoMVelocity(const FVector& NewCoMVelocity);

	/** Resets lateral dodging velocity to zero. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Transfer")
	void ResetLateralVelocity();

	/** Fired on the exact frame an internal momentum kick is applied. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Transfer")
	FOnMomentumKickFired OnMomentumKickFired;

	/** Fired on the exact frame an external mace strike impacts an obstacle. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Transfer")
	FOnExternalStrikeLanded OnExternalStrikeLanded;

protected:
	/** Feel knobs for the kick. Overridden at runtime by the equipped mace definition. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Transfer")
	FMomentumTransferTuning TransferTuning;

	/** Lateral velocity produced by internal kicks, decaying via LateralDampingPerSec. Local YZ only. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Transfer")
	FVector LateralVelocity = FVector::ZeroVector;

	/** Permanent system Center of Mass velocity in world space (cm/s). Shifted by external strikes. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Transfer")
	FVector SystemCoMVelocity = FVector::ZeroVector;
};
