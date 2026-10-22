/**
 * @file SpaceMentumCoreTypes.h
 * @brief Game-flow level enums and structs shared across the Core module.
 *
 * @creator_feel
 * "thrilling vulnerability mixed with heavy kinetic power" (GDD §0.1)
 *
 * @emotional_intent
 * A run is a single, unbroken plunge through space. These phases exist to frame that
 * plunge — a breath before, the relentless run, and an honest end — never to interrupt it.
 */

#pragma once

#include "CoreMinimal.h"
#include "SpaceMentumCoreTypes.generated.h"

/** High-level phase of a single run. Owned by ASpaceGameState. */
UENUM(BlueprintType)
enum class ESpaceRunPhase : uint8
{
	/** Stationary at sector threshold: anchored against starting asteroid, aiming departure vector and charging push. */
	LaunchPushOff         UMETA(DisplayName = "Launch Push-Off"),
	/** Before the plunge: camera settles, player grips the mace (legacy alias). */
	PreLaunch             UMETA(DisplayName = "Pre-Launch"),
	/** Relentless forward drift — the active gameplay loop. */
	Running               UMETA(DisplayName = "Running"),
	/** Rule 3 Failure 1: The astronaut collided with an obstacle at lethal speed. */
	CrashedKineticImpact  UMETA(DisplayName = "Crashed (Kinetic Impact)"),
	/** Rule 3 Failure 2: 13s–40s predictive lookahead detected zero reachable anchors in empty void. */
	LostToTheVoid         UMETA(DisplayName = "Lost to the Void"),
	/** Rule 2 Win Condition: Successfully navigated corridor to sector objective. */
	Completed             UMETA(DisplayName = "Completed"),
};

/** Classification of tactical radar contacts within the 120m cockpit sphere. */
UENUM(BlueprintType)
enum class EContactHazardType : uint8
{
	Asteroid              UMETA(DisplayName = "Dense Asteroid"),
	StationDebris         UMETA(DisplayName = "Station Debris"),
	SlalomGate            UMETA(DisplayName = "Slalom Navigation Gate"),
	StartingAsteroid      UMETA(DisplayName = "Starting Celestial Anchor"),
	ObjectiveMarker       UMETA(DisplayName = "Sector Objective"),
};

/** Spatial contact tracked by the cockpit tactical radar within the 120m lookahead sphere. */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FTacticalRadarContact
{
	GENERATED_BODY()

	/** Relative position from astronaut Center of Mass (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar")
	FVector RelativePosition = FVector::ZeroVector;

	/** Direct Euclidean distance from astronaut (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar")
	float DistanceCm = 0.0f;

	/** Hazard classification for color-coded radar rendering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar")
	EContactHazardType HazardType = EContactHazardType::Asteroid;

	/** Collision/bounding radius of contact (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radar")
	float RadiusCm = 80.0f;
};

/** High-priority proximity collision hazard alert state (Step 7 Audio & Visual Polish). */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FProximityAlarmState
{
	GENERATED_BODY()

	/** True when an obstacle is within critical proximity alarm range (< 2500 cm = 25 m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
	bool bIsActive = false;

	/** Euclidean distance from astronaut body to nearest obstacle (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
	float DistanceToNearestObstacleCm = 10000.0f;

	/** Normalized threat ratio [0.0 = safe at 25m, 1.0 = imminent collision at 0m]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
	float ThreatRatio = 0.0f;

	/** Unit vector pointing toward the nearest proximate hazard. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
	FVector ThreatDirection = FVector::ZeroVector;
};

/** Live cockpit flight instrumentation telemetry stream (Rule 4 compliant — no predictive lines). */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FFlightTelemetryData
{
	GENERATED_BODY()

	/** Instantaneous directional heading vector (or departure aim during LaunchPushOff). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	FVector DirectionalHeadingVector = FVector::ForwardVector;

	/** Instantaneous forward flight speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float ForwardSpeedCmPerSec = 3000.0f;

	/** Instantaneous lateral drift magnitude sqrt(vy^2 + vz^2) (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float LateralDriftSpeedCmPerSec = 0.0f;

	/** Absolute total speed magnitude sqrt(vx^2 + vy^2 + vz^2) (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float TotalSpeedCmPerSec = 3000.0f;

	/** Speed converted to kilometers per hour (km/h) for cybernetic speedometer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float TotalSpeedKmh = 108.0f;

	/** 0.0 to 1.0 charge ratio during LaunchPushOff phase. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float PushPowerChargeRatio = 0.0f;

	/** Live rotational kinetic energy E_k = 0.5 * m_m * v_t^2 (Joules) during flight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float RotationalKineticEnergyJoules = 0.0f;

	/** Active proximity collision alarm state. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	FProximityAlarmState ProximityAlarm;

	/** Proximate navigational contacts within the 120m cockpit sphere. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	TArray<FTacticalRadarContact> RadarContacts;

	/** Whether a colossal asteroid is currently targeted and locked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	bool bKineticLockActive = false;

	/** Distance to currently locked target (meters). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float LockedTargetDistanceMeters = 0.0f;

	/** Relative closing speed of locked target (km/h). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float LockedTargetRelativeSpeedKmh = 0.0f;

	/** Time-to-impact (TTI) for locked target in seconds (-1 if diverging). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float LockedTargetTimeToImpactSec = -1.0f;

	/** Whether locked target has breached critical proximity (< 18m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	bool bLockedTargetCriticalProximity = false;

	/** Active kinetic push capacitor charge ratio [0.0, 1.0]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float KineticPushChargeRatio = 0.0f;

	/** Kinetic push force rating in kiloNewtons (kN). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float KineticPushForceKiloNewtons = 0.0f;
};

/** Tunings for the Starting Asteroid Push-Off launch phase. */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FLaunchPushOffParameters
{
	GENERATED_BODY()

	/** Maximum launch force exerted against starting asteroid (Newtons). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	float MaxLaunchForceNewtons = 12000.0f;

	/** Target launch departure speed at 100% push charge (cm/s). Defaults to 3000 cm/s = 108 km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	float MaxLaunchSpeedCmPerSec = 3000.0f;

	/** Rate at which push power builds up when holding charge (1.0 = full charge in 1.0 second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	float ChargeRatePerSec = 0.85f;

	/** Angular steering rate of the departure direction arrow (deg/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	float AimAngularSpeedDegPerSec = 90.0f;

	/** Maximum deflection angle off axial corridor normal (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	float MaxDepartureDeflectionAngleDeg = 35.0f;
};
