/**
 * @file SpaceMentumKineticManipulationTypes.h
 * @brief Type definitions, state structs, and tuning parameters for kinetic push and pull mechanics.
 *
 * @creator_feel
 * "the game needs a screen where the asteroids are much bigger too.
 *  the belt of the asteroid needs to be much bigger and outside the view.
 *  the asteroids should be comming from any angle which has atleast one component from the forward.
 *  we aim an asteroid and then lock it and then we decide how hard we will push it unless
 *  it is too close, that we need to decide if we still want to push or it will push it.
 *  lets implement the push first, then we can work on the pull"
 *
 * @emotional_intent
 * Tension, massive scale, and split-second Newtonian decisions. Aiming at a towering,
 * mountain-sized asteroid tumbling toward you from deep space and deciding exactly how much
 * kinetic charge to unleash. When it breaches critical proximity, the terrifying dilemma
 * kicks in: push with explosive force to deflect it, or risk its colossal oncoming momentum
 * completely overpowering your kinetic burst and pushing YOU into the void.
 */

#pragma once

#include "CoreMinimal.h"
#include "SpaceMentumKineticManipulationTypes.generated.h"

/**
 * Operating mode for the mechanical kinetic manipulator.
 * Push is fully operational; Pull is architected for immediate drop-in extension.
 */
UENUM(BlueprintType)
enum class EKineticManipulationMode : uint8
{
	/** Kinetic repulsion impulse directed away from the manipulator. */
	Push  UMETA(DisplayName = "Kinetic Push (Repulsion)"),

	/** Kinetic gravitational attraction pulling the target toward the manipulator (Next Phase). */
	Pull  UMETA(DisplayName = "Kinetic Pull (Attraction - Next Phase)"),

	/** Manipulator inactive or idling. */
	None  UMETA(DisplayName = "Inactive / Neutral"),
};

/**
 * Target lock-on state for an oncoming celestial hazard.
 */
UENUM(BlueprintType)
enum class ETargetLockStatus : uint8
{
	/** No candidate asteroid in sight or aiming cone. */
	NoTarget          UMETA(DisplayName = "No Target"),

	/** Aim ray is sweeping across a candidate asteroid; calculating trajectory solution. */
	AcquiringTarget   UMETA(DisplayName = "Acquiring Target"),

	/** High-confidence tracking lock established; telemetry active. */
	TargetLocked      UMETA(DisplayName = "Target Locked"),

	/** Target has breached critical close-proximity threshold (< 18m). Threat critical. */
	CriticalProximity UMETA(DisplayName = "Critical Proximity Alert"),
};

/**
 * Outcome of the critical close-proximity push dilemma.
 * "unless it is too close, that we need to decide if we still want to push or it will push it."
 */
UENUM(BlueprintType)
enum class EProximityDecisionOutcome : uint8
{
	/** Target is outside danger proximity; standard Newtonian push applies. */
	SafeRangePush           UMETA(DisplayName = "Safe Range Push"),

	/** Player committed to pushing inside critical distance and successfully overpowered the oncoming rock. */
	HeroicClosePushDeflect  UMETA(DisplayName = "Heroic Point-Blank Deflection"),

	/** Asteroid momentum was too high / push charge was too weak: asteroid overpowered the push and shoved the player! */
	AsteroidOverpoweredPush UMETA(DisplayName = "Asteroid Overpowered Push (It Pushes You)"),

	/** Player chose not to push and attempted an internal mace evasion dodge instead. */
	EvadedWithoutPush       UMETA(DisplayName = "Evasive Dodge Without Push"),
};

/**
 * Telemetry and kinematic tracking data for a locked celestial target.
 */
USTRUCT(BlueprintType)
struct FKineticTargetLockInfo
{
	GENERATED_BODY()

