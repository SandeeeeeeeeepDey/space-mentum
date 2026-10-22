/**
 * @file SpaceAstronautCharacter.cpp
 * @brief Builds the astronaut's component hierarchy, wires physics delegates, and renders real-time telemetry.
 *
 * @emotional_intent Small astronaut, heavy mace, a camera that lets you read the arc and feel the kick.
 */

#include "Player/SpaceAstronautCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AbilitySystemBridgeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ForwardDriftComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Data/MaceDefinitionDataAsset.h"
#include "Debug/SpaceMentumDebugVisualizer.h"
#include "Environment/SpaceObstacleActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/SpaceGameState.h"
#include "UI/SpaceFlightInstrumentationComponent.h"
#include "Camera/SpaceFlightCameraComponent.h"
#include "VFX/SpaceFlightVFXComponent.h"
#include "Audio/SpaceProceduralFlightAudioComponent.h"
#include "Subsystems/SpaceSectorProgressionSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/SpaceKineticManipulatorComponent.h"

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

	// --- Flight Telemetry & Radar Instrumentation (Rule 4) ---
	FlightInstrumentation = CreateDefaultSubobject<USpaceFlightInstrumentationComponent>(TEXT("FlightInstrumentation"));

	// --- Step 5 Camera, VFX & Dynamic Audio ---
	FlightCamera = CreateDefaultSubobject<USpaceFlightCameraComponent>(TEXT("FlightCamera"));
	FlightVFX = CreateDefaultSubobject<USpaceFlightVFXComponent>(TEXT("FlightVFX"));
	FlightAudio = CreateDefaultSubobject<USpaceProceduralFlightAudioComponent>(TEXT("FlightAudio"));
	KineticManipulator = CreateDefaultSubobject<USpaceKineticManipulatorComponent>(TEXT("KineticManipulator"));

	// --- Step 7 Mechanical Arm Rigging, Mace Head & Physics Constraints ---
	MechanicalArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MechanicalArmMesh"));
	MechanicalArmMesh->SetupAttachment(GetMesh(), FName(TEXT("Socket_AppendageMount")));
	MechanicalArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MaceHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MaceHeadMesh"));
	MaceHeadMesh->SetupAttachment(GetRootComponent());
	MaceHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ArmPhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("ArmPhysicsConstraint"));
	ArmPhysicsConstraint->SetupAttachment(GetMesh());

	// Default Step 7 High-Fidelity PBR Material Assets
	SuitBaseMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Environment/Materials/M_Astronaut_Suit_PBR.M_Astronaut_Suit_PBR")));
	ArmBaseMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Environment/Materials/M_MechanicalArm_HardSurface.M_MechanicalArm_HardSurface")));
	MaceBaseMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Environment/Materials/M_MaceHead_Heavy.M_MaceHead_Heavy")));

	// Configure character movement for space flying
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Flying);
		Movement->GravityScale = 0.0f;
		Movement->BrakingDecelerationFlying = 0.0f;
		Movement->AirControl = 0.0f;
		Movement->MaxFlySpeed = 50000.0f;
	}

	// Configure capsule component with Continuous Collision Detection (CCD)
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetUseCCD(true);
		Capsule->SetNotifyRigidBodyCollision(true);
		Capsule->OnComponentHit.AddDynamic(this, &ASpaceAstronautCharacter::HandleCapsuleHit);
	}
}

void ASpaceAstronautCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize previous mace head position
	if (MaceOrbit)
	{
		PreviousMaceHeadLocation = MaceOrbit->GetMaceHeadWorldPosition();
	}
	else
	{
		PreviousMaceHeadLocation = GetActorLocation();
	}

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

	// Initialize Step 5 Camera, VFX, and Dynamic Audio systems
	if (FlightCamera)
	{
		FlightCamera->InitializeCameraRig(ChaseSpringArm, ChaseCamera);
	}

	if (FlightVFX)
	{
		FlightVFX->InitializeVFX(MaceOrbit, GetRootComponent(), GetRootComponent());
	}

	if (FlightAudio)
	{
		FlightAudio->InitializeFlightAudio(MaceOrbit);
	}

	// Initialize Step 7 Dynamic Material Instances for live shader telemetry & glow
	if (GetMesh())
	{
		SuitDynamicMaterial = GetMesh()->CreateDynamicMaterialInstance(0, SuitBaseMaterial.LoadSynchronous());
	}
	if (MechanicalArmMesh)
	{
		ArmDynamicMaterial = MechanicalArmMesh->CreateDynamicMaterialInstance(0, ArmBaseMaterial.LoadSynchronous());
	}
	if (MaceHeadMesh)
	{
		MaceDynamicMaterial = MaceHeadMesh->CreateDynamicMaterialInstance(0, MaceBaseMaterial.LoadSynchronous());
	}
}

void ASpaceAstronautCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Handle Launch Push-Off state machine (§1.4)
	UWorld* World = GetWorld();
	const ASpaceGameState* GameState = World ? World->GetGameState<ASpaceGameState>() : nullptr;
	const bool bInLaunchPhase = GameState && (GameState->GetCurrentRunPhase() == ESpaceRunPhase::LaunchPushOff);

	if (bInLaunchPhase)
	{
		if (bIsChargingLaunchPush)
		{
			CurrentPushChargeRatio = FMath::Clamp(
				CurrentPushChargeRatio + (DeltaSeconds * LaunchParameters.ChargeRatePerSec),
				0.0f,
				1.0f
			);

			// Tactile camera compression effect during pneumatic ram charge
			TargetFieldOfView = SpaceAstronautDefaults::BaseFieldOfViewDeg - (CurrentPushChargeRatio * 6.0f);
		}
	}

	// Dynamic camera FOV relaxation: smooth return to resting 80 deg
	TargetFieldOfView = FMath::FInterpTo(TargetFieldOfView, SpaceAstronautDefaults::BaseFieldOfViewDeg, DeltaSeconds, 2.5f);
	CurrentFieldOfView = FMath::FInterpTo(CurrentFieldOfView, TargetFieldOfView, DeltaSeconds, 12.0f);
	if (ChaseCamera)
	{
		ChaseCamera->SetFieldOfView(CurrentFieldOfView);
	}

	// Apply internal two-body counter-displacement and counter-rotation to mesh (The Dodge)
	if (MaceOrbit && GetMesh())
	{
		const FCenterOfMassDisplacementState CoMState = MaceOrbit->GetCoMDisplacementState();
		GetMesh()->SetRelativeLocation(CoMState.AstronautDisplacement);
		GetMesh()->SetRelativeRotation(FRotator(0.0f, 0.0f, FMath::RadiansToDegrees(CoMState.CounterRotationAngleRad)));
	}

	// Continuous collision sweep for the entire mechanical appendage against environmental obstacles
	PerformAppendageCollisionSweep(DeltaSeconds);

	// Enforce Rule 4 ("Blind Math"): zero predictive trajectory arcs rendered in normal gameplay
	if (bEnableDebugVisualization && !bStrictBlindMathEnforced)
	{
		FSpaceMentumDebugVisualizer::DrawCharacterDebugGizmos(this, this, DeltaSeconds);
	}

	if (bRenderOnScreenTelemetry && !bStrictBlindMathEnforced)
	{
		FSpaceMentumDebugVisualizer::RenderOnScreenTelemetry(this);
	}

	// Update Step 7 Mechanical Arm and Mace Head 3D transforms
	if (MaceOrbit && MaceHeadMesh)
	{
		const FVector HeadPos = MaceOrbit->GetMaceHeadWorldPosition();
		MaceHeadMesh->SetWorldLocation(HeadPos);
		const FRotator HeadRot(0.0f, 0.0f, FMath::RadiansToDegrees(MaceOrbit->GetOrbitAngleRad()));
		MaceHeadMesh->SetWorldRotation(HeadRot);
	}

	if (MechanicalArmMesh && MaceOrbit)
	{
		const FVector SocketPos = GetMesh() ? GetMesh()->GetSocketLocation(FName(TEXT("Socket_AppendageMount"))) : GetActorLocation();
		const FVector TargetHead = MaceOrbit->GetMaceHeadWorldPosition();
		const FVector ArmDir = TargetHead - SocketPos;
		const float ArmLen = ArmDir.Size();
		if (ArmLen > 1.0f)
		{
			MechanicalArmMesh->SetWorldLocation(SocketPos);
			MechanicalArmMesh->SetWorldRotation(ArmDir.Rotation());
			MechanicalArmMesh->SetWorldScale3D(FVector(ArmLen / 100.0f, 1.0f, 1.0f));
		}
	}

	// Update Step 7 High-Fidelity PBR Dynamic Materials
	UpdateHighFidelityDynamicMaterials(DeltaSeconds);
}

