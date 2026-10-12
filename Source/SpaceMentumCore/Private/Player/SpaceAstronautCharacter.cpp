/**
 * @file SpaceAstronautCharacter.cpp
 * @brief Builds the astronaut's component hierarchy and the TPP chase camera rig.
 *
 * @emotional_intent See header. Small astronaut, heavy mace, a camera that lets you read the arc.
 */

#include "Player/SpaceAstronautCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AbilitySystemBridgeComponent.h"
#include "Components/ForwardDriftComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Data/MaceDefinitionDataAsset.h"
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

	/** Resting FOV (deg). The dynamic 80 -> 105 shift arrives with the FOV shifter in Step 5. */
	constexpr float BaseFieldOfViewDeg = 80.0f;
}

ASpaceAstronautCharacter::ASpaceAstronautCharacter()
{
	// Logic lives in components; the character itself never needs to tick.
	PrimaryActorTick.bCanEverTick = false;

	// The camera must not inherit controller rotation: in SpaceMentum, looking around is
	// not a thing — the view is locked down-tunnel so all attention stays on the mace.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// --- Camera rig ---
	ChaseSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseSpringArm"));
	ChaseSpringArm->SetupAttachment(GetRootComponent());
	ChaseSpringArm->TargetArmLength = SpaceAstronautDefaults::ChaseArmLengthCm;
	ChaseSpringArm->SetRelativeRotation(FRotator(SpaceAstronautDefaults::ChasePitchDeg, 0.0f, 0.0f));
	ChaseSpringArm->SocketOffset = FVector(0.0f, 0.0f, SpaceAstronautDefaults::ChaseSocketLiftCm);
	ChaseSpringArm->bUsePawnControlRotation = false;
	ChaseSpringArm->bDoCollisionTest = false; // Never let debris yank the camera in front of the player.
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

	// --- Flavour ---
	AbilityBridge = CreateDefaultSubobject<UAbilitySystemBridgeComponent>(TEXT("AbilityBridge"));
}
