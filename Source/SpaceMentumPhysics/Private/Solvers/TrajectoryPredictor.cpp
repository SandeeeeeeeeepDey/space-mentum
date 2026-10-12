/**
 * @file TrajectoryPredictor.cpp
 * @brief Stateless trajectory math.
 */

#include "Solvers/TrajectoryPredictor.h"

FVector FTrajectoryPredictor::ComputeTangentDirection(const float OrbitAngleRad, const float AngularVelocitySign)
{
	// Mace head position on the unit circle in local YZ: (0, cos(theta), sin(theta)).
	// d/dtheta of that is (0, -sin(theta), cos(theta)) — the counter-clockwise tangent.
	// Clockwise spin simply flips it.
	const float Sign = AngularVelocitySign >= 0.0f ? 1.0f : -1.0f;
	float SinTheta = 0.0f;
	float CosTheta = 1.0f;
	FMath::SinCos(&SinTheta, &CosTheta, OrbitAngleRad);
	return FVector(0.0f, -SinTheta * Sign, CosTheta * Sign);
}
