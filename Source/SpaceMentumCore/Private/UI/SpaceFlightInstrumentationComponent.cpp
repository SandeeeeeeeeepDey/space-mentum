/**
 * @file SpaceFlightInstrumentationComponent.cpp
 * @brief Implementation of cockpit flight instrumentation (Radar, Speedometer, Direction Arrow, Push Power).
 */

#include "UI/SpaceFlightInstrumentationComponent.h"
#include "Player/SpaceAstronautCharacter.h"
#include "GameModes/SpaceGameState.h"
#include "Environment/SpaceObstacleActor.h"
#include "Environment/SpaceStartingAsteroidActor.h"
#include "Environment/SpaceGreyboxArenaActor.h"
#include "Components/ForwardDriftComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "Audio/SpaceProceduralFlightAudioComponent.h"
#include "Components/SpaceKineticManipulatorComponent.h"
#include "Kismet/GameplayStatics.h"

USpaceFlightInstrumentationComponent::USpaceFlightInstrumentationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	RadarDetectionRangeCm = 12000.0f; // 120 meters lookahead sphere
	RadarScanAccumulator = 0.0f;
}

void USpaceFlightInstrumentationComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ASpaceAstronautCharacter>(GetOwner());

	UWorld* World = GetWorld();
	if (World)
	{
		CachedGameState = World->GetGameState<ASpaceGameState>();
	}
}

void USpaceFlightInstrumentationComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwningCharacter.IsValid())
	{
		return;
	}

	const ESpaceRunPhase CurrentPhase = CachedGameState.IsValid()
		? CachedGameState->GetCurrentRunPhase()
		: ESpaceRunPhase::Running;

	const FVector AstronautLocation = OwningCharacter->GetActorLocation();

	// 1. Compute Instantaneous Heading & Speeds
	FVector VelocityVector = OwningCharacter->GetVelocity();

	if (CurrentPhase == ESpaceRunPhase::LaunchPushOff)
	{
		// During launch: directional heading reflects departure aim vector
		CurrentTelemetry.DirectionalHeadingVector = OwningCharacter->GetActorForwardVector().GetSafeNormal();
		CurrentTelemetry.ForwardSpeedCmPerSec = 0.0f;
		CurrentTelemetry.LateralDriftSpeedCmPerSec = 0.0f;
		CurrentTelemetry.TotalSpeedCmPerSec = 0.0f;
		CurrentTelemetry.TotalSpeedKmh = 0.0f;
		CurrentTelemetry.RotationalKineticEnergyJoules = 0.0f;
	}
	else
	{
		// During flight: directional heading reflects true instantaneous velocity vector
		if (!VelocityVector.IsNearlyZero())
		{
			CurrentTelemetry.DirectionalHeadingVector = VelocityVector.GetSafeNormal();
		}
		else
		{
			CurrentTelemetry.DirectionalHeadingVector = FVector::ForwardVector;
		}

		CurrentTelemetry.ForwardSpeedCmPerSec = VelocityVector.X;
		CurrentTelemetry.LateralDriftSpeedCmPerSec = FMath::Sqrt(FMath::Square(VelocityVector.Y) + FMath::Square(VelocityVector.Z));
		CurrentTelemetry.TotalSpeedCmPerSec = VelocityVector.Size();
		CurrentTelemetry.TotalSpeedKmh = CurrentTelemetry.TotalSpeedCmPerSec * 0.036f;

		// Rotational kinetic energy: E_k = 0.5 * m_m * v_t^2
		UMaceOrbitComponent* OrbitComp = OwningCharacter->GetMaceOrbit();
		if (OrbitComp)
		{
			const float OmegaRad = OrbitComp->GetAngularVelocityRadPerSec();
			const float TetherMeters = OrbitComp->GetTetherLengthCm() * 0.01f;
			const float MaceMassKg = OrbitComp->GetMaceMassKg();
			const float TangentialSpeedMPerSec = FMath::Abs(OmegaRad) * TetherMeters;
			CurrentTelemetry.RotationalKineticEnergyJoules = 0.5f * MaceMassKg * FMath::Square(TangentialSpeedMPerSec);
		}
	}

	// 1.5. Query Kinetic Manipulator Telemetry (Target Lock, Range, TTI, Push Charge)
	USpaceKineticManipulatorComponent* ManipulatorComp = OwningCharacter->GetKineticManipulator();
	if (ManipulatorComp)
	{
		const ETargetLockStatus LockStatus = ManipulatorComp->GetLockStatus();
		CurrentTelemetry.bKineticLockActive = (LockStatus == ETargetLockStatus::TargetLocked || LockStatus == ETargetLockStatus::CriticalProximity);
		const FKineticTargetLockInfo& LockInfo = ManipulatorComp->GetLockInfo();
		CurrentTelemetry.LockedTargetDistanceMeters = LockInfo.SurfaceDistanceCm * 0.01f;
		CurrentTelemetry.LockedTargetRelativeSpeedKmh = LockInfo.RelativeVelocity.Size() * 0.036f;
		CurrentTelemetry.LockedTargetTimeToImpactSec = LockInfo.TimeToImpactSeconds;
		CurrentTelemetry.bLockedTargetCriticalProximity = LockInfo.bInCriticalZone;
		CurrentTelemetry.KineticPushChargeRatio = ManipulatorComp->GetPushChargeRatio();
		CurrentTelemetry.KineticPushForceKiloNewtons = (ManipulatorComp->GetPushChargeRatio() * 50000.0f) * 0.001f;
	}
	else
	{
		CurrentTelemetry.bKineticLockActive = false;
		CurrentTelemetry.LockedTargetDistanceMeters = 0.0f;
		CurrentTelemetry.LockedTargetRelativeSpeedKmh = 0.0f;
		CurrentTelemetry.LockedTargetTimeToImpactSec = -1.0f;
		CurrentTelemetry.bLockedTargetCriticalProximity = false;
		CurrentTelemetry.KineticPushChargeRatio = 0.0f;
		CurrentTelemetry.KineticPushForceKiloNewtons = 0.0f;
	}

	// 2. Refresh Tactical Radar Contacts at 20 Hz
	RadarScanAccumulator += DeltaTime;
	if (RadarScanAccumulator >= 0.05f)
	{
		RadarScanAccumulator = 0.0f;
		RefreshRadarContacts();
	}

	// 3. Broadcast updated telemetry for cockpit HUD widgets
	if (OnFlightTelemetryUpdated.IsBound())
	{
		OnFlightTelemetryUpdated.Broadcast(CurrentTelemetry);
	}
}

