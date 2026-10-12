/**
 * @file SpaceAstronautCharacter.cpp
 * @brief Builds the astronaut's component hierarchy, wires physics delegates, and renders real-time telemetry.
 *
 * @emotional_intent Small astronaut, heavy mace, a camera that lets you read the arc and feel the kick.
 */

#include "Player/SpaceAstronautCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AbilitySystemBridgeComponent.h"
#include "Components/ForwardDriftComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Data/MaceDefinitionDataAsset.h"
#include "Debug/SpaceMentumDebugVisualizer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

namespace SpaceAstronautDefaults
{
	/** Boom length (cm): far enough to frame the full 160 cm mace arc around the astronaut. */
	constexpr float ChaseArmLengthCm = 650.0f;

	/** Downward pitch (deg): "slightly elevated above the astronaut, tilted down toward the forward horizon". */
	constexpr float ChasePitchDeg = -12.0f;

	/** Lift (cm) so the astronaut sits in the lower-middle of the frame and the tunnel ahead stays clear. */
	constexpr float ChaseSocketLiftCm = 120.0f;

	/** Positional lag: elastic enough to feel the kick, tight enough to never lose the astronaut. */
	constexpr float CameraLagSpeed = 8.0f;
	constexpr float CameraRotationLagSpeed = 10.0f;

	/** Resting FOV (deg). */
	constexpr float BaseFieldOfViewDeg = 80.0f;
}

ASpaceAstronautCharacter::ASpaceAstronautCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// View is locked down-tunnel: no controller pitch/yaw/roll inheritance
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// --- Camera rig (GDD §3.1.1) ---
	ChaseSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseSpringArm"));
	ChaseSpringArm->SetupAttachment(GetRootComponent());
	ChaseSpringArm->TargetArmLength = SpaceAstronautDefaults::ChaseArmLengthCm;
	ChaseSpringArm->SetRelativeRotation(FRotator(SpaceAstronautDefaults::ChasePitchDeg, 0.0f, 0.0f));
	ChaseSpringArm->SocketOffset = FVector(0.0f, 0.0f, SpaceAstronautDefaults::ChaseSocketLiftCm);
	ChaseSpringArm->bUsePawnControlRotation = false;
	ChaseSpringArm->bDoCollisionTest = false; // Never let debris snap camera in front of astronaut
	ChaseSpringArm->bEnableCameraLag = true;
	ChaseSpringArm->bEnableCameraRotationLag = true;
	ChaseSpringArm->CameraLagSpeed = SpaceAstronautDefaults::CameraLagSpeed;
	ChaseSpringArm->CameraRotationLagSpeed = SpaceAstronautDefaults::CameraRotationLagSpeed;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(ChaseSpringArm, USpringArmComponent::SocketName);
	ChaseCamera->bUsePawnControlRotation = false;
	ChaseCamera->SetFieldOfView(SpaceAstronautDefaults::BaseFieldOfViewDeg);

	// --- Physics feel ---
	ForwardDrift = CreateDefaultSubobject<UForwardDriftComponent>(TEXT("ForwardDrift"));
	MaceOrbit = CreateDefaultSubobject<UMaceOrbitComponent>(TEXT("MaceOrbit"));
	MomentumTransfer = CreateDefaultSubobject<UMomentumTransferComponent>(TEXT("MomentumTransfer"));

	// --- Abilities & Flavour ---
	AbilityBridge = CreateDefaultSubobject<UAbilitySystemBridgeComponent>(TEXT("AbilityBridge"));

	// Configure character movement for space flying
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Flying);
		Movement->GravityScale = 0.0f;
		Movement->BrakingDecelerationFlying = 0.0f;
		Movement->AirControl = 0.0f;
		Movement->MaxFlySpeed = 50000.0f;
	}
}

void ASpaceAstronautCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Wire MaceOrbit release event to MomentumTransfer execution
	if (MaceOrbit)
	{
		MaceOrbit->OnMaceOrbitReleased.AddDynamic(this, &ASpaceAstronautCharacter::HandleMaceOrbitReleased);
	}

	// Wire MomentumTransfer kick event to camera impulse reaction
	if (MomentumTransfer)
	{
		MomentumTransfer->OnMomentumKickFired.AddDynamic(this, &ASpaceAstronautCharacter::HandleMomentumKickFired);
	}

	// Push default mace definition into components if authored
	if (EquippedMace)
	{
		EquipMaceDefinition(EquippedMace);
	}
}

void ASpaceAstronautCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Dynamic camera FOV relaxation: smooth return to resting 80 deg
	TargetFieldOfView = FMath::FInterpTo(TargetFieldOfView, SpaceAstronautDefaults::BaseFieldOfViewDeg, DeltaSeconds, 2.5f);
	CurrentFieldOfView = FMath::FInterpTo(CurrentFieldOfView, TargetFieldOfView, DeltaSeconds, 12.0f);
	if (ChaseCamera)
	{
		ChaseCamera->SetFieldOfView(CurrentFieldOfView);
	}

	// Render real-time spatial debug gizmos
	if (bEnableDebugVisualization)
	{
		FSpaceMentumDebugVisualizer::DrawCharacterDebugGizmos(this, this, DeltaSeconds);
	}

	// Render on-screen HUD telemetry
	if (bRenderOnScreenTelemetry)
	{
		FSpaceMentumDebugVisualizer::RenderOnScreenTelemetry(this);
	}
}

void ASpaceAstronautCharacter::EquipMaceDefinition(UMaceDefinitionDataAsset* NewMaceDefinition)
{
	if (!NewMaceDefinition)
	{
		return;
	}

	EquippedMace = NewMaceDefinition;

	if (MaceOrbit)
	{
		MaceOrbit->SetOrbitTuning(NewMaceDefinition->OrbitTuning);
	}

	if (MomentumTransfer)
	{
		MomentumTransfer->SetTransferTuning(NewMaceDefinition->TransferTuning);
	}
}

void ASpaceAstronautCharacter::HandleMaceOrbitReleased(
	float OrbitAngleRad,
	float AngularVelocityRadPerSec,
	float TetherLengthCm,
	float MaceMassKg)
{
	if (MomentumTransfer)
	{
		MomentumTransfer->ExecuteMomentumTransfer(
			OrbitAngleRad,
			AngularVelocityRadPerSec,
			TetherLengthCm,
			MaceMassKg
		);
	}
}

void ASpaceAstronautCharacter::HandleMomentumKickFired(
	FVector ImpartedImpulse,
	FVector ResultingLateralVelocity,
	EMomentumKickModel KickModel)
{
	// Visceral camera FOV kick pulse: widen FOV briefly on violent evasion kicks
	const float KickSpeed = ResultingLateralVelocity.Size();
	const float KickRatio = FMath::Clamp(KickSpeed / 1800.0f, 0.0f, 1.0f);
	TargetFieldOfView = SpaceAstronautDefaults::BaseFieldOfViewDeg + (KickRatio * 16.0f);
}
