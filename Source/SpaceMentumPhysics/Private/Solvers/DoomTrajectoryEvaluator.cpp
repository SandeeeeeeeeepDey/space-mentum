/**
 * @file DoomTrajectoryEvaluator.cpp
 * @brief Implementation of predictive void doom trajectory evaluator.
 *
 * @creator_feel
 * "If the engine calculates the player's trajectory will not intersect... trigger a
 *  dramatic failure state... in the next 13 to 40 seconds."
 */

#include "Solvers/DoomTrajectoryEvaluator.h"

bool FDoomTrajectoryEvaluator::EvaluateDoomTrajectory(
	const FVector& CurrentCoMPosition,
	const FVector& CurrentCoMVelocity,
	float MaceReachRadiusCm,
	const TArray<FObstacleSpatialProxy>& CandidateObstacles,
	float MinLookaheadSeconds,
	float MaxLookaheadSeconds,
	float StallVelocityThresholdCmPerSec,
	FDoomPredictionSnapshot& OutSnapshot)
{
	OutSnapshot.LookaheadHorizonSeconds = MaxLookaheadSeconds;
	OutSnapshot.ReachableObstacleCount = 0;
	OutSnapshot.NearestReachableTimeSeconds = -1.0f;
	OutSnapshot.bIsDoomImminent = false;
	OutSnapshot.Reason = EDoomFailureReason::None;
	OutSnapshot.EstimatedTimeToDoomSeconds = 0.0f;

	const float CurrentSpeedSq = CurrentCoMVelocity.SizeSquared();
	const float MinSpeedSq = FMath::Square(StallVelocityThresholdCmPerSec);

	// Condition 1: Velocity stall — zero relative speed to environment
	if (CurrentSpeedSq < MinSpeedSq)
	{
		OutSnapshot.bIsDoomImminent = true;
		OutSnapshot.Reason = EDoomFailureReason::ZeroVelocity;
		OutSnapshot.EstimatedTimeToDoomSeconds = 0.0f;
		return true;
	}

	const float CurrentSpeed = FMath::Sqrt(CurrentSpeedSq);
	float NearestTime = TNumericLimits<float>::Max();
	int32 TotalReachableInRange = 0;
	int32 NearTermReachable = 0;

	for (const FObstacleSpatialProxy& Obstacle : CandidateObstacles)
	{
		if (!Obstacle.bIsValidAnchor)
		{
			continue;
		}

		float HitTime = 0.0f;
		// Check for reachability in the 0 -> MaxLookahead window
		if (IsObstacleReachableAlongTrajectory(
			CurrentCoMPosition,
			CurrentCoMVelocity,
			Obstacle.Center,
			Obstacle.RadiusCm,
			MaceReachRadiusCm,
			0.0f,
			MaxLookaheadSeconds,
			&HitTime))
		{
			if (HitTime < NearestTime)
			{
				NearestTime = HitTime;
			}

			if (HitTime <= MinLookaheadSeconds)
			{
				NearTermReachable++;
			}

			if (HitTime >= MinLookaheadSeconds && HitTime <= MaxLookaheadSeconds)
			{
				TotalReachableInRange++;
			}
		}
	}

	OutSnapshot.ReachableObstacleCount = TotalReachableInRange + NearTermReachable;
	if (NearestTime < TNumericLimits<float>::Max())
	{
		OutSnapshot.NearestReachableTimeSeconds = NearestTime;
	}

	// Condition 2: Void Drift — If there are zero reachable anchors in the 13s–40s lookahead window
	// AND no near-term anchors to steer off of, the player is doomed to drift into empty space.
	if (NearTermReachable == 0 && TotalReachableInRange == 0)
	{
		OutSnapshot.bIsDoomImminent = true;
		OutSnapshot.Reason = EDoomFailureReason::UnreachableVoidDrift;
		// Estimate time until entering irrecoverable deep void
		OutSnapshot.EstimatedTimeToDoomSeconds = MinLookaheadSeconds;
		return true;
	}

	return false;
}

bool FDoomTrajectoryEvaluator::IsObstacleReachableAlongTrajectory(
	const FVector& TrajectoryOrigin,
	const FVector& TrajectoryVelocity,
	const FVector& ObstacleCenter,
	float ObstacleRadiusCm,
	float ReachRadiusCm,
	float MinTimeSeconds,
	float MaxTimeSeconds,
	float* OutIntersectionTime)
{
	const float SpeedSq = TrajectoryVelocity.SizeSquared();
	if (SpeedSq <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// Vector from trajectory origin to obstacle center
	const FVector ToObstacle = ObstacleCenter - TrajectoryOrigin;

	// Project obstacle onto velocity vector: t = (ToObstacle . v) / |v|^2
	const float TimeOfClosestApproach = FVector::DotProduct(ToObstacle, TrajectoryVelocity) / SpeedSq;

	const float Speed = FMath::Sqrt(SpeedSq);
	const float CombinedRadius = ObstacleRadiusCm + ReachRadiusCm;
	const float CombinedRadiusSq = FMath::Square(CombinedRadius);

	// Position on trajectory line at closest approach
	const FVector ClosestPoint = TrajectoryOrigin + (TrajectoryVelocity * TimeOfClosestApproach);
	const float DistanceSq = FVector::DistSquared(ClosestPoint, ObstacleCenter);

	// Perpendicular distance must be within combined reach radius
	if (DistanceSq > CombinedRadiusSq)
	{
		return false;
	}

	// Calculate longitudinal entry/exit bounds along the trajectory tube
	const float HalfChordLength = FMath::Sqrt(CombinedRadiusSq - DistanceSq);
	const float HalfChordTime = HalfChordLength / Speed;

	const float EntryTime = TimeOfClosestApproach - HalfChordTime;
	const float ExitTime = TimeOfClosestApproach + HalfChordTime;

	// Check interval overlap: [EntryTime, ExitTime] with [MinTimeSeconds, MaxTimeSeconds]
	const float OverlapStart = FMath::Max(EntryTime, MinTimeSeconds);
	const float OverlapEnd = FMath::Min(ExitTime, MaxTimeSeconds);

	if (OverlapStart <= OverlapEnd)
	{
		if (OutIntersectionTime)
		{
			*OutIntersectionTime = FMath::Max(TimeOfClosestApproach, 0.0f);
		}
		return true;
	}

	return false;
}