void ASpaceAstronautCharacter::PerformAppendageCollisionSweep(float DeltaSeconds)
{
	if (!MaceOrbit || DeltaSeconds <= 0.0f)
	{
		return;
	}

	const FVector CurrentMaceLocation = MaceOrbit->GetMaceHeadWorldPosition();
	const FVector CurrentAstronautLocation = GetActorLocation();

	if (PreviousMaceHeadLocation.IsZero())
	{
		PreviousMaceHeadLocation = CurrentMaceLocation;
		PreviousAstronautLocation = CurrentAstronautLocation;
		return;
	}

	const FVector TangentialVelocity = MaceOrbit->GetTangentialVelocityWorld();
	const FVector BodyVelocity = GetVelocity();

	UWorld* World = GetWorld();
	if (World)
	{
		FCollisionQueryParams QueryParams(TEXT("AppendageSweep"), false, this);

		// 1. Kinetic Ram Head Sweep (continuous sweep between previous and current head positions)
		TArray<FHitResult> HeadHitResults;
		const FCollisionShape HeadShape = FCollisionShape::MakeSphere(40.0f);
		const bool bHeadHit = World->SweepMultiByChannel(
			HeadHitResults,
			PreviousMaceHeadLocation,
			CurrentMaceLocation,
			FQuat::Identity,
			ECC_WorldDynamic,
			HeadShape,
			QueryParams
		);

		bool bResolvedDeflection = false;

		if (bHeadHit)
		{
			for (const FHitResult& Hit : HeadHitResults)
			{
				if (ASpaceObstacleActor* Obstacle = Cast<ASpaceObstacleActor>(Hit.GetActor()))
				{
					const float MaceMass = MaceOrbit->GetOrbitTuning().MaceMassKg;
					const float TotalMass = MaceMass + 160.0f;
					const FVector HeadVelocity = TangentialVelocity + BodyVelocity;

					Obstacle->ProcessMaceStrike(
						HeadVelocity,
						Hit.ImpactPoint,
						Hit.ImpactNormal,
						MaceMass,
						TotalMass,
						this
					);

					if (FlightVFX)
					{
						FlightVFX->TriggerKineticImpactBurst(Hit.ImpactPoint, Hit.ImpactNormal, 1.0f);
					}

					if (FlightAudio)
					{
						FlightAudio->PlayObstacleStrikeClang(1.0f);
					}

					if (FlightCamera)
					{
						FlightCamera->AddTrauma(0.35f);
					}

					LastAppendageDeflectionTime = World->GetTimeSeconds();
					bResolvedDeflection = true;
					break;
				}
			}
		}

		// 2. Telescoping Arm Segment Sweep: multi-sphere sweep along the arm shaft (astronaut socket to mace head)
		// Allows pushing and deflecting obstacles with any point of the extended appendage
		if (!bResolvedDeflection)
		{
			constexpr int32 NumArmSegments = 4;
			for (int32 i = 1; i <= NumArmSegments; ++i)
			{
				const float U = static_cast<float>(i) / static_cast<float>(NumArmSegments + 1);
				const FVector ArmSampleStart = FMath::Lerp(PreviousAstronautLocation, PreviousMaceHeadLocation, U);
				const FVector ArmSampleEnd = FMath::Lerp(CurrentAstronautLocation, CurrentMaceLocation, U);
				const float SegmentRadius = FMath::Lerp(18.0f, 32.0f, U);

				TArray<FHitResult> ArmHitResults;
				const FCollisionShape ArmShape = FCollisionShape::MakeSphere(SegmentRadius);

				if (World->SweepMultiByChannel(ArmHitResults, ArmSampleStart, ArmSampleEnd, FQuat::Identity, ECC_WorldDynamic, ArmShape, QueryParams))
				{
					for (const FHitResult& Hit : ArmHitResults)
					{
						if (ASpaceObstacleActor* Obstacle = Cast<ASpaceObstacleActor>(Hit.GetActor()))
						{
							const float MaceMass = MaceOrbit->GetOrbitTuning().MaceMassKg;
							const float TotalMass = MaceMass + 160.0f;
							const FVector ArmPointVelocity = (TangentialVelocity * U) + BodyVelocity;

							Obstacle->ProcessMaceStrike(
								ArmPointVelocity,
								Hit.ImpactPoint,
								Hit.ImpactNormal,
								MaceMass,
								TotalMass,
								this
							);

							if (FlightVFX)
							{
								FlightVFX->TriggerKineticImpactBurst(Hit.ImpactPoint, Hit.ImpactNormal, 0.7f);
							}

							if (FlightAudio)
							{
								FlightAudio->PlayObstacleStrikeClang(0.7f);
							}

							if (FlightCamera)
							{
								FlightCamera->AddTrauma(0.25f);
							}

							LastAppendageDeflectionTime = World->GetTimeSeconds();
							bResolvedDeflection = true;

							if (UGameInstance* GI = GetGameInstance())
							{
								if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
								{
									Progression->RecordExternalStrikeLanded(1.0f, Obstacle->GetName());
								}
							}
							break;
						}
					}
				}

				if (bResolvedDeflection)
				{
					break;
				}
			}
		}
	}

	PreviousMaceHeadLocation = CurrentMaceLocation;
	PreviousAstronautLocation = CurrentAstronautLocation;
}

