/**
 * @file CenterOfMassKinematicsSolver.cpp
 * @brief Implementation of Google DeepMind-grade Newtonian mechanics solver.
 *
 * @creator_feel
 * "Swinging the heavy appendage to the left, the ship's body is violently forced to the right.
 *  The player uses this to dodge obstacles, but if they pull the appendage back to neutral,
 *  their body snaps back to the CoM line. This creates a 'physics debt'."
 * "The ONLY way to permanently change the trajectory of the ship's Center of Mass is to
 *  physically strike the appendage against an environmental obstacle."
 */

#include "Solvers/CenterOfMassKinematicsSolver.h"

void FCenterOfMassKinematicsSolver::EvaluateInternalKinematics(
	float OrbitAngleRad,
	float AngularVelocityRadPerSec,
	float TetherLengthCm,
	float MaceMassKg,
	float AstronautMassKg,
	float BodyRadiusCm,
	FCenterOfMassDisplacementState& OutState)
{
	const float SafeMaceMass = FMath::Max(MaceMassKg, 0.1f);
	const float SafeAstronautMass = FMath::Max(AstronautMassKg, 1.0f);
	const float TotalMass = SafeMaceMass + SafeAstronautMass;

	// Appendage vector d from astronaut center in local YZ plane
	const float CosTheta = FMath::Cos(OrbitAngleRad);
	const float SinTheta = FMath::Sin(OrbitAngleRad);
	const FVector RelativeAppendageVector(0.0f, CosTheta * TetherLengthCm, SinTheta * TetherLengthCm);

	// Newton's Third Law relative to system Center of Mass:
	// m_s * r_s + m_m * r_m = 0  =>  r_s = - (m_m / M_total) * d
	OutState.AstronautDisplacement = - (SafeMaceMass / TotalMass) * RelativeAppendageVector;
	OutState.MaceHeadDisplacement = + (SafeAstronautMass / TotalMass) * RelativeAppendageVector;

	// Counter-rotation angle and angular velocity
	OutState.CounterRotationAngleRad = ComputeCounterRotationAngleRad(
		OrbitAngleRad,
		TetherLengthCm,
		SafeMaceMass,
		SafeAstronautMass,
		BodyRadiusCm
	);

	// Counter angular velocity (omega_s = - (I_m / I_s) * omega_m)
	const float EffectiveRadius = FMath::Max(BodyRadiusCm, 20.0f);
	const float Im = SafeMaceMass * FMath::Square(TetherLengthCm);
	const float Is = 0.5f * SafeAstronautMass * FMath::Square(EffectiveRadius); // Cylinder inertia
	const float Ratio = FMath::Min(Im / FMath::Max(Is, 1.0f), 4.0f); // Clamped for visual stability

	OutState.CounterAngularVelocityRadPerSec = - Ratio * AngularVelocityRadPerSec;
}

FVector FCenterOfMassKinematicsSolver::ComputeAstronautInternalDisplacement(
	float OrbitAngleRad,
	float TetherLengthCm,
	float MaceMassKg,
	float AstronautMassKg)
{
	const float SafeMaceMass = FMath::Max(MaceMassKg, 0.1f);
	const float SafeAstronautMass = FMath::Max(AstronautMassKg, 1.0f);
	const float TotalMass = SafeMaceMass + SafeAstronautMass;

	const float CosTheta = FMath::Cos(OrbitAngleRad);
	const float SinTheta = FMath::Sin(OrbitAngleRad);
	const FVector RelativeAppendageVector(0.0f, CosTheta * TetherLengthCm, SinTheta * TetherLengthCm);

	return - (SafeMaceMass / TotalMass) * RelativeAppendageVector;
}

FVector FCenterOfMassKinematicsSolver::ComputeMaceHeadInternalDisplacement(
	float OrbitAngleRad,
	float TetherLengthCm,
	float MaceMassKg,
	float AstronautMassKg)
{
	const float SafeMaceMass = FMath::Max(MaceMassKg, 0.1f);
	const float SafeAstronautMass = FMath::Max(AstronautMassKg, 1.0f);
	const float TotalMass = SafeMaceMass + SafeAstronautMass;

	const float CosTheta = FMath::Cos(OrbitAngleRad);
	const float SinTheta = FMath::Sin(OrbitAngleRad);
	const FVector RelativeAppendageVector(0.0f, CosTheta * TetherLengthCm, SinTheta * TetherLengthCm);

	return + (SafeAstronautMass / TotalMass) * RelativeAppendageVector;
}

