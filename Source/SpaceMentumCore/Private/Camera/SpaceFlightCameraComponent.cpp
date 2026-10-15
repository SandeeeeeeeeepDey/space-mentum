// ==============================================================================
// SpaceFlightCameraComponent.cpp — Cinematic dynamic flight camera rig.
//
// @creator_feel
//   "a space person zooming in the forward direction in a uniform velocity and
//    the person has a mace in his hand."
//
// @emotional_intent
//   Smooth, high-impact chase camera with dynamic FOV, Dutch roll, and trauma response.
// ==============================================================================

#include "Camera/SpaceFlightCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Player/SpaceAstronautCharacter.h"
#include "GameFramework/Pawn.h"

USpaceFlightCameraComponent::USpaceFlightCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // Run after physics has resolved pawn velocity
}

void USpaceFlightCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningAstronaut = Cast<ASpaceAstronautCharacter>(GetOwner());
	CurrentFov = CameraTuning.BaseFovDeg;
	TargetFov = CameraTuning.BaseFovDeg;
	CurrentArmLength = CameraTuning.BaseArmLengthCm;
	CurrentRollDeg = 0.0f;
}

void USpaceFlightCameraComponent::InitializeCameraRig(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArmComp = InSpringArm;
	CameraComp = InCamera;

	if (CameraComp)
	{
		CameraComp->SetFieldOfView(CurrentFov);
	}

	if (SpringArmComp)
	{
		SpringArmComp->TargetArmLength = CurrentArmLength;
	}
}

void USpaceFlightCameraComponent::TriggerFovPunch(float PunchMagnitudeRatio)
{
	const float ClampedRatio = FMath::Clamp(PunchMagnitudeRatio, 0.0f, 1.5f);
	FovPunchOffset = FMath::Min(CameraTuning.KickFovPunchDeg * 1.5f, FovPunchOffset + (ClampedRatio * CameraTuning.KickFovPunchDeg));
}

void USpaceFlightCameraComponent::AddTrauma(float InTrauma)
{
	Trauma = FMath::Clamp(Trauma + InTrauma, 0.0f, 1.0f);
}

void USpaceFlightCameraComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	const FVector PawnVelocity = OwnerPawn->GetVelocity();
	const FVector ForwardVector = OwnerPawn->GetActorForwardVector();
	const FVector RightVector = OwnerPawn->GetActorRightVector();

	const float ForwardSpeed = FVector::DotProduct(PawnVelocity, ForwardVector);
	const float LateralRightSpeed = FVector::DotProduct(PawnVelocity, RightVector);

	// 1. Dynamic Speed-Reactive FOV Dilation
	const float SpeedRatio = FMath::Clamp(
		ForwardSpeed / FMath::Max(100.0f, CameraTuning.ReferenceHyperspaceSpeedCmPerSec),
		0.0f,
		1.5f
	);

	// Exponential decay of kick FOV punch offset
	FovPunchOffset = FMath::FInterpTo(FovPunchOffset, 0.0f, DeltaTime, CameraTuning.FovInterpSpeed * 1.2f);

	TargetFov = CameraTuning.BaseFovDeg + (SpeedRatio * CameraTuning.MaxSpeedFovDilationDeg) + FovPunchOffset;
	TargetFov = FMath::Clamp(TargetFov, 60.0f, 120.0f);
	CurrentFov = FMath::FInterpTo(CurrentFov, TargetFov, DeltaTime, CameraTuning.FovInterpSpeed);

	if (CameraComp)
	{
		CameraComp->SetFieldOfView(CurrentFov);
	}

	// 2. Dynamic Spring Arm Length Dilation (Pull-back at high velocity)
	const float TargetArmLength = CameraTuning.BaseArmLengthCm + (SpeedRatio * CameraTuning.MaxSpeedArmLengthDilationCm);
	CurrentArmLength = FMath::FInterpTo(CurrentArmLength, TargetArmLength, DeltaTime, CameraTuning.ArmLengthInterpSpeed);

	if (SpringArmComp)
	{
		SpringArmComp->TargetArmLength = CurrentArmLength;
	}

	// 3. Dynamic Dutch Roll (Camera banking during violent lateral momentum evasion kicks)
	const float LateralRatio = FMath::Clamp(
		LateralRightSpeed / FMath::Max(100.0f, CameraTuning.ReferenceLateralRollSpeedCmPerSec),
		-1.0f,
		1.0f
	);
	const float TargetRoll = -LateralRatio * CameraTuning.MaxDutchRollDeg;
	CurrentRollDeg = FMath::FInterpTo(CurrentRollDeg, TargetRoll, DeltaTime, CameraTuning.RollInterpSpeed);

	// 4. Kinetic Trauma Screen Shake ($Shake = Trauma^2$)
	FRotator ShakeRotation = FRotator::ZeroRotator;
	if (Trauma > 0.001f)
	{
		TraumaTime += DeltaTime * 25.0f;
		const float ShakeMagnitude = Trauma * Trauma * CameraTuning.MaxTraumaShakeDeg;

		ShakeRotation.Pitch = FMath::Sin(TraumaTime * 1.3f) * ShakeMagnitude;
		ShakeRotation.Yaw = FMath::Cos(TraumaTime * 1.7f) * ShakeMagnitude * 0.7f;
		ShakeRotation.Roll = FMath::Sin(TraumaTime * 2.1f) * ShakeMagnitude * 0.5f;

		Trauma = FMath::FInterpTo(Trauma, 0.0f, DeltaTime, CameraTuning.TraumaDecayRate);
	}

	if (SpringArmComp)
	{
		// Apply base spring arm angle (-10 deg pitch) with dynamic roll and trauma shake
		const FRotator BaseBoomRotation(-10.0f, 0.0f, CurrentRollDeg);
		SpringArmComp->SetRelativeRotation(BaseBoomRotation + ShakeRotation);
	}
}
