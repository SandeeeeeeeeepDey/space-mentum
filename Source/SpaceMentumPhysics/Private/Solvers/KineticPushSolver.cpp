/**
 * @file KineticPushSolver.cpp
 * @brief Implementation of pure kinematics and Newton reaction solvers for kinetic push mechanics.
 *
 * @creator_feel
 * "we aim an asteroid and then lock it and then we decide how hard we will push it unless
 *  it is too close, that we need to decide if we still want to push or it will push it.
 *  lets implement the push first, then we can work on the pull"
 *
 * @emotional_intent
 * Enforces pure Newtonian mechanics with visceral gameplay stakes. When pushing a colossal
 * celestial body, its massive inertia demands respect. If the player allows the rock to close
 * inside the critical proximity threshold, weak pushes fail: the asteroid's momentum crushes
 * through the kinetic field and shunts the astronaut violently into the void.
 */

#include "Solvers/KineticPushSolver.h"

void FKineticPushSolver::CalculateTargetTrajectoryMetrics(
	const FVector& TargetPosWorld,
	const FVector& TargetVelWorld,
	const FVector& AstronautPosWorld,
	const FVector& AstronautVelWorld,
	float TargetRadiusCm,
	FVector& OutRelativeVelocity,
	float& OutSurfaceDistanceCm,
	float& OutTimeToImpactSec)
{
	OutRelativeVelocity = TargetVelWorld - AstronautVelWorld;

	const FVector Displacement = TargetPosWorld - AstronautPosWorld;
	const float CenterDistanceCm = Displacement.Size();

	OutSurfaceDistanceCm = FMath::Max(0.0f, CenterDistanceCm - TargetRadiusCm - 60.0f);

	if (CenterDistanceCm <= 1.0f)
	{
		OutTimeToImpactSec = 0.0f;
		return;
	}

	const FVector LineOfSight = Displacement / CenterDistanceCm;
	// Closing speed is the relative velocity projected along the line of sight toward the astronaut
	const float ClosingSpeedCmPerSec = -FVector::DotProduct(OutRelativeVelocity, LineOfSight);

	if (ClosingSpeedCmPerSec > 10.0f)
	{
		OutTimeToImpactSec = OutSurfaceDistanceCm / ClosingSpeedCmPerSec;
	}
	else
	{
		// Relative trajectory is diverging or static
		OutTimeToImpactSec = -1.0f;
	}
}

bool FKineticPushSolver::IsTargetWithinAimCone(
	const FVector& AimOriginWorld,
	const FVector& AimDirection,
	const FVector& TargetPosWorld,
	float MaxRangeCm,
	float HalfAngleDeg,
	float& OutAngleOffAimDeg)
{
	const FVector ToTarget = TargetPosWorld - AimOriginWorld;
	const float Dist = ToTarget.Size();

	if (Dist > MaxRangeCm || Dist < 10.0f)
	{
		OutAngleOffAimDeg = 180.0f;
		return false;
	}

	const FVector NormalizedAim = AimDirection.GetSafeNormal();
	const FVector NormalizedToTarget = ToTarget / Dist;

	const float Dot = FMath::Clamp(FVector::DotProduct(NormalizedAim, NormalizedToTarget), -1.0f, 1.0f);
	OutAngleOffAimDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

	return OutAngleOffAimDeg <= HalfAngleDeg;
}