void ASpaceAstronautCharacter::HandleCapsuleHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (ASpaceObstacleActor* Obstacle = Cast<ASpaceObstacleActor>(OtherActor))
	{
		// Deflection grace period: If the obstacle was just struck/deflected by the appendage in the last 150ms,
		// do not treat glancing astronaut capsule touch as an instant fatal collision!
		if (UWorld* World = GetWorld())
		{
			if (LastAppendageDeflectionTime > 0.0f && (World->GetTimeSeconds() - LastAppendageDeflectionTime) < 0.15f)
			{
				return;
			}
		}

		const bool bIsFatal = Obstacle->ProcessAstronautBodyCollision(GetVelocity(), this);
		if (FlightCamera)
		{
			FlightCamera->AddTrauma(bIsFatal ? 1.0f : 0.4f);
		}

		if (bIsFatal)
		{
			if (UWorld* World = GetWorld())
			{
				if (ASpaceGameState* GameState = World->GetGameState<ASpaceGameState>())
				{
					GameState->SetRunPhase(ESpaceRunPhase::CrashedKineticImpact);
				}

				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
					{
						Progression->TerminateActiveSectorRunFailed(false);
					}
				}
			}
		}
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

	if (FlightCamera)
	{
		FlightCamera->TriggerFovPunch(KickRatio);
	}

	if (FlightAudio)
	{
		FlightAudio->PlayMomentumKickSnap(KickRatio);
	}
}

void ASpaceAstronautCharacter::StartChargingLaunchPush()
{
	bIsChargingLaunchPush = true;
}

void ASpaceAstronautCharacter::ReleaseLaunchPushAndFire()
{
	if (!bIsChargingLaunchPush)
	{
		return;
	}

	bIsChargingLaunchPush = false;
	const float FinalCharge = FMath::Clamp(CurrentPushChargeRatio, 0.15f, 1.0f);
	CurrentPushChargeRatio = 0.0f;

	const FVector AimDirection = GetLaunchAimDirection();
	const float DepartureSpeed = FinalCharge * LaunchParameters.MaxLaunchSpeedCmPerSec;
	const FVector LaunchVelocity = AimDirection * DepartureSpeed;

	if (ForwardDrift)
	{
		ForwardDrift->SetForwardSpeed(LaunchVelocity.X);
	}

	if (MomentumTransfer)
	{
		MomentumTransfer->SetCoMVelocity(LaunchVelocity);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->Velocity = LaunchVelocity;
	}

	// Explosive departure kick pulse to camera FOV & trauma
	TargetFieldOfView = FMath::Min(105.0f, SpaceAstronautDefaults::BaseFieldOfViewDeg + (FinalCharge * 18.0f));

	if (FlightCamera)
	{
		FlightCamera->TriggerFovPunch(FinalCharge * 1.5f);
		FlightCamera->AddTrauma(FinalCharge * 0.4f);
	}

	if (FlightVFX)
	{
		FlightVFX->TriggerLaunchExhaustBurst(GetActorLocation(), AimDirection, FinalCharge);
	}

	if (FlightAudio)
	{
		FlightAudio->PlayLaunchSubBassThump(FinalCharge);
	}

	UWorld* World = GetWorld();
	if (World)
	{
		if (ASpaceGameState* GameState = World->GetGameState<ASpaceGameState>())
		{
			GameState->SetRunPhase(ESpaceRunPhase::Running);
		}
	}
}

void ASpaceAstronautCharacter::AdjustLaunchAim(float DeltaDegrees)
{
	LaunchAimAngleDeg = FMath::Clamp(
		LaunchAimAngleDeg + DeltaDegrees,
		-LaunchParameters.MaxDepartureDeflectionAngleDeg,
		LaunchParameters.MaxDepartureDeflectionAngleDeg
	);

	SetActorRotation(FRotator(0.0f, LaunchAimAngleDeg, 0.0f));
}

