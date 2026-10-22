/**
 * @file KineticPushSolver.h
 * @brief Google DeepMind-grade pure analytical solver for kinetic push mechanics and proximity decisions.
 *
 * @creator_feel
 * "the game needs a screen where the asteroids are much bigger too.
 *  the belt of the asteroid needs to be much bigger and outside the view.
 *  the asteroids should be comming from any angle which has atleast one component from the forward.
 *  we aim an asteroid and then lock it and then we decide how hard we will push it unless
 *  it is too close, that we need to decide if we still want to push or it will push it.
 *  lets implement the push first, then we can work on the pull"
 *
 * @emotional_intent
 * High-stakes mathematical rigor. This solver models the Newton reaction pair when the player
 * fires an explosive hydraulic/kinetic push against an incoming colossal celestial body.
 * If the player hesitates until critical proximity, the solver determines whether the player's
 * charge rating is sufficient to overcome the asteroid's momentum or if the incoming rock's
 * crushing inertia overpowers the push and drives the player backward ("it will push it").
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/SpaceMentumKineticManipulationTypes.h"

/**
 * Pure, stateless mathematical solver for kinetic push kinematics,
 * aim resolution, and close-proximity overpower outcomes.
 */
struct SPACEMENTUMPHYSICS_API FKineticPushSolver
{
	/**
	 * Computes the relative closing velocity and estimated time-to-impact (TTI).
	 *
	 * @param TargetPosWorld       Asteroid center of mass world location.
	 * @param TargetVelWorld       Asteroid world velocity vector (cm/s).
	 * @param AstronautPosWorld    Astronaut world location.
	 * @param AstronautVelWorld    Astronaut world velocity vector (cm/s).
	 * @param TargetRadiusCm       Target asteroid collision radius (cm).
	 * @param OutRelativeVelocity  Output relative velocity (TargetVel - AstronautVel).
	 * @param OutSurfaceDistanceCm Output Euclidean surface-to-surface distance.
	 * @param OutTimeToImpactSec   Output time-to-impact (seconds, or -1 if diverging).
	 */
	static void CalculateTargetTrajectoryMetrics(
		const FVector& TargetPosWorld,
		const FVector& TargetVelWorld,
		const FVector& AstronautPosWorld,
		const FVector& AstronautVelWorld,
		float TargetRadiusCm,
		FVector& OutRelativeVelocity,
		float& OutSurfaceDistanceCm,
		float& OutTimeToImpactSec);

	/**
	 * Tests whether a prospective candidate asteroid falls within the player's aim acquisition cone.
	 *
	 * @param AimOriginWorld  World origin of the aiming ray (astronaut position).
	 * @param AimDirection    Normalized unit direction vector of the player's aim.
	 * @param TargetPosWorld  World location of the candidate asteroid.
	 * @param MaxRangeCm      Maximum allowed lock range (cm).
	 * @param HalfAngleDeg    Half-angle of the targeting cone (degrees).
	 * @param OutAngleOffAimDeg Output angular deviation from the aim vector.
	 * @return True if target is within range and within the cone.
	 */
	static bool IsTargetWithinAimCone(
		const FVector& AimOriginWorld,
		const FVector& AimDirection,
		const FVector& TargetPosWorld,
		float MaxRangeCm,
		float HalfAngleDeg,
		float& OutAngleOffAimDeg);

	/**
	 * Evaluates the full kinetic push physics solution, calculating the transferred impulse,
	 * the asteroid deflection, the astronaut reaction recoil, and the critical proximity outcome.
	 *
	 * @param Tuning              Tuning configuration (push force, overpower thresholds, etc.).
	 * @param PushChargeRatio     Normalized player push charge [0.0, 1.0].
	 * @param LockInfo            Current snapshot of the locked target.
	 * @param AstronautMassKg     Total mass of the astronaut + equipment (kg).
	 * @param OutResult           Complete execution result bundle.
	 */
	static void EvaluateKineticPushExecution(
		const FKineticPushTuning& Tuning,
		float PushChargeRatio,
		const FKineticTargetLockInfo& LockInfo,
		float AstronautMassKg,
		FKineticPushExecutionResult& OutResult);
};
