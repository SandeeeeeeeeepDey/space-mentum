/**
 * @file MomentumTransferComponent.cpp
 * @brief Momentum transfer component implementation — whiplash kick calculation and exponential damping.
 *
 * @emotional_intent Crisp, punchy, instantaneous whiplash.
 */

#include "Components/MomentumTransferComponent.h"
#include "Solvers/TrajectoryPredictor.h"

UMomentumTransferComponent::UMomentumTransferComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMomentumTransferComponent::BeginPlay()
{
	Super::BeginPlay();
	LateralVelocity = FVector::ZeroVector;
}

void UMomentumTransferComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f || LateralVelocity.IsNearlyZero())
	{
		return;
	}

	// Exact framerate-independent exponential decay: v(t) = v(0) * e^(-lambda * dt)
	if (TransferTuning.LateralDampingPerSec > 0.0f)
	{
		const float DecayFactor = FMath::Exp(-TransferTuning.LateralDampingPerSec * DeltaTime);
		LateralVelocity *= DecayFactor;
	}

	// Forward X is sacred: lateral velocity exists strictly in local YZ
	LateralVelocity.X = 0.0f;

	// Snap subnormal residual velocities to clean zero
	if (LateralVelocity.SizeSquared() < 0.01f)
	{
		LateralVelocity = FVector::ZeroVector;
	}
}

void UMomentumTransferComponent::ExecuteMomentumTransfer(
	float ReleaseAngleRad,
	float ReleaseAngularVelocityRadPerSec,
	float TetherLengthCm,
	float MaceMassKg)
{
	// Forgive accidental taps below angular speed threshold
	const float AngularSpeedDegPerSec = FMath::RadiansToDegrees(FMath::Abs(ReleaseAngularVelocityRadPerSec));
	if (AngularSpeedDegPerSec < TransferTuning.MinAngularSpeedForKickDegPerSec)
	{
		return;
	}

	// Compute instantaneous kinetic impulse vector J using pure stateless solver
	const FVector Impulse = FTrajectoryPredictor::ComputeSteeringImpulse(
		ReleaseAngleRad,
		ReleaseAngularVelocityRadPerSec,
		TetherLengthCm,
		MaceMassKg,
		TransferTuning.ImpulseEfficiencyAlpha,
		TransferTuning.KickModel
	);

	// Delta V = J / m_astronaut
	const float SafeAstronautMassKg = FMath::Max(1.0f, TransferTuning.AstronautMassKg);
	const FVector DeltaV = Impulse / SafeAstronautMassKg;

	// Apply delta velocity instantaneously to lateral Y and Z
	LateralVelocity.Y += DeltaV.Y;
	LateralVelocity.Z += DeltaV.Z;
	LateralVelocity.X = 0.0f;

	// Hard clamp to prevent runaway speed accumulation
	LateralVelocity = LateralVelocity.GetClampedToMaxSize(TransferTuning.MaxLateralSpeedCmPerSec);

	// Dispatch event for camera punch, sound effects, and particle bursts
	if (OnMomentumKickFired.IsBound())
	{
		OnMomentumKickFired.Broadcast(Impulse, LateralVelocity, TransferTuning.KickModel);
	}
}

void UMomentumTransferComponent::SetTransferTuning(const FMomentumTransferTuning& InTuning)
{
	TransferTuning = InTuning;
}

void UMomentumTransferComponent::ToggleKickModel()
{
	TransferTuning.KickModel = (TransferTuning.KickModel == EMomentumKickModel::ReactionRecoil)
		? EMomentumKickModel::CarryAlong
		: EMomentumKickModel::ReactionRecoil;
}

void UMomentumTransferComponent::SetKickModel(EMomentumKickModel NewModel)
{
	TransferTuning.KickModel = NewModel;
}

FVector UMomentumTransferComponent::GetLateralVelocityWorld() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return LateralVelocity;
	}

	return OwnerActor->GetActorTransform().TransformVector(LateralVelocity);
}

void UMomentumTransferComponent::ResetLateralVelocity()
{
	LateralVelocity = FVector::ZeroVector;
}
