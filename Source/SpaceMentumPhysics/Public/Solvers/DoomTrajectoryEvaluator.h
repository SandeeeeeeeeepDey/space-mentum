/**
 * @file DoomTrajectoryEvaluator.h
 * @brief Google DeepMind-grade background predictive trajectory lookahead evaluator ("The Void").
 *
 * @creator_feel
 * "If the engine calculates the player's trajectory will not intersect... trigger a
 *  dramatic failure state... in the next 13 to 40 seconds."
 * "The terror of space isn't just crashing; it's drifting helplessly. The game must
 *  mercilessly cut off doomed runs to keep the adrenaline high and pacing fast."
 *
 * @emotional_intent
 * Existential Dread. In deep space, drifting with no means of steering is a death sentence.
 * This evaluator projects the Center of Mass trajectory into a 13s–40s future horizon.
 * If zero obstacles/anchors exist within reach of the mace, the evaluator flags an instant
 * failure condition to terminate aimless drift and preserve high adrenaline.
 *
 * Requirements (update.md Rule 3 & GDD §2.1 F-4):
 *  - Continuous background check.
 *  - Condition 1: System velocity drops below survivable movement threshold.
 *  - Condition 2: Trajectory will encounter zero strikeable obstacles within the 13s to 40s horizon.
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/SpaceMentumPhysicsTypes.h"

/** Representation of an obstacle bounding sphere for spatial reachability sweeps. */
struct SPACEMENTUMPHYSICS_API FObstacleSpatialProxy
{
	/** World location of obstacle center. */
	FVector Center = FVector::ZeroVector;

	/** Effective collision radius (cm). */
	float RadiusCm = 150.0f;

	/** Whether this obstacle is a valid strike anchor for momentum transfer. */
	bool bIsValidAnchor = true;

	FObstacleSpatialProxy() = default;
	FObstacleSpatialProxy(const FVector& InCenter, float InRadius, bool bInValidAnchor = true)
		: Center(InCenter), RadiusCm(InRadius), bIsValidAnchor(bInValidAnchor) {}
};

/**
 * Pure, stateless mathematical solver for trajectory lookahead and void doom detection.
 */
struct SPACEMENTUMPHYSICS_API FDoomTrajectoryEvaluator
{
	/**
	 * Default lookahead horizon parameters specified by Creator directive.
	 */
	static constexpr float DefaultMinLookaheadSeconds = 13.0f;
	static constexpr float DefaultMaxLookaheadSeconds = 40.0f;
	static constexpr float DefaultStallVelocityThresholdCmPerSec = 100.0f; // 1 m/s

	/**
	 * Evaluates future trajectory for void soft-locks or velocity stalls.
	 *
	 * @param CurrentCoMPosition           Current world position of system Center of Mass.
	 * @param CurrentCoMVelocity           Current world velocity of system Center of Mass.
	 * @param MaceReachRadiusCm            Total reach of mace (TetherLength + MaceRadius + margin).
	 * @param CandidateObstacles           Array of environmental obstacles within the corridor.
	 * @param MinLookaheadSeconds          Minimum lookahead horizon (default 13.0s).
	 * @param MaxLookaheadSeconds          Maximum lookahead horizon (default 40.0s).
	 * @param StallVelocityThresholdCmPerSec Minimum required speed (default 100 cm/s).
	 * @param OutSnapshot                  Output telemetry snapshot with doom diagnosis.
	 * @return True if a doom failure condition is detected.
	 */
	static bool EvaluateDoomTrajectory(
		const FVector& CurrentCoMPosition,
		const FVector& CurrentCoMVelocity,
		float MaceReachRadiusCm,
		const TArray<FObstacleSpatialProxy>& CandidateObstacles,
		float MinLookaheadSeconds,
		float MaxLookaheadSeconds,
		float StallVelocityThresholdCmPerSec,
		FDoomPredictionSnapshot& OutSnapshot);

	/**
	 * Tests whether a single obstacle bounding sphere intersects the reachable trajectory tube.
	 *
	 * @param TrajectoryOrigin    Starting position of Center of Mass.
	 * @param TrajectoryVelocity  Linear velocity vector.
	 * @param ObstacleCenter      Center of the target obstacle.
	 * @param ObstacleRadiusCm    Radius of the target obstacle.
	 * @param ReachRadiusCm       Maximum reach of mace tether + head.
	 * @param MinTimeSeconds      Minimum time horizon bound.
	 * @param MaxTimeSeconds      Maximum time horizon bound.
	 * @param OutIntersectionTime Optional output for time along trajectory of closest approach.
	 * @return True if obstacle can be physically struck within the specified time window.
	 */
	static bool IsObstacleReachableAlongTrajectory(
		const FVector& TrajectoryOrigin,
		const FVector& TrajectoryVelocity,
		const FVector& ObstacleCenter,
		float ObstacleRadiusCm,
		float ReachRadiusCm,
		float MinTimeSeconds,
		float MaxTimeSeconds,
		float* OutIntersectionTime = nullptr);
};
