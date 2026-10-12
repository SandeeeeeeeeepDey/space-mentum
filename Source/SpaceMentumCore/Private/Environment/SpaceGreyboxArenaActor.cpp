/**
 * @file SpaceGreyboxArenaActor.cpp
 * @brief Implementation of the Google DeepMind-grade procedural testing arena and greybox level harness.
 */

#include "Environment/SpaceGreyboxArenaActor.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Components/ForwardDriftComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ASpaceGreyboxArenaActor::ASpaceGreyboxArenaActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	// Scene root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// High-contrast celestial lighting components
	KeyCosmicLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyCosmicLight"));
	KeyCosmicLight->SetupAttachment(RootComponent);
	KeyCosmicLight->SetRelativeRotation(FRotator(-40.0f, 45.0f, 0.0f));
	KeyCosmicLight->SetMobility(EComponentMobility::Movable);
	KeyCosmicLight->bCastVolumetricShadow = true;

	RimCosmicLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("RimCosmicLight"));
	RimCosmicLight->SetupAttachment(RootComponent);
	RimCosmicLight->SetRelativeRotation(FRotator(35.0f, -135.0f, 0.0f));
	RimCosmicLight->SetMobility(EComponentMobility::Movable);
	RimCosmicLight->bCastVolumetricShadow = false;

	CosmicSkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("CosmicSkyLight"));
	CosmicSkyLight->SetupAttachment(RootComponent);
	CosmicSkyLight->SetMobility(EComponentMobility::Movable);
	CosmicSkyLight->bRealTimeCapture = true;

	DeepSpaceFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("DeepSpaceFog"));
	DeepSpaceFog->SetupAttachment(RootComponent);
	DeepSpaceFog->SetMobility(EComponentMobility::Movable);
	DeepSpaceFog->SetFogDensity(0.00004f);

	PopulateDefaultSlalomTargets();
	ApplyLightingConfiguration();
}

void ASpaceGreyboxArenaActor::PopulateDefaultSlalomTargets()
{
	if (SlalomTargets.Num() > 0)
	{
		return;
	}

	// Curated testbed gates along the 600m corridor
	// At 3000 cm/s (30 m/s), gates arrive every 2-3 seconds:
	SlalomTargets.Add({
		TEXT("Gate 1: Lateral Right Evasion (+250cm)"),
		6000.0f,   // 60m (2.0s mark)
		FVector2D(250.0f, 0.0f),
		120.0f,
		FColor(255, 70, 70),
		false
	});

	SlalomTargets.Add({
		TEXT("Gate 2: Counter Whiplash Left (-300cm)"),
		12000.0f,  // 120m (4.0s mark)
		FVector2D(-300.0f, 0.0f),
		120.0f,
		FColor(255, 120, 40),
		false
	});

	SlalomTargets.Add({
		TEXT("Gate 3: High Apex Evade (+200cm Up)"),
		18000.0f,  // 180m (6.0s mark)
		FVector2D(0.0f, 200.0f),
		120.0f,
		FColor(40, 220, 255),
		false
	});

	SlalomTargets.Add({
		TEXT("Gate 4: Diagonal Vector Test (+220cm Right, -180cm Down)"),
		24000.0f,  // 240m (8.0s mark)
		FVector2D(220.0f, -180.0f),
		140.0f,
		FColor(200, 40, 255),
		false
	});

	SlalomTargets.Add({
		TEXT("Gate 5: Double S-Curve Climax (-280cm Left, +150cm Up)"),
		30000.0f,  // 300m (10.0s mark)
		FVector2D(-280.0f, 150.0f),
		130.0f,
		FColor(255, 230, 40),
		false
	});
}

void ASpaceGreyboxArenaActor::ApplyLightingConfiguration()
{
	if (KeyCosmicLight)
	{
		KeyCosmicLight->SetLightColor(LightingConfig.KeyCosmicLightColor);
		KeyCosmicLight->SetIntensity(LightingConfig.KeyCosmicLightIntensity);
	}

	if (RimCosmicLight)
	{
		RimCosmicLight->SetLightColor(LightingConfig.RimLightColor);
		RimCosmicLight->SetIntensity(LightingConfig.KeyCosmicLightIntensity * 0.6f);
	}

	if (CosmicSkyLight)
	{
		CosmicSkyLight->SetLightColor(LightingConfig.AmbientSkyColor);
		CosmicSkyLight->SetIntensity(1.0f);
	}
}

void ASpaceGreyboxArenaActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyLightingConfiguration();
	PopulateDefaultSlalomTargets();
}

void ASpaceGreyboxArenaActor::BeginPlay()
{
	Super::BeginPlay();
	ResetSlalomEvaluations();
}

void ASpaceGreyboxArenaActor::ResetSlalomEvaluations()
{
	for (FGreyboxSlalomTarget& Target : SlalomTargets)
	{
		Target.bHasBeenEvaluated = false;
	}
}

void ASpaceGreyboxArenaActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bEnableRuntimeDebugDrawing)
	{
		DrawProceduralCorridorRings();
	}

	EvaluateSlalomGates(DeltaSeconds);
}

