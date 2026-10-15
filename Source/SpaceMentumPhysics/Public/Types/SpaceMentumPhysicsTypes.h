/**
 * @file SpaceMentumPhysicsTypes.h
 * @brief Shared tuning structs, state structs, and enums for the momentum-steering physics.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction to move away from obsticles."
 *
 * @emotional_intent
 * Every number in this file is a FEEL knob, not just arithmetic. Designers tune these
 * through UMaceDefinitionDataAsset (SpaceMentumAbilities) without touching C++.
 * When you change a default here, write down in the comment WHY it feels better.
 *
 * Coordinate convention (see Docs/TECHNICAL_SPECIFICATIONS.md §3):
 *   +X  = forward drift direction (the GDD's "v_z")
 *   +Y  = right, +Z = up, as seen from the chase camera behind the astronaut.
 *   The mace orbits in the astronaut's local YZ plane (full 360° cylindrical dodging).
 *   Orbit angle 0 rad = +Y (right); positive angle = counter-clockwise seen from behind.
 */

#pragma once

#include "CoreMinimal.h"
#include "SpaceMentumPhysicsTypes.generated.h"

/**
 * Which way the player is winding the mace.
 *
 * @creator_feel "for pc A and D looks good. only two buttons for now to spin the mace"
 * A -> CounterClockwise, D -> Clockwise (as seen from the chase camera).
 */
UENUM(BlueprintType)
enum class EMaceSpinDirection : uint8
{
	/** No spin button held — "moment of stopping is the moment of release". */
	None             UMETA(DisplayName = "None (Released)"),
	/** Held A: angle increases (right -> up -> left -> down). */
	CounterClockwise UMETA(DisplayName = "Counter-Clockwise (A)"),
	/** Held D: angle decreases (right -> down -> left -> up). */
	Clockwise        UMETA(DisplayName = "Clockwise (D)"),
};

/**
 * How the stored spin momentum becomes the astronaut's steering kick on release.
 *
 * OPEN FEEL QUESTION (flagged for the Creator in Step 1):
 * GDD §3.1.2 specifies a REACTION impulse J = -alpha * m_mace * v_tangential (kick opposite
 * to the mace's travel). A literal "grab and stop the mace" in zero-g instead carries the
 * astronaut ALONG the mace's travel. Both are physically defensible; the prototype lets us
 * flip between them and pick whichever produces the better "whiplash".
 */
UENUM(BlueprintType)
enum class EMomentumKickModel : uint8
{
	/** GDD §3.1.2 default: astronaut is kicked opposite to the mace's tangential velocity. */
	ReactionRecoil   UMETA(DisplayName = "Reaction Recoil (GDD default)"),
	/** Astronaut is carried along with the mace's tangential velocity, like being yanked by it. */
	CarryAlong       UMETA(DisplayName = "Carry Along (Yank)"),
};

