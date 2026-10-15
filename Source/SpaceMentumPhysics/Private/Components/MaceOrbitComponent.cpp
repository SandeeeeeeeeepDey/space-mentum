/**
 * @file MaceOrbitComponent.cpp
 * @brief Mace orbit component implementation — angular acceleration and deterministic angle integration.
 *
 * @emotional_intent Heavy wind-up, readable sweep, rhythmic release.
 */

#include "Components/MaceOrbitComponent.h"
#include "Solvers/CenterOfMassKinematicsSolver.h"
#include "Solvers/TrajectoryPredictor.h"

UMaceOrbitComponent::UMaceOrbitComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMaceOrbitComponent::BeginPlay()
{
	Super::BeginPlay();

	FCenterOfMassKinematicsSolver::EvaluateInternalKinematics(
		OrbitState.OrbitAngleRad,
		OrbitState.AngularVelocityRadPerSec,
		OrbitTuning.TetherLengthCm,
		OrbitTuning.MaceMassKg,
		AstronautMassKg,
		45.0f,
		CoMDisplacementState
	);
}

void UMaceOrbitComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	const float AccelRadPerSec2 = FMath::DegreesToRadians(FMath::Max(1.0f, OrbitTuning.SpinAccelerationDegPerSec2));
	const float MaxSpeedRadPerSec = FMath::DegreesToRadians(FMath::Max(1.0f, OrbitTuning.MaxAngularSpeedDegPerSec));

	// Accelerate toward target rotational velocity based on held input
	switch (OrbitState.HeldSpinDirection)
	{
	case EMaceSpinDirection::CounterClockwise:
		// Positive omega (angle sweeps right -> up -> left -> down)
		if (OrbitState.AngularVelocityRadPerSec < 0.0f)
		{
			// Counter-spin braking: allow faster reversal so player feels responsive recovery
			OrbitState.AngularVelocityRadPerSec += AccelRadPerSec2 * 2.0f * DeltaTime;
		}
		else
		{
			OrbitState.AngularVelocityRadPerSec = FMath::Min(
				OrbitState.AngularVelocityRadPerSec + AccelRadPerSec2 * DeltaTime,
				MaxSpeedRadPerSec
			);
		}
		break;

	case EMaceSpinDirection::Clockwise:
		// Negative omega (angle sweeps right -> down -> left -> up)
		if (OrbitState.AngularVelocityRadPerSec > 0.0f)
		{
			OrbitState.AngularVelocityRadPerSec -= AccelRadPerSec2 * 2.0f * DeltaTime;
		}
		else
		{
			OrbitState.AngularVelocityRadPerSec = FMath::Max(
				OrbitState.AngularVelocityRadPerSec - AccelRadPerSec2 * DeltaTime,
				-MaxSpeedRadPerSec
			);
		}
		break;

	case EMaceSpinDirection::None:
	default:
		// Spin is arrested on release — momentum has already been transferred to the astronaut
		OrbitState.AngularVelocityRadPerSec = 0.0f;
		break;
	}

	// Integrate orbit angle theta
	if (!FMath::IsNearlyZero(OrbitState.AngularVelocityRadPerSec))
	{
		OrbitState.OrbitAngleRad += OrbitState.AngularVelocityRadPerSec * DeltaTime;

		// Normalize theta to [0, 2*PI)
		OrbitState.OrbitAngleRad = FMath::Fmod(OrbitState.OrbitAngleRad, 2.0f * PI);
		if (OrbitState.OrbitAngleRad < 0.0f)
		{
			OrbitState.OrbitAngleRad += 2.0f * PI;
		}
	}

	// Real-time two-body internal displacement and counter-rotation evaluation
	FCenterOfMassKinematicsSolver::EvaluateInternalKinematics(
		OrbitState.OrbitAngleRad,
		OrbitState.AngularVelocityRadPerSec,
		OrbitTuning.TetherLengthCm,
		OrbitTuning.MaceMassKg,
		AstronautMassKg,
		45.0f,
		CoMDisplacementState
	);
}

