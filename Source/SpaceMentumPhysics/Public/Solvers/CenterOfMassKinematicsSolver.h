/**
 * @file CenterOfMassKinematicsSolver.h
 * @brief Google DeepMind-grade pure kinematics solver for Internal vs External Newtonian mechanics.
 *
 * @creator_feel
 * "Swinging the heavy appendage to the left, the ship's body is violently forced to the right.
 *  The player uses this to dodge obstacles, but if they pull the appendage back to neutral,
 *  their body snaps back to the CoM line. This creates a 'physics debt'."
 * "The ONLY way to permanently change the trajectory of the ship's Center of Mass is to
 *  physically strike the appendage against an environmental obstacle."
 *
 * @emotional_intent
 * Pure Newtonian duality. In free space, internal movement is zero-sum: the player dodges
 * via counter-displacement, but inherits physics debt. To alter their destiny and permanently
 * bend their trajectory around cosmic graveyards, they must land brutal physical strikes
 * against environmental obstacles.
 *
 * Physics Specifications (update.md Rule 1 & GDD §2.0):
 *  1. Internal Conservation: m_s * r_s + m_m * r_m = 0 relative to system Center of Mass.
 *     Default 1:4 ratio displaces astronaut body by -0.25 * r_mace.
 *  2. Counter-Rotation: I_s * omega_s + I_m * omega_m = 0 around the forward roll axis.
 *  3. External Momentum Transfer: Delta v_CoM = J_ext / M_total on physical obstacle impact.
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/SpaceMentumPhysicsTypes.h"

/**
 * Pure, stateless solver for two-body internal displacement, counter-rotation,
 * and external collision impulse transfer.
 */
struct SPACEMENTUMPHYSICS_API FCenterOfMassKinematicsSolver
{
	/**
	 * Evaluates internal two-body displacement and counter-rotation in local coordinates.
	 *
	 * Local coordinate conventions:
	 *   +X = forward drift axis
	 *   +Y = right (angle theta = 0 rad)
	 *   +Z = up (angle theta = PI/2 rad)
	 *
	 * @param OrbitAngleRad           Current mace orbit angle theta in radians.
	 * @param AngularVelocityRadPerSec Current mace angular velocity omega (rad/s).
	 * @param TetherLengthCm          Tether radius L (cm).
	 * @param MaceMassKg              Mass of the heavy appendage m_m (default 40 kg).
	 * @param AstronautMassKg         Mass of astronaut body m_s (default 160 kg for 1:4 ratio).
	 * @param BodyRadiusCm            Effective radius of astronaut body for moment of inertia calculation (cm).
	 * @param OutState                Output state struct populated with displacements and counter-rotations.
	 */
	static void EvaluateInternalKinematics(
		float OrbitAngleRad,
		float AngularVelocityRadPerSec,
		float TetherLengthCm,
		float MaceMassKg,
		float AstronautMassKg,
		float BodyRadiusCm,
		FCenterOfMassDisplacementState& OutState);

	/**
	 * Computes the relative displacement vector of the astronaut body from the system CoM.
	 * r_s = - (m_m / (m_s + m_m)) * d_mace
	 */
	static FVector ComputeAstronautInternalDisplacement(
		float OrbitAngleRad,
		float TetherLengthCm,
		float MaceMassKg,
		float AstronautMassKg);

	/**
	 * Computes the relative displacement vector of the mace head from the system CoM.
	 * r_m = + (m_s / (m_s + m_m)) * d_mace
	 */
	static FVector ComputeMaceHeadInternalDisplacement(
		float OrbitAngleRad,
		float TetherLengthCm,
		float MaceMassKg,
		float AstronautMassKg);

	/**
	 * Computes the astronaut body's counter-rotation angle about the forward +X axis.
	 * phi_s = - (I_m / I_s) * theta_m
	 */
	static float ComputeCounterRotationAngleRad(
		float OrbitAngleRad,
		float TetherLengthCm,
		float MaceMassKg,
		float AstronautMassKg,
		float BodyRadiusCm);

	/**
	 * Resolves an external physical strike between the mace head and an environmental obstacle.
	 * Translates contact impulse into permanent Center of Mass trajectory deflection.
	 *
	 * @param MaceVelocityWorld        World velocity of mace head at collision (cm/s).
	 * @param ObstacleVelocityWorld    World velocity of obstacle (cm/s, zero if anchored).
	 * @param HitLocationWorld         World location of contact point (cm).
	 * @param HitNormalWorld           Normal pointing out of obstacle surface into mace head.
	 * @param TotalSystemMassKg        Total mass of astronaut + mace (M_total = m_s + m_m).
	 * @param ObstacleMassKg           Mass of obstacle (ignored if bIsAnchored is true).
	 * @param Restitution              Collision coefficient of restitution e [0, 1].
	 * @param bIsAnchored              True if obstacle is anchored / massive asteroid.
	 * @param OutImpulse               Populated impulse result containing Delta v_CoM.
	 * @return True if a valid compressive impact occurred (separating velocities return false).
	 */
	static bool SolveExternalStrikeCollision(
		const FVector& MaceVelocityWorld,
		const FVector& ObstacleVelocityWorld,
		const FVector& HitLocationWorld,
		const FVector& HitNormalWorld,
		float TotalSystemMassKg,
		float ObstacleMassKg,
		float Restitution,
		bool bIsAnchored,
		FExternalStrikeImpulse& OutImpulse);

	/**
	 * Evaluates whether an astronaut body impact exceeds the survivable kinetic energy threshold.
	 * Rule 3 Failure State: Kinetic Impact (Death by Crashing).
	 *
	 * @param RelativeVelocityWorld Relative impact velocity vector (cm/s).
	 * @param LethalSpeedThresholdCmPerSec Speed threshold above which impact is fatal (default 800 cm/s = 28.8 km/h).
	 * @return True if impact is fatal.
	 */
	static bool IsKineticImpactLethal(
		const FVector& RelativeVelocityWorld,
		float LethalSpeedThresholdCmPerSec = 800.0f);
};