/**
 * Tuning for the spinning mace orbit.
 *
 * @emotional_intent The mace must feel genuinely HEAVY — it should take a beat to wind up,
 * so a perfectly-timed release feels earned rather than free.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FMaceOrbitTuning
{
	GENERATED_BODY()

	/** Tether length r (cm). Longer = bigger, slower-looking arcs and a stronger kick per rad/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Orbit", meta = (ClampMin = "10.0", Units = "Centimeters"))
	float TetherLengthCm = 160.0f;

	/** Mace mass m_m (kg). The "weight" the player feels in the kick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Orbit", meta = (ClampMin = "0.1", Units = "Kilograms"))
	float MaceMassKg = 40.0f;

	/** Angular acceleration while a spin button is held (deg/s²). Lower = heavier wind-up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Orbit", meta = (ClampMin = "1.0"))
	float SpinAccelerationDegPerSec2 = 1440.0f;

	/** Angular speed cap (deg/s). 1080 = 3 revolutions per second — fast, but still readable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Orbit", meta = (ClampMin = "1.0"))
	float MaxAngularSpeedDegPerSec = 1080.0f;
};

/**
 * Tuning for converting stored spin into a lateral steering kick.
 *
 * @emotional_intent "Avoid mushy linear interpolation; prioritize a crisp, punchy kinetic
 * impulse." (GDD §0.2). The kick is instantaneous; only the aftermath is damped.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FMomentumTransferTuning
{
	GENERATED_BODY()

	/** Astronaut mass m_p (kg). Mace/astronaut mass ratio sets how violent the kick feels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Transfer", meta = (ClampMin = "1.0", Units = "Kilograms"))
	float AstronautMassKg = 90.0f;

	/** Impulse efficiency alpha in J = alpha * m_m * v_t. 1.0 = perfectly elastic transfer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Transfer", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ImpulseEfficiencyAlpha = 0.6f;

	/** Kick direction model — see EMomentumKickModel for the open feel question. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Transfer")
	EMomentumKickModel KickModel = EMomentumKickModel::ReactionRecoil;

	/** Releases below this angular speed (deg/s) produce no kick — forgives accidental taps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Transfer", meta = (ClampMin = "0.0"))
	float MinAngularSpeedForKickDegPerSec = 90.0f;

	/** Exponential decay rate (1/s) of lateral velocity, so the player can stabilise and chain turns. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Transfer", meta = (ClampMin = "0.0"))
	float LateralDampingPerSec = 1.5f;

	/** Hard cap on lateral speed (cm/s) so chained kicks never fling the player off-track. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mace|Transfer", meta = (ClampMin = "0.0"))
	float MaxLateralSpeedCmPerSec = 1800.0f;
};

/**
 * Tuning for the relentless forward drift.
 *
 * @creator_feel "zooming in the forward direction in a uniform velocity"
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FForwardDriftTuning
{
	GENERATED_BODY()

	/** Constant forward speed v_forward (cm/s). 3000 cm/s = 108 km/h — fast enough to feel dangerous. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drift", meta = (ClampMin = "0.0"))
	float ForwardSpeedCmPerSec = 3000.0f;
};

/**
 * Deterministic snapshot of the mace orbit. Plain data so it can be recorded for
 * replays / ghost runs (GDD §2.2 "Deterministic Simulation").
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FMaceOrbitState
{
	GENERATED_BODY()

	/** Current orbit angle theta (radians) in the local YZ plane. 0 = right (+Y). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mace|State")
	float OrbitAngleRad = 0.0f;

	/** Signed angular velocity omega (rad/s). Positive = counter-clockwise from behind. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mace|State")
	float AngularVelocityRadPerSec = 0.0f;

	/** Which spin input is currently held. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mace|State")
	EMaceSpinDirection HeldSpinDirection = EMaceSpinDirection::None;
};

/**
 * Single predicted trajectory point along the forward tunnel corridor.
 * Used by trajectory visualizers, debug HUD, and predictive AI agents.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FTrajectoryPoint
{
	GENERATED_BODY()

	/** World position at sample time. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Trajectory")
	FVector Position = FVector::ZeroVector;

	/** World velocity at sample time. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Trajectory")
	FVector Velocity = FVector::ZeroVector;

	/** Time offset (seconds) from current simulation tick. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Trajectory")
	float TimeSeconds = 0.0f;
};

/**
 * Real-time physics telemetry bundle.
 * Pure data struct designed for debug HUDs, replay logs, and AI observation vectors.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FMaceTelemetry
{
	GENERATED_BODY()

	/** Orbit angle in degrees [0, 360). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	float OrbitAngleDeg = 0.0f;

	/** Instantaneous angular speed (deg/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	float AngularSpeedDegPerSec = 0.0f;

	/** Current rotational cadence (revolutions per minute). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	float RotationsPerMinute = 0.0f;

	/** World location of the rotating mace head. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	FVector MaceHeadWorldPosition = FVector::ZeroVector;

	/** Tangential linear velocity of the mace head in world space (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	FVector TangentialVelocityWorld = FVector::ZeroVector;

	/** Current lateral dodging velocity (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	FVector LateralVelocity = FVector::ZeroVector;

	/** Relentless forward speed along tunnel (+X, cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	float ForwardSpeedCmPerSec = 0.0f;

	/** Active player input direction. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	EMaceSpinDirection SpinDirection = EMaceSpinDirection::None;

	/** Active kick model being evaluated. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Telemetry")
	EMomentumKickModel KickModel = EMomentumKickModel::ReactionRecoil;
};

/**
 * Underlying reason for a run termination condition.
 * Rule 3 Failure States (GDD §2.0 & update.md):
 *  - KineticImpact: ship body collided with obstacle at lethal speed.
 *  - UnreachableVoidDrift: zero reachable obstacles within the 13s–40s lookahead window.
 *  - ZeroVelocity: astronaut stalled relative to environment with no propulsion.
 */