FVector ASpaceAstronautCharacter::GetLaunchAimDirection() const
{
	const FRotator AimRotator(0.0f, LaunchAimAngleDeg, 0.0f);
	return AimRotator.RotateVector(FVector::ForwardVector).GetSafeNormal();
}

void ASpaceAstronautCharacter::UpdateHighFidelityDynamicMaterials(float DeltaSeconds)
{
	if (!MaceOrbit)
	{
		return;
	}

	const float AbsOmega = FMath::Abs(MaceOrbit->GetAngularVelocityRadPerSec());
	const float MaxOmega = FMath::Max(1.0f, MaceOrbit->GetOrbitTuning().MaxAngularSpeedRadPerSec);
	const float NormalizedOmega = FMath::Clamp(AbsOmega / MaxOmega, 0.0f, 1.0f);

	const FVector Velocity = GetVelocity();
	const float LateralSpeed = FMath::Sqrt(FMath::Square(Velocity.Y) + FMath::Square(Velocity.Z));
	const float NormalizedLateral = FMath::Clamp(LateralSpeed / 1800.0f, 0.0f, 1.0f);

	// 1. Suit Dynamic Material (Emissive telemetry, RCS thruster pulse, health/danger)
	if (SuitDynamicMaterial)
	{
		FLinearColor SuitEmissive = FLinearColor(0.0f, 0.95f, 1.0f); // Default Neon Cyan
		UWorld* World = GetWorld();
		const ASpaceGameState* GameState = World ? World->GetGameState<ASpaceGameState>() : nullptr;
		if (GameState)
		{
			const ESpaceRunPhase CurrentPhase = GameState->GetCurrentRunPhase();
			if (CurrentPhase == ESpaceRunPhase::CrashedKineticImpact)
			{
				SuitEmissive = FLinearColor(1.0f, 0.02f, 0.05f); // Red alert
			}
			else if (CurrentPhase == ESpaceRunPhase::LostToTheVoid)
			{
				SuitEmissive = FLinearColor(0.45f, 0.05f, 0.85f); // Purple void decay
			}
			else if (CurrentPhase == ESpaceRunPhase::Completed)
			{
				SuitEmissive = FLinearColor(0.0f, 1.0f, 0.45f); // Emerald victory
			}
		}

		SuitDynamicMaterial->SetVectorParameterValue(FName(TEXT("EmissiveColor")), SuitEmissive);
		SuitDynamicMaterial->SetScalarParameterValue(FName(TEXT("ThrusterPulse")), NormalizedLateral);
		SuitDynamicMaterial->SetScalarParameterValue(FName(TEXT("FlightSpeed")), Velocity.X);
		SuitDynamicMaterial->SetScalarParameterValue(FName(TEXT("PushChargeGlow")), CurrentPushChargeRatio);
	}

	// 2. Mechanical Arm Dynamic Material (Hydraulic heat glow and stress ratio)
	if (ArmDynamicMaterial)
	{
		const float HeatGlow = NormalizedOmega * 1.5f + (CurrentPushChargeRatio * 0.8f);
		ArmDynamicMaterial->SetScalarParameterValue(FName(TEXT("HydraulicStressHeatGlow")), HeatGlow);
		ArmDynamicMaterial->SetScalarParameterValue(FName(TEXT("MetallicPBR")), 0.9f);
		ArmDynamicMaterial->SetScalarParameterValue(FName(TEXT("RoughnessPBR")), 0.25f);
	}

	// 3. Mace Head Dynamic Material (Plasma core frequency, spin charge glow, centrifugal heat)
	if (MaceDynamicMaterial)
	{
		const float PlasmaFreq = 85.0f + (NormalizedOmega * 555.0f);
		const FLinearColor CoreColor = FLinearColor::LerpUsingHSV(
			FLinearColor(0.0f, 0.95f, 1.0f),    // Rest Cyan
			FLinearColor(0.97f, 0.15f, 0.52f),   // Blazing High-RPM Magenta
			NormalizedOmega
		);

		MaceDynamicMaterial->SetScalarParameterValue(FName(TEXT("PlasmaCoreFrequency")), PlasmaFreq);
		MaceDynamicMaterial->SetVectorParameterValue(FName(TEXT("PlasmaCoreColor")), CoreColor);
		MaceDynamicMaterial->SetScalarParameterValue(FName(TEXT("CentrifugalGlowIntensity")), 1.5f + (NormalizedOmega * 4.0f));
	}
}
