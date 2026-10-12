/**
 * @file SpaceMentumDebugVisualizer.cpp
 * @brief Telemetry visualizer and real-time trajectory gizmo implementation.
 */

#include "Debug/SpaceMentumDebugVisualizer.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Components/MaceOrbitComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Components/ForwardDriftComponent.h"
#include "Solvers/TrajectoryPredictor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

void FSpaceMentumDebugVisualizer::DrawCharacterDebugGizmos(
	const UObject* WorldContextObject,
	const ASpaceAstronautCharacter* Character,
	float DeltaTime)
{
	if (!WorldContextObject || !Character)
	{
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return;
	}

	const UMaceOrbitComponent* Orbit = Character->GetMaceOrbit();
	const UMomentumTransferComponent* Transfer = Character->GetMomentumTransfer();
	const UForwardDriftComponent* Drift = Character->GetForwardDrift();

	if (!Orbit || !Transfer || !Drift)
	{
		return;
	}

	const FVector Center = Character->GetActorLocation();
	const FVector Forward = Character->GetActorForwardVector();
	const FVector Right = Character->GetActorRightVector();
	const FVector Up = Character->GetActorUpVector();

	const FMaceOrbitTuning OrbitTuning = Orbit->GetOrbitTuning();
	const FMaceOrbitState OrbitState = Orbit->GetOrbitState();
	const FMomentumTransferTuning TransferTuning = Transfer->GetTransferTuning();

	// 1. Draw 360-degree orbit ring in local YZ plane
	DrawDebugCircle(
		World,
		Center,
		OrbitTuning.TetherLengthCm,
		48,
		FColor(50, 60, 80),
		false,
		-1.0f,
		0,
		1.5f,
		Right,
		Up,
		false
	);

	// 2. Draw mace tether line from astronaut root to mace head
	const FVector MaceHeadWorld = Orbit->GetMaceHeadWorldPosition();
	DrawDebugLine(
		World,
		Center,
		MaceHeadWorld,
		FColor(120, 140, 170),
		false,
		-1.0f,
		0,
		2.0f
	);

	// 3. Draw mace head sphere with dynamic speed-shifted coloration (Cyan -> Violet)
	const float MaxSpeedRadPerSec = FMath::DegreesToRadians(FMath::Max(1.0f, OrbitTuning.MaxAngularSpeedDegPerSec));
	const float SpeedRatio = FMath::Clamp(FMath::Abs(OrbitState.AngularVelocityRadPerSec) / MaxSpeedRadPerSec, 0.0f, 1.0f);
	const FColor MaceHeadColor = FColor(
		static_cast<uint8>(FMath::Lerp(0.0f, 240.0f, SpeedRatio)),
		static_cast<uint8>(FMath::Lerp(220.0f, 40.0f, SpeedRatio)),
		255
	);

	DrawDebugSphere(
		World,
		MaceHeadWorld,
		14.0f + SpeedRatio * 4.0f,
		12,
		MaceHeadColor,
		false,
		-1.0f,
		0,
		1.5f
	);

	// 4. Draw instantaneous tangential velocity vector arrow
	const FVector TangentialVelocity = Orbit->GetTangentialVelocityWorld();
	if (!TangentialVelocity.IsNearlyZero())
	{
		DrawDebugDirectionalArrow(
			World,
			MaceHeadWorld,
			MaceHeadWorld + TangentialVelocity * 0.12f,
			16.0f,
			FColor(255, 220, 30),
			false,
			-1.0f,
			0,
			2.0f
		);
	}

	// 5. Draw instantaneous kick impulse preview arrow (where release will kick the astronaut right NOW)
	if (!FMath::IsNearlyZero(OrbitState.AngularVelocityRadPerSec))
	{
		const FVector DeltaVLocal = FTrajectoryPredictor::ComputeSteeringDeltaVelocity(
			OrbitState.OrbitAngleRad,
			OrbitState.AngularVelocityRadPerSec,
			OrbitTuning,
			TransferTuning
		);

		const FVector DeltaVWorld = Character->GetActorTransform().TransformVector(DeltaVLocal);
		if (!DeltaVWorld.IsNearlyZero())
		{
			DrawDebugDirectionalArrow(
				World,
				Center,
				Center + DeltaVWorld * 0.35f,
				24.0f,
				FColor(30, 255, 130),
				false,
				-1.0f,
				0,
				3.0f
			);
		}
	}

	// 6. Draw projected corridor flight path (1.5-second future horizon)
	TArray<FTrajectoryPoint> TrajectoryPoints;
	FTrajectoryPredictor::PredictTrajectoryCurve(
		Center,
		Forward,
		Right,
		Up,
		Drift->GetForwardSpeedCmPerSec(),
		Transfer->GetLateralVelocity(),
		TransferTuning.LateralDampingPerSec,
		1.5f,
		24,
		TrajectoryPoints
	);

	for (int32 Index = 0; Index < TrajectoryPoints.Num() - 1; ++Index)
	{
		DrawDebugLine(
			World,
			TrajectoryPoints[Index].Position,
			TrajectoryPoints[Index + 1].Position,
			FColor(0, 220, 255),
			false,
			-1.0f,
			0,
			2.5f
		);
	}
}