UENUM(BlueprintType)
enum class EDoomFailureReason : uint8
{
	None                 UMETA(DisplayName = "None (Safe)"),
	KineticImpact        UMETA(DisplayName = "Kinetic Impact (Crash)"),
	UnreachableVoidDrift UMETA(DisplayName = "Lost to the Void (Unreachable)"),
	ZeroVelocity         UMETA(DisplayName = "Zero Velocity Stall"),
};

/**
 * Real-time internal kinematics state of the two-body astronaut/mace system.
 * Enforces Newton's third law in vacuum: m_s * r_s + m_m * r_m = 0 relative to system CoM.
 *
 * @creator_feel
 * "Swinging the heavy appendage to the left, the ship's body is violently forced to the right.
 *  The player uses this to dodge obstacles, but if they pull the appendage back to neutral,
 *  their body snaps back to the CoM line. This creates a 'physics debt'."
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FCenterOfMassDisplacementState
{
	GENERATED_BODY()

	/** Relative displacement of astronaut body from system Center of Mass (cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinematics")
	FVector AstronautDisplacement = FVector::ZeroVector;

	/** Relative displacement of mace head from system Center of Mass (cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinematics")
	FVector MaceHeadDisplacement = FVector::ZeroVector;

	/** Counter-rotation angle of astronaut body about forward +X axis (radians). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinematics")
	float CounterRotationAngleRad = 0.0f;

	/** Counter-rotational angular velocity of astronaut body (rad/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinematics")
	float CounterAngularVelocityRadPerSec = 0.0f;

	/** World location of the system Center of Mass (cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinematics")
	FVector SystemCoMPosition = FVector::ZeroVector;

	/** World velocity of the system Center of Mass (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinematics")
	FVector SystemCoMVelocity = FVector::ZeroVector;
};

/**
 * Result data from an external physical collision between the mace head and an obstacle.
 * Rule 1 External Forces (update.md):
 * "The ONLY way to permanently change the trajectory of the ship's Center of Mass is to
 *  physically strike the appendage against an environmental obstacle."
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FExternalStrikeImpulse
{
	GENERATED_BODY()

	/** World location of the physical collision impact point. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	FVector HitLocation = FVector::ZeroVector;

	/** Surface normal of the impacted obstacle at contact point. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	FVector HitNormal = FVector::UpVector;

	/** Relative velocity of mace head relative to obstacle at moment of collision (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	FVector RelativeStrikeVelocity = FVector::ZeroVector;

	/** Transferred external collision impulse vector J_ext (kg * cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	FVector ImpartedImpulse = FVector::ZeroVector;

	/** Permanent velocity change applied to the system Center of Mass: Delta v_CoM = J_ext / M_total (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	FVector ResultingCoMDeltaV = FVector::ZeroVector;

	/** Coefficient of restitution [0 = plastic, 1 = elastic]. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	float Restitution = 0.75f;

	/** Whether the collision shattered a brittle / destructible obstacle. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Collision")
	bool bShatteredObstacle = false;
};

/**
 * Telemetry snapshot from the continuous background predictive Doom Detection check.
 * Sweeps the 13.0s – 40.0s lookahead horizon to detect hopeless trajectories into empty space.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMPHYSICS_API FDoomPredictionSnapshot
{
	GENERATED_BODY()

	/** True if predictive lookahead determined run cannot recover and is doomed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	bool bIsDoomImminent = false;

	/** Underlying reason for doom (ZeroVelocity or UnreachableVoidDrift). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	EDoomFailureReason Reason = EDoomFailureReason::None;

	/** Total lookahead horizon evaluated in seconds (e.g. 40.0s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	float LookaheadHorizonSeconds = 40.0f;

	/** Number of valid obstacles/anchors intersecting the mace's maximum reach envelope. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	int32 ReachableObstacleCount = 0;

	/** Time offset (seconds) to the nearest reachable obstacle, or -1 if none. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	float NearestReachableTimeSeconds = -1.0f;

	/** Estimated seconds before entering irrecoverable void. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	float EstimatedTimeToDoomSeconds = 0.0f;
};