void ASpaceGreyboxArenaActor::DrawProceduralCorridorRings()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();
	const FVector Up = GetActorUpVector();

	// Draw reference corridor rings along +X axis
	const int32 NumRings = FMath::FloorToInt(TunnelConfig.CorridorLengthCm / FMath::Max(100.0f, TunnelConfig.RingIntervalCm));
	for (int32 Index = 0; Index <= NumRings; ++Index)
	{
		const float RingX = Index * TunnelConfig.RingIntervalCm;
		const FVector RingCenter = Origin + Forward * RingX;

		// Check if this ring is a milestone gate
		const bool bIsMilestone = FMath::IsNearlyZero(FMath::Fmod(RingX, TunnelConfig.MilestoneIntervalCm), 10.0f);
		const FColor RingColor = bIsMilestone ? TunnelConfig.MilestoneRingColor : TunnelConfig.RegularRingColor;
		const float Thickness = bIsMilestone ? 4.0f : 1.5f;

		DrawDebugCircle(
			World,
			RingCenter,
			TunnelConfig.CorridorRadiusCm,
			TunnelConfig.RingSegments,
			RingColor,
			false,
			-1.0f,
			0,
			Thickness,
			Right,
			Up,
			false
		);

		// If milestone, draw center crosshair for alignment reference
		if (bIsMilestone)
		{
			DrawDebugLine(
				World,
				RingCenter - Right * 150.0f,
				RingCenter + Right * 150.0f,
				TunnelConfig.MilestoneRingColor,
				false,
				-1.0f,
				0,
				2.0f
			);
			DrawDebugLine(
				World,
				RingCenter - Up * 150.0f,
				RingCenter + Up * 150.0f,
				TunnelConfig.MilestoneRingColor,
				false,
				-1.0f,
				0,
				2.0f
			);
		}
	}

	// Draw slalom test targets
	for (const FGreyboxSlalomTarget& Target : SlalomTargets)
	{
		const FVector TargetCenter = Origin + Forward * Target.ForwardDistanceCm + Right * Target.LateralOffsetCm.X + Up * Target.LateralOffsetCm.Y;

		// Draw obstacle danger sphere
		DrawDebugSphere(
			World,
			TargetCenter,
			Target.TargetRadiusCm,
			16,
			Target.bHasBeenEvaluated ? FColor(80, 80, 80) : Target.TargetColor,
			false,
			-1.0f,
			0,
			2.5f
		);

		// Draw stem connecting target to outer tunnel ring for depth perception
		const FVector NormalOffset = (Right * Target.LateralOffsetCm.X + Up * Target.LateralOffsetCm.Y).GetSafeNormal();
		const FVector WallAnchor = Origin + Forward * Target.ForwardDistanceCm + NormalOffset * TunnelConfig.CorridorRadiusCm;
		DrawDebugLine(
			World,
			TargetCenter,
			WallAnchor,
			FColor(60, 70, 90),
			false,
			-1.0f,
			0,
			1.0f
		);
	}
}

void ASpaceGreyboxArenaActor::EvaluateSlalomGates(float DeltaSeconds)
{
	if (!CachedAstronaut.IsValid())
	{
		CachedAstronaut = Cast<ASpaceAstronautCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	}

	if (!CachedAstronaut.IsValid())
	{
		return;
	}

	const FVector PlayerLocation = CachedAstronaut->GetActorLocation();
	const FVector Origin = GetActorLocation();
	const float PlayerX = PlayerLocation.X - Origin.X;

	for (FGreyboxSlalomTarget& Target : SlalomTargets)
	{
		if (Target.bHasBeenEvaluated)
		{
			continue;
		}

		// When player crosses the gate plane (along +X)
		if (PlayerX >= Target.ForwardDistanceCm)
		{
			Target.bHasBeenEvaluated = true;

			// Compute player's lateral position in local YZ
			const float PlayerY = PlayerLocation.Y - Origin.Y;
			const float PlayerZ = PlayerLocation.Z - Origin.Z;

			const float DeltaY = PlayerY - Target.LateralOffsetCm.X;
			const float DeltaZ = PlayerZ - Target.LateralOffsetCm.Y;
			const float ClearanceDistance = FMath::Sqrt(DeltaY * DeltaY + DeltaZ * DeltaZ);

			// Evasion successful if player didn't collide with the target core
			const bool bSuccessful = ClearanceDistance > Target.TargetRadiusCm;

			OnSlalomGateEvaluated.Broadcast(
				Target.TargetName,
				ClearanceDistance,
				bSuccessful,
				CachedAstronaut->GetVelocity()
			);

			// Output prominent on-screen telemetry
			if (GEngine)
			{
				const FColor MessageColor = bSuccessful ? FColor(50, 255, 120) : FColor(255, 60, 60);
				const FString ResultText = bSuccessful ? TEXT("CLEAN EVASION! PASS") : TEXT("COLLISION HAZARD! GRAZED");

				GEngine->AddOnScreenDebugMessage(
					-1,
					4.0f,
					MessageColor,
					FString::Printf(
						TEXT("[%s] %s | Clearance: %.1f cm (Req: >%.1f cm)"),
						*ResultText,
						*Target.TargetName,
						ClearanceDistance,
						Target.TargetRadiusCm
					)
				);
			}
		}
	}
}
