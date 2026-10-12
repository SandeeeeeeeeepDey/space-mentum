/**
 * @file TrajectoryPredictor.h
 * @brief Pure, stateless mathematical solver for mace orbit kinematics, impulse transfer, and projected flight paths.
 *
 * @creator_feel
 * "by spinning and stopping the spin in a pertiicular direction, one can stir their direction
 *  to move away from obsticles ... it has physics."
 *
 * @emotional_intent
 * Mastery must feel learnable and deterministic. This solver powers debug gizmos, HUD prediction
 * ribbons, and replay verification — allowing the player to build rhythm and muscle memory
 * for "release HERE, go THERE".
 *
 * Engineering Standards:
 * - Google DeepMind style modular pure math solver.
 * - Stateless, zero memory allocation during evaluation, fully deterministic.
 * - Adheres strictly to coordinate conventions in Docs/TECHNICAL_SPECIFICATIONS.md §3.
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/SpaceMentumPhysicsTypes.h"

/**
 * Stateless trajectory and momentum solver shared by physics components, debug gizmos, and tests.
 * All functions are static and free of side effects.
 */
struct SPACEMENTUMPHYSICS_API FTrajectoryPredictor
{
	/**
	 * Unit tangent direction of the mace head in the local YZ plane for a given orbit angle and spin sign.
	 * Coordinate conventions:
	 *   Local +X = forward drift
	 *   Local +Y = right (angle theta = 0 rad)
	 *   Local +Z = up (angle theta = PI/2 rad)
	 *
	 * @param OrbitAngleRad       Orbit angle theta in radians (0 = +Y right, CCW positive seen from behind).
	 * @param AngularVelocitySign +1 for counter-clockwise, -1 for clockwise.
	 * @return Unit direction vector with X = 0.
	 */
	static FVector ComputeTangentDirection(float OrbitAngleRad, float AngularVelocitySign);

	/**
	 * Computes the mace head's local position on the YZ orbit circle.
	 *
	 * @param OrbitAngleRad  Orbit angle theta (radians).
	 * @param TetherLengthCm Radius of orbit r (cm).
	 * @return Local position offset with X = 0.
	 */
	static FVector ComputeMaceHeadLocalPosition(float OrbitAngleRad, float TetherLengthCm);

	/**
	 * Computes the instantaneous tangential linear velocity vector of the mace head in local space.
	 * v_t = omega * r * tangent_dir
	 *
	 * @param OrbitAngleRad           Current orbit angle theta (rad).
	 * @param AngularVelocityRadPerSec Signed angular velocity omega (rad/s).
	 * @param TetherLengthCm          Tether radius r (cm).
	 * @return Tangential velocity vector in local space (cm/s).
	 */
	static FVector ComputeMaceTangentialVelocity(float OrbitAngleRad, float AngularVelocityRadPerSec, float TetherLengthCm);

	/**
	 * Computes the kinetic impulse vector J imparted by arresting the mace spin.
	 * GDD §3.1.2:
	 *   ReactionRecoil: J = -alpha * m_m * v_t (opposing reaction force)
	 *   CarryAlong:     J = +alpha * m_m * v_t (carry-along yank)
	 *
	 * @param OrbitAngleRad           Angle at release (rad).
	 * @param AngularVelocityRadPerSec Angular velocity at release (rad/s).
	 * @param TetherLengthCm          Tether radius (cm).
	 * @param MaceMassKg              Mass of mace head (kg).
	 * @param EfficiencyAlpha         Impulse transfer coefficient alpha [0, 2].
	 * @param KickModel               ReactionRecoil vs CarryAlong.
	 * @return Impulse vector J in local YZ coordinates (kg * cm/s).
	 */
	static FVector ComputeSteeringImpulse(
		float OrbitAngleRad,
		float AngularVelocityRadPerSec,
		float TetherLengthCm,
		float MaceMassKg,
		float EfficiencyAlpha,
		EMomentumKickModel KickModel);

	/**
	 * Computes the instantaneous delta velocity delta_v imparted to the astronaut.
	 * delta_v = J / m_astronaut
	 *
	 * @param OrbitAngleRad           Angle at release (rad).
	 * @param AngularVelocityRadPerSec Angular velocity at release (rad/s).
	 * @param OrbitTuning             Tuning parameters for the mace orbit.
	 * @param TransferTuning          Tuning parameters for the transfer kick.
	 * @return Velocity change vector in local YZ coordinates (cm/s).
	 */
	static FVector ComputeSteeringDeltaVelocity(
		float OrbitAngleRad,
		float AngularVelocityRadPerSec,
		const FMaceOrbitTuning& OrbitTuning,
		const FMomentumTransferTuning& TransferTuning);

	/**
	 * Predicts future trajectory points in world space along the corridor over a discrete time horizon.
	 * Accounts for constant forward drift (+X) and exponential damping on lateral velocity (YZ).
	 *
	 * @param StartWorldLocation    Astronaut starting position in world space.
	 * @param ForwardVector         Normalized forward direction (+X axis in world space).
	 * @param RightVector           Normalized right direction (+Y axis in world space).
	 * @param UpVector              Normalized up direction (+Z axis in world space).
	 * @param ForwardSpeedCmPerSec  Uniform forward velocity along ForwardVector.
	 * @param CurrentLateralLocal   Current lateral velocity (Y, Z) in local coordinates.
	 * @param LateralDampingPerSec  Exponential decay rate (1/s).
	 * @param TimeHorizonSec        How many seconds into the future to project (e.g. 1.5s).
	 * @param StepCount             Number of discrete sample points (e.g. 24).
	 * @param OutPoints             Array populated with projected trajectory sample points.
	 */
	static void PredictTrajectoryCurve(
		const FVector& StartWorldLocation,
		const FVector& ForwardVector,
		const FVector& RightVector,
		const FVector& UpVector,
		float ForwardSpeedCmPerSec,
		const FVector& CurrentLateralLocal,
		float LateralDampingPerSec,
		float TimeHorizonSec,
		int32 StepCount,
		TArray<FTrajectoryPoint>& OutPoints);
};