void UMaceOrbitComponent::SetHeldSpinDirection(EMaceSpinDirection Direction)
{
	if (OrbitState.HeldSpinDirection == Direction)
	{
		return;
	}

	const EMaceSpinDirection PreviousDirection = OrbitState.HeldSpinDirection;
	OrbitState.HeldSpinDirection = Direction;

	// Transition from a held spin to None: the exact moment of stopping / release!
	if (PreviousDirection != EMaceSpinDirection::None && Direction == EMaceSpinDirection::None)
	{
		const float ReleaseAngle = OrbitState.OrbitAngleRad;
		const float ReleaseOmega = OrbitState.AngularVelocityRadPerSec;

		// Broadcast release event to transfer component and listening systems
		if (OnMaceOrbitReleased.IsBound())
		{
			OnMaceOrbitReleased.Broadcast(
				ReleaseAngle,
				ReleaseOmega,
				OrbitTuning.TetherLengthCm,
				OrbitTuning.MaceMassKg
			);
		}

		// Instantly arrest mace angular velocity upon release
		OrbitState.AngularVelocityRadPerSec = 0.0f;
	}
}

void UMaceOrbitComponent::SetOrbitTuning(const FMaceOrbitTuning& InTuning)
{
	OrbitTuning = InTuning;
}

FVector UMaceOrbitComponent::GetMaceHeadLocalPosition() const
{
	return FTrajectoryPredictor::ComputeMaceHeadLocalPosition(
		OrbitState.OrbitAngleRad,
		OrbitTuning.TetherLengthCm
	);
}

FVector UMaceOrbitComponent::GetMaceHeadWorldPosition() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return GetMaceHeadLocalPosition();
	}

	return OwnerActor->GetActorTransform().TransformPosition(GetMaceHeadLocalPosition());
}

FVector UMaceOrbitComponent::GetTangentialVelocityWorld() const
{
	const AActor* OwnerActor = GetOwner();
	const FVector LocalTangential = FTrajectoryPredictor::ComputeMaceTangentialVelocity(
		OrbitState.OrbitAngleRad,
		OrbitState.AngularVelocityRadPerSec,
		OrbitTuning.TetherLengthCm
	);

	if (!OwnerActor)
	{
		return LocalTangential;
	}

	return OwnerActor->GetActorTransform().TransformVector(LocalTangential);
}

float UMaceOrbitComponent::GetCurrentRPM() const
{
	// 1 rad/s = 60 / (2 * PI) RPM ≈ 9.549296 RPM
	return (FMath::Abs(OrbitState.AngularVelocityRadPerSec) * 60.0f) / (2.0f * PI);
}

float UMaceOrbitComponent::GetCurrentTangentialSpeedCmPerSec() const
{
	return FMath::Abs(OrbitState.AngularVelocityRadPerSec) * OrbitTuning.TetherLengthCm;
}

void UMaceOrbitComponent::ResetOrbit(float InitialAngleRad)
{
	OrbitState.OrbitAngleRad = FMath::Fmod(InitialAngleRad, 2.0f * PI);
	if (OrbitState.OrbitAngleRad < 0.0f)
	{
		OrbitState.OrbitAngleRad += 2.0f * PI;
	}
	OrbitState.AngularVelocityRadPerSec = 0.0f;
	OrbitState.HeldSpinDirection = EMaceSpinDirection::None;
}

FMaceTelemetry UMaceOrbitComponent::GetTelemetry() const
{
	FMaceTelemetry Telemetry;
	Telemetry.OrbitAngleDeg = FMath::RadiansToDegrees(OrbitState.OrbitAngleRad);
	Telemetry.AngularSpeedDegPerSec = FMath::RadiansToDegrees(FMath::Abs(OrbitState.AngularVelocityRadPerSec));
	Telemetry.RotationsPerMinute = GetCurrentRPM();
	Telemetry.MaceHeadWorldPosition = GetMaceHeadWorldPosition();
	Telemetry.TangentialVelocityWorld = GetTangentialVelocityWorld();
	Telemetry.SpinDirection = OrbitState.HeldSpinDirection;
	return Telemetry;
}
