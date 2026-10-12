/**
 * @file TrajectoryPredictor.h
 * @brief Pure, stateless math for predicting where a release at a given orbit angle will send the astronaut.
 *
 * @creator_feel
 * "by spinning and stopping the spin in a pertiicular direction, one can stir their direction
 *  to move away from obsticles."
 *
 * @emotional_intent
 * Mastery should feel learnable. This solver powers debug gizmos (GDD §2.2 "trajectory
 * prediction visualizers") and, later, optional assist UI — so the player can build
 * intuition for "release HERE, go THERE" until it becomes muscle memory.
 *
 * Design: plain C++ (no UObject) so it is trivially unit-testable and deterministic.
 * Implemented in Step 2 alongside the orbit/transfer components it mirrors.
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/SpaceMentumPhysicsTypes.h"

/**
 * Stateless trajectory math shared by gameplay, debug draw, and tests.
 * All functions are static and side-effect free.
 */
struct SPACEMENTUMPHYSICS_API FTrajectoryPredictor
{
	/**
	 * Unit tangent direction of the mace head in the local YZ plane for a given orbit angle and spin sign.
	 * @param OrbitAngleRad       Orbit angle theta (0 = +Y right, CCW positive from behind).
	 * @param AngularVelocitySign +1 for counter-clockwise, -1 for clockwise.
	 * @return Unit vector with X = 0.
	 */
	static FVector ComputeTangentDirection(float OrbitAngleRad, float AngularVelocitySign);
};