void USpaceFlightInstrumentationComponent::RefreshRadarContacts()
{
	UWorld* World = GetWorld();
	if (!World || !OwningCharacter.IsValid())
	{
		return;
	}

	const FVector AstronautLocation = OwningCharacter->GetActorLocation();
	CurrentTelemetry.RadarContacts.Reset();

	float MinObstacleDistanceCm = 999999.0f;
	FVector ClosestObstacleDir = FVector::ZeroVector;

	// A. Query proximate obstacles (asteroids & station debris)
	TArray<AActor*> FoundObstacles;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceObstacleActor::StaticClass(), FoundObstacles);

	for (AActor* Actor : FoundObstacles)
	{
		ASpaceObstacleActor* Obs = Cast<ASpaceObstacleActor>(Actor);
		if (!Obs || Obs->IsDestroyed())
		{
			continue;
		}

		const FVector RelPos = Obs->GetActorLocation() - AstronautLocation;
		const float Dist = RelPos.Size();

		// Track closest hazardous obstacle for proximity alarm
		const float SurfaceDist = FMath::Max(0.0f, Dist - Obs->GetCollisionRadiusCm());
		if (SurfaceDist < MinObstacleDistanceCm)
		{
			MinObstacleDistanceCm = SurfaceDist;
			ClosestObstacleDir = RelPos.GetSafeNormal();
		}

		if (Dist <= RadarDetectionRangeCm)
		{
			FTacticalRadarContact Contact;
			Contact.RelativePosition = RelPos;
			Contact.DistanceCm = Dist;
			Contact.RadiusCm = Obs->GetCollisionRadiusCm();
			Contact.HazardType = Obs->IsAnchored()
				? EContactHazardType::Asteroid
				: EContactHazardType::StationDebris;

			CurrentTelemetry.RadarContacts.Add(Contact);
		}
	}

	// B. Query starting asteroid
	TArray<AActor*> FoundStartingAsteroids;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStartingAsteroidActor::StaticClass(), FoundStartingAsteroids);

	for (AActor* Actor : FoundStartingAsteroids)
	{
		ASpaceStartingAsteroidActor* StartAsteroid = Cast<ASpaceStartingAsteroidActor>(Actor);
		if (!StartAsteroid)
		{
			continue;
		}

		const FVector RelPos = StartAsteroid->GetActorLocation() - AstronautLocation;
		const float Dist = RelPos.Size();

		if (Dist <= RadarDetectionRangeCm)
		{
			FTacticalRadarContact Contact;
			Contact.RelativePosition = RelPos;
			Contact.DistanceCm = Dist;
			Contact.RadiusCm = StartAsteroid->GetAsteroidRadius();
			Contact.HazardType = EContactHazardType::StartingAsteroid;

			CurrentTelemetry.RadarContacts.Add(Contact);
		}
	}

	// C. Query slalom gates from arena actor
	TArray<AActor*> FoundArenas;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceGreyboxArenaActor::StaticClass(), FoundArenas);

	for (AActor* Actor : FoundArenas)
	{
		ASpaceGreyboxArenaActor* Arena = Cast<ASpaceGreyboxArenaActor>(Actor);
		if (!Arena)
		{
			continue;
		}

		const TArray<FSlalomGateState>& Gates = Arena->GetActiveSlalomGates();
		for (const FSlalomGateState& Gate : Gates)
		{
			const FVector GateWorldPos = Arena->GetActorLocation() + FVector(Gate.DistanceAlongCorridorCm, Gate.HoleOffset.Y, Gate.HoleOffset.Z);
			const FVector RelPos = GateWorldPos - AstronautLocation;
			const float Dist = RelPos.Size();

			if (Dist <= RadarDetectionRangeCm)
			{
				FTacticalRadarContact Contact;
				Contact.RelativePosition = RelPos;
				Contact.DistanceCm = Dist;
				Contact.RadiusCm = Gate.ClearanceRadiusCm;
				Contact.HazardType = EContactHazardType::SlalomGate;

				CurrentTelemetry.RadarContacts.Add(Contact);
			}
		}
	}

	// D. Evaluate Proximity Alarm State (Step 7 Audio & Visual Polish)
	constexpr float ProximityAlertThresholdCm = 2500.0f; // 25 meters
	if (MinObstacleDistanceCm <= ProximityAlertThresholdCm)
	{
		CurrentTelemetry.ProximityAlarm.bIsActive = true;
		CurrentTelemetry.ProximityAlarm.DistanceToNearestObstacleCm = MinObstacleDistanceCm;
		CurrentTelemetry.ProximityAlarm.ThreatRatio = FMath::Clamp(1.0f - (MinObstacleDistanceCm / ProximityAlertThresholdCm), 0.0f, 1.0f);
		CurrentTelemetry.ProximityAlarm.ThreatDirection = ClosestObstacleDir;
	}
	else
	{
		CurrentTelemetry.ProximityAlarm.bIsActive = false;
		CurrentTelemetry.ProximityAlarm.DistanceToNearestObstacleCm = MinObstacleDistanceCm;
		CurrentTelemetry.ProximityAlarm.ThreatRatio = 0.0f;
		CurrentTelemetry.ProximityAlarm.ThreatDirection = FVector::ZeroVector;
	}

	if (OwningCharacter.IsValid() && OwningCharacter->GetFlightAudio())
	{
		OwningCharacter->GetFlightAudio()->UpdateProximityRadarAlarm(MinObstacleDistanceCm, ProximityAlertThresholdCm);
	}
}