	/** Pointer to the targeted actor (e.g. ASpaceColossalAsteroidActor). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	/** Instantaneous world location of target center of mass. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector TargetWorldLocation = FVector::ZeroVector;

	/** Absolute velocity of the asteroid in world space (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector TargetVelocityWorld = FVector::ZeroVector;

	/** Relative velocity vector (Asteroid Velocity - Astronaut Velocity) in cm/s. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector RelativeVelocity = FVector::ZeroVector;

	/** Direct Euclidean distance from astronaut appendage to asteroid surface (cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float SurfaceDistanceCm = 0.0f;

	/** Center-to-center distance (cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float CenterDistanceCm = 0.0f;

	/** Estimated time-to-impact in seconds based on closing velocity. Negative if moving away. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float TimeToImpactSeconds = 0.0f;

	/** Bounding collision radius of the target asteroid (cm). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float TargetRadiusCm = 350.0f;

	/** Physical mass of the target asteroid (kg). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float TargetMassKg = 25000.0f;

	/** Angle off the astronaut's forward view heading (degrees). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float ApproachAngleDeg = 0.0f;

	/** Whether the target has entered the critical proximity danger zone. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	bool bInCriticalZone = false;
};

/**
 * Designer-exposed feel knobs for kinetic aiming, locking, and push mechanics.
 */
USTRUCT(BlueprintType)
struct FKineticPushTuning
{
	GENERATED_BODY()

	/** Maximum repulsive force delivered at 100% capacitor charge (Newtons). Default = 50,000 N. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "5000.0"))
	float MaxPushForceNewtons = 50000.0f;

	/** Maximum delivered impulse rating at 100% charge (N*s). Default = 20,000 N*s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "1000.0"))
	float MaxPushImpulseNs = 20000.0f;

	/** Rate of push charge accumulation per second (1.0 = 100% charge in 1.0 second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "0.2", ClampMax = "5.0"))
	float ChargeRatePerSec = 0.95f;

	/** Maximum lock acquisition range (cm). Default = 15,000 cm (150 meters). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "2000.0"))
	float MaxLockRangeCm = 15000.0f;

	/**
	 * Critical close-proximity distance threshold (cm).
	 * If target is closer than this, the "push or be pushed" dilemma triggers.
	 * Default = 1800 cm (18 meters).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "500.0"))
	float CriticalProximityThresholdCm = 1800.0f;

	/** Half-angle of the targeting acquisition cone off the aim vector (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "5.0", ClampMax = "60.0"))
	float AimConeHalfAngleDeg = 28.0f;

	/**
	 * Ratio of incoming asteroid momentum to push impulse required for the asteroid to overpower the push.
	 * If (TargetMomentum > PushImpulse * Ratio), the asteroid shunts the player backwards.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float OverpowerMomentumThresholdRatio = 1.35f;

	/** Newton's 3rd Law reaction recoil efficiency on the astronaut [0, 1]. Default = 0.85. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ReactionRecoilEfficiency = 0.85f;
};

/**
 * Result bundle returned after executing a kinetic push.
 */
USTRUCT(BlueprintType)
struct FKineticPushExecutionResult
{
	GENERATED_BODY()

	/** Whether the push successfully deployed an impulse. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	bool bImpulseDelivered = false;

	/** Outcome category of the push decision. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	EProximityDecisionOutcome Outcome = EProximityDecisionOutcome::SafeRangePush;

	/** Transferred impulse vector applied to the asteroid (N*s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector TransferredImpulseToAsteroid = FVector::ZeroVector;

	/** Newton reaction recoil impulse vector applied to the astronaut (N*s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector ReactionImpulseToAstronaut = FVector::ZeroVector;

	/** Resulting change in velocity for the asteroid (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector AsteroidDeltaVelocity = FVector::ZeroVector;

	/** Resulting change in velocity for the astronaut (cm/s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector AstronautDeltaVelocity = FVector::ZeroVector;

	/** Shunt impulse magnitude if the asteroid overpowered the push (N*s). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float ShuntImpulseMagnitude = 0.0f;
};