void FKineticPushSolver::EvaluateKineticPushExecution(
	const FKineticPushTuning& Tuning,
	float PushChargeRatio,
	const FKineticTargetLockInfo& LockInfo,
	float AstronautMassKg,
	FKineticPushExecutionResult& OutResult)
{
	OutResult = FKineticPushExecutionResult();

	if (AstronautMassKg <= 1.0f || LockInfo.TargetMassKg <= 1.0f)
	{
		return;
	}

	const float ClampedCharge = FMath::Clamp(PushChargeRatio, 0.08f, 1.0f);
	const float DeliveredImpulseNs = ClampedCharge * Tuning.MaxPushImpulseNs;

	// Aim line of sight pointing from astronaut toward the target
	FVector AimNormal = (LockInfo.TargetWorldLocation - LockInfo.TargetVelocityWorld).GetSafeNormal();
	if (AimNormal.IsNearlyZero())
	{
		AimNormal = FVector::ForwardVector;
	}

	// Evaluate proximity dilemma
	const bool bIsCriticalProximity = LockInfo.SurfaceDistanceCm <= Tuning.CriticalProximityThresholdCm;

	// Incoming momentum toward the player (kg * m/s = N*s)
	// LockInfo.RelativeVelocity is in cm/s -> convert to m/s by * 0.01
	const FVector IncomingMomentumNs = LockInfo.TargetMassKg * (LockInfo.RelativeVelocity * 0.01f);
	const float ClosingMomentumTowardPlayerNs = FMath::Max(0.0f, FVector::DotProduct(IncomingMomentumNs, -AimNormal));

	if (bIsCriticalProximity)
	{
		// Inside danger zone: check if asteroid momentum overpowers push
		const float OverpowerResistanceThreshold = DeliveredImpulseNs * Tuning.OverpowerMomentumThresholdRatio;

		if (ClosingMomentumTowardPlayerNs > OverpowerResistanceThreshold && ClampedCharge < 0.75f)
		{
			// "IT WILL PUSH IT": The oncoming asteroid's inertia completely overpowers the push!
			OutResult.bImpulseDelivered = true;
			OutResult.Outcome = EProximityDecisionOutcome::AsteroidOverpoweredPush;

			const float ShuntMagnitudeNs = FMath::Clamp(ClosingMomentumTowardPlayerNs * 0.40f, 1200.0f, 8000.0f);
			OutResult.ShuntImpulseMagnitude = ShuntMagnitudeNs;

			// Astronaut is violently shunted backwards along asteroid's travel vector
			const FVector AsteroidTravelDir = LockInfo.TargetVelocityWorld.GetSafeNormal();
			const FVector ShuntDirection = !AsteroidTravelDir.IsNearlyZero() ? AsteroidTravelDir : -AimNormal;

			OutResult.ReactionImpulseToAstronaut = ShuntDirection * ShuntMagnitudeNs;
			// Delta velocity = J / m (m/s) * 100 (cm/s)
			OutResult.AstronautDeltaVelocity = (OutResult.ReactionImpulseToAstronaut / AstronautMassKg) * 100.0f;
			OutResult.TransferredImpulseToAsteroid = FVector::ZeroVector;
			OutResult.AsteroidDeltaVelocity = FVector::ZeroVector;
			return;
		}
		else
		{
			// Heroic point-blank deflection: explosive last-second burst successfully deflects rock
			OutResult.Outcome = EProximityDecisionOutcome::HeroicClosePushDeflect;
		}
	}
	else
	{
		OutResult.Outcome = EProximityDecisionOutcome::SafeRangePush;
	}

	// Apply Newton reaction pair
	const FVector RepulsionImpulseVector = AimNormal * DeliveredImpulseNs;
	OutResult.TransferredImpulseToAsteroid = RepulsionImpulseVector;

	// Asteroid delta velocity: Delta v = J / M_asteroid (m/s * 100 -> cm/s)
	OutResult.AsteroidDeltaVelocity = (RepulsionImpulseVector / LockInfo.TargetMassKg) * 100.0f;

	// Astronaut reaction recoil: Delta v = -J * Efficiency / m_astronaut (m/s * 100 -> cm/s)
	const FVector RecoilImpulseVector = -RepulsionImpulseVector * Tuning.ReactionRecoilEfficiency;
	OutResult.ReactionImpulseToAstronaut = RecoilImpulseVector;
	OutResult.AstronautDeltaVelocity = (RecoilImpulseVector / AstronautMassKg) * 100.0f;

	OutResult.bImpulseDelivered = true;
}