float FCenterOfMassKinematicsSolver::ComputeCounterRotationAngleRad(
	float OrbitAngleRad,
	float TetherLengthCm,
	float MaceMassKg,
	float AstronautMassKg,
	float BodyRadiusCm)
{
	const float SafeMaceMass = FMath::Max(MaceMassKg, 0.1f);
	const float SafeAstronautMass = FMath::Max(AstronautMassKg, 1.0f);
	const float EffectiveRadius = FMath::Max(BodyRadiusCm, 20.0f);

	// Moment of inertia: I_m = m_m * L^2, I_s = 1/2 * m_s * R_s^2 (solid cylinder along axis)
	const float Im = SafeMaceMass * FMath::Square(TetherLengthCm);
	const float Is = 0.5f * SafeAstronautMass * FMath::Square(EffectiveRadius);

	// Ratio clamped to avoid disorienting hyper-spinning while preserving counter-roll feeling
	const float Ratio = FMath::Clamp(Im / FMath::Max(Is, 1.0f), 0.1f, 2.5f);

	return - Ratio * OrbitAngleRad;
}

bool FCenterOfMassKinematicsSolver::SolveExternalStrikeCollision(
	const FVector& MaceVelocityWorld,
	const FVector& ObstacleVelocityWorld,
	const FVector& HitLocationWorld,
	const FVector& HitNormalWorld,
	float TotalSystemMassKg,
	float ObstacleMassKg,
	float Restitution,
	bool bIsAnchored,
	FExternalStrikeImpulse& OutImpulse)
{
	const FVector Normal = HitNormalWorld.GetSafeNormal();
	if (Normal.IsNearlyZero())
	{
		return false;
	}

	const FVector RelativeVelocity = MaceVelocityWorld - ObstacleVelocityWorld;
	const float NormalVelocity = FVector::DotProduct(RelativeVelocity, Normal);

	// Only resolve compressive impact (closing distance along normal into obstacle)
	// Normal points outward from obstacle surface into mace head, so collision closing velocity is negative
	if (NormalVelocity >= 0.0f)
	{
		return false;
	}

	const float SafeSystemMass = FMath::Max(TotalSystemMassKg, 1.0f);
	const float ClampedRestitution = FMath::Clamp(Restitution, 0.0f, 1.0f);

	// Reduced mass calculation
	float ReducedMass = SafeSystemMass;
	if (!bIsAnchored)
	{
		const float SafeObstacleMass = FMath::Max(ObstacleMassKg, 1.0f);
		ReducedMass = (SafeSystemMass * SafeObstacleMass) / (SafeSystemMass + SafeObstacleMass);
	}

	// Normal collision impulse magnitude: J_n = - (1 + e) * mu * v_n
	const float ImpulseMagnitude = - (1.0f + ClampedRestitution) * ReducedMass * NormalVelocity;
	const FVector ImpulseVector = ImpulseMagnitude * Normal;

	// Populate output impulse
	OutImpulse.HitLocation = HitLocationWorld;
	OutImpulse.HitNormal = Normal;
	OutImpulse.RelativeStrikeVelocity = RelativeVelocity;
	OutImpulse.ImpartedImpulse = ImpulseVector;
	OutImpulse.Restitution = ClampedRestitution;

	// Permanent Center of Mass velocity deflection: Delta v_CoM = J_ext / M_total
	OutImpulse.ResultingCoMDeltaV = ImpulseVector / SafeSystemMass;
	OutImpulse.bShatteredObstacle = false;

	return true;
}

bool FCenterOfMassKinematicsSolver::IsKineticImpactLethal(
	const FVector& RelativeVelocityWorld,
	float LethalSpeedThresholdCmPerSec)
{
	return RelativeVelocityWorld.SizeSquared() >= FMath::Square(LethalSpeedThresholdCmPerSec);
}
