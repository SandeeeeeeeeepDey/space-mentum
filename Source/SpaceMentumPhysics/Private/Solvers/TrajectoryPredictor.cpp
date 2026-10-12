/**
 * @file TrajectoryPredictor.cpp
 * @brief Stateless, deterministic mathematical solver implementation.
 */

#include "Solvers/TrajectoryPredictor.h"

FVector FTrajectoryPredictor::ComputeTangentDirection(const float OrbitAngleRad, const float AngularVelocitySign)
{
	// Mace head position on the unit circle in local YZ: (0, cos(theta), sin(theta)).
	// d/dtheta of that is (0, -sin(theta), cos(theta)) — the counter-clockwise tangent.
	// Clockwise spin simply flips the tangent direction.
	const float Sign = AngularVelocitySign >= 0.0f ? 1.0f : -1.0f;
	float SinTheta = 0.0f;
	float CosTheta = 1.0f;
	FMath::SinCos(&SinTheta, &CosTheta, OrbitAngleRad);
	return FVector(0.0f, -SinTheta * Sign, CosTheta * Sign);
}

FVector FTrajectoryPredictor::ComputeMaceHeadLocalPosition(const float OrbitAngleRad, const float TetherLengthCm)
{
	float SinTheta = 0.0f;
	float CosTheta = 1.0f;
	FMath::SinCos(&SinTheta, &CosTheta, OrbitAngleRad);
	return FVector(0.0f, TetherLengthCm * CosTheta, TetherLengthCm * SinTheta);
}

FVector FTrajectoryPredictor::ComputeMaceTangentialVelocity(
	const float OrbitAngleRad,
	const float AngularVelocityRadPerSec,
	const float TetherLengthCm)
{
	if (FMath::IsNearlyZero(AngularVelocityRadPerSec))
	{
		return FVector::ZeroVector;
	}

	const float Sign = AngularVelocityRadPerSec >= 0.0f ? 1.0f : -1.0f;
	const FVector TangentDir = ComputeTangentDirection(OrbitAngleRad, Sign);
	const float TangentialSpeedCmPerSec = FMath::Abs(AngularVelocityRadPerSec) * TetherLengthCm;

	return TangentDir * TangentialSpeedCmPerSec;
}

FVector FTrajectoryPredictor::ComputeSteeringImpulse(
	const float OrbitAngleRad,
	const float AngularVelocityRadPerSec,
	const float TetherLengthCm,
	const float MaceMassKg,
	const float EfficiencyAlpha,
	const EMomentumKickModel KickModel)
{
	const FVector TangentialVel = ComputeMaceTangentialVelocity(OrbitAngleRad, AngularVelocityRadPerSec, TetherLengthCm);
	if (TangentialVel.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	// J = alpha * m_m * v_t
	const float DirectionMultiplier = (KickModel == EMomentumKickModel::ReactionRecoil) ? -1.0f : 1.0f;
	return TangentialVel * (DirectionMultiplier * EfficiencyAlpha * MaceMassKg);
}

FVector FTrajectoryPredictor::ComputeSteeringDeltaVelocity(
	const float OrbitAngleRad,
	const float AngularVelocityRadPerSec,
	const FMaceOrbitTuning& OrbitTuning,
	const FMomentumTransferTuning& TransferTuning)
{
	// Accidental taps under minimum angular speed threshold yield zero impulse
	const float AngularSpeedDegPerSec = FMath::RadiansToDegrees(FMath::Abs(AngularVelocityRadPerSec));
	if (AngularSpeedDegPerSec < TransferTuning.MinAngularSpeedForKickDegPerSec)
	{
		return FVector::ZeroVector;
	}

	const FVector Impulse = ComputeSteeringImpulse(
		OrbitAngleRad,
		AngularVelocityRadPerSec,
		OrbitTuning.TetherLengthCm,
		OrbitTuning.MaceMassKg,
		TransferTuning.ImpulseEfficiencyAlpha,
		TransferTuning.KickModel
	);

	const float SafeAstronautMassKg = FMath::Max(1.0f, TransferTuning.AstronautMassKg);
	// delta_v = J / m_astronaut
	return Impulse / SafeAstronautMassKg;
}

void FTrajectoryPredictor::PredictTrajectoryCurve(
	const FVector& StartWorldLocation,
	const FVector& ForwardVector,
	const FVector& RightVector,
	const FVector& UpVector,
	const float ForwardSpeedCmPerSec,
	const FVector& CurrentLateralLocal,
	const float LateralDampingPerSec,
	const float TimeHorizonSec,
	const int32 StepCount,
	TArray<FTrajectoryPoint>& OutPoints)
{
	OutPoints.Reset(FMath::Max(1, StepCount));
	if (StepCount <= 0 || TimeHorizonSec <= 0.0f)
	{
		return;
	}

	const float DeltaTime = TimeHorizonSec / static_cast<float>(StepCount);
	for (int32 Index = 0; Index <= StepCount; ++Index)
	{
		const float TimeOffset = Index * DeltaTime;

		// Exact analytical closed-form integral:
		// Forward displacement = v_forward * t
		const float ForwardDisplacement = ForwardSpeedCmPerSec * TimeOffset;

		// Lateral displacement with exponential damping:
		// integral_0^t v_0 * exp(-lambda * tau) dtau = v_0 * (1 - exp(-lambda * t)) / lambda
		float LateralFactor = TimeOffset;
		float CurrentDecayedFactor = 1.0f;
		if (LateralDampingPerSec > 0.0001f)
		{
			CurrentDecayedFactor = FMath::Exp(-LateralDampingPerSec * TimeOffset);
			LateralFactor = (1.0f - CurrentDecayedFactor) / LateralDampingPerSec;
		}

		const float LateralYDisplacement = CurrentLateralLocal.Y * LateralFactor;
		const float LateralZDisplacement = CurrentLateralLocal.Z * LateralFactor;

		// Instantaneous world velocity at sample time
		const FVector SampleWorldVelocity =
			ForwardVector * ForwardSpeedCmPerSec +
			RightVector * (CurrentLateralLocal.Y * CurrentDecayedFactor) +
			UpVector * (CurrentLateralLocal.Z * CurrentDecayedFactor);

		// Projected world location at sample time
		const FVector SampleWorldLocation =
			StartWorldLocation +
			ForwardVector * ForwardDisplacement +
			RightVector * LateralYDisplacement +
			UpVector * LateralZDisplacement;

		FTrajectoryPoint Point;
		Point.Position = SampleWorldLocation;
		Point.Velocity = SampleWorldVelocity;
		Point.TimeSeconds = TimeOffset;
		OutPoints.Add(Point);
	}
}