void FSpaceMentumDebugVisualizer::RenderOnScreenTelemetry(const ASpaceAstronautCharacter* Character)
{
	if (!Character || !GEngine)
	{
		return;
	}

	const UMaceOrbitComponent* Orbit = Character->GetMaceOrbit();
	const UMomentumTransferComponent* Transfer = Character->GetMomentumTransfer();
	const UForwardDriftComponent* Drift = Character->GetForwardDrift();

	if (!Orbit || !Transfer || !Drift)
	{
		return;
	}

	const FMaceOrbitState State = Orbit->GetOrbitState();
	const FMomentumTransferTuning TransferTuning = Transfer->GetTransferTuning();
	const FVector LateralVel = Transfer->GetLateralVelocity();

	const FString SpinInputStr =
		(State.HeldSpinDirection == EMaceSpinDirection::CounterClockwise) ? TEXT("HOLDING A [Counter-Clockwise]") :
		(State.HeldSpinDirection == EMaceSpinDirection::Clockwise) ? TEXT("HOLDING D [Clockwise]") :
		TEXT("NONE (Release Kick Armed)");

	const FString KickModelStr =
		(TransferTuning.KickModel == EMomentumKickModel::ReactionRecoil) ?
		TEXT("ReactionRecoil (Opposite Mace Velocity)") :
		TEXT("CarryAlong (Yank Along Mace Velocity)");

	const FColor InputColor = (State.HeldSpinDirection != EMaceSpinDirection::None) ? FColor::Yellow : FColor::Green;

	GEngine->AddOnScreenDebugMessage(1101, 0.0f, InputColor, FString::Printf(TEXT("[SpaceMentum Input]       %s"), *SpinInputStr));
	GEngine->AddOnScreenDebugMessage(1102, 0.0f, FColor::Cyan, FString::Printf(TEXT("[Mace Angle / Cadence]   %.1f deg | %.1f RPM (%.0f deg/s)"),
		FMath::RadiansToDegrees(State.OrbitAngleRad),
		Orbit->GetCurrentRPM(),
		FMath::RadiansToDegrees(FMath::Abs(State.AngularVelocityRadPerSec))));
	GEngine->AddOnScreenDebugMessage(1103, 0.0f, FColor::Emerald, FString::Printf(TEXT("[Lateral Velocity YZ]    Y: %+.1f cm/s | Z: %+.1f cm/s (|v|: %.1f cm/s)"),
		LateralVel.Y, LateralVel.Z, LateralVel.Size()));
	GEngine->AddOnScreenDebugMessage(1104, 0.0f, FColor::White, FString::Printf(TEXT("[Forward Speed +X]       %.0f cm/s (%.1f km/h)"),
		Drift->GetForwardSpeedCmPerSec(),
		(Drift->GetForwardSpeedCmPerSec() * 0.036f)));
	GEngine->AddOnScreenDebugMessage(1105, 0.0f, FColor::Orange, FString::Printf(TEXT("[Kick Model (Press 'M')] %s"), *KickModelStr));
}
