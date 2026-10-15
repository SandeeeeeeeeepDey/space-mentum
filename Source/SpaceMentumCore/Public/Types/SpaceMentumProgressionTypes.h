/**
 * @file SpaceMentumProgressionTypes.h
 * @brief Google DeepMind-grade data structures and mathematical definitions for Sector Progression,
 *        Kinetic Telemetry Scoring, Milestone Objectives, and Leaderboard Persistence.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction to move away from obsticles."
 * "abandon realistic space spacing. The levels need to be incredibly dense navigational hazards..."
 *
 * @emotional_intent
 * Surviving a high-velocity celestial gauntlet is a feat of raw spatial intuition and Newtonian mastery.
 * Every clean evasion, every deliberate momentum strike, and every millisecond spent hurtling through
 * dense debris must be recognized and celebrated. Sector completion portals provide a breathtaking crescendo,
 * while kinetic efficiency grades and leaderboard persistence reward mastery and invite endless mastery.
 */

#pragma once

#include "CoreMinimal.h"
#include "SpaceMentumProgressionTypes.generated.h"

/**
 * Performance grading scale evaluating flight mastery, kinetic momentum conservation, and evasion cleanliness.
 */
UENUM(BlueprintType)
enum class EKineticFlightGrade : uint8
{
	/** Flawless spatial flow: maximum speed, zero collision grazes, all gates cleared. */
	Grade_S_Plus   UMETA(DisplayName = "S+ (Kinetic Ascendant)"),
	/** Exceptional mastery: high speed, clean evasion lines, aggressive momentum strikes. */
	Grade_S        UMETA(DisplayName = "S (Orbital Maestro)"),
	/** Commendable navigation: reached sector portal with solid momentum and clean line. */
	Grade_A        UMETA(DisplayName = "A (Clean Navigator)"),
	/** Surviving through turbulence: reached portal despite several obstacle grazes. */
	Grade_B        UMETA(DisplayName = "B (Debris Survivor)"),
	/** Barely made it through: low momentum or frequent boundary friction. */
	Grade_C        UMETA(DisplayName = "C (Newtonian Novice)"),
	/** Run terminated by catastrophic kinetic impact or lost to empty void. */
	Failed         UMETA(DisplayName = "F (Terminated in Void)"),
};

/**
 * Spatial definition and score bonus for an in-flight milestone checkpoint along the corridor.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FSectorMilestoneDefinition
{
	GENERATED_BODY()

	/** Milestone index along the sector runway (1, 2, 3, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 MilestoneIndex = 1;

	/** Fractional progress along total corridor length (e.g. 0.25, 0.50, 0.75). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	float DistanceRatio = 0.25f;

	/** Absolute forward distance from sector threshold in centimeters (+X). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	float DistanceCm = 30000.0f;

	/** Informative callsign for HUD display (e.g. "Alpha Threshold", "Debris Terminus"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FString MilestoneTitle = TEXT("Checkpoint Alpha");

	/** Bonus score awarded immediately upon crossing this threshold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 ScoreBonus = 500;

	/** Emissive neon accent color for the physical milestone arch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FLinearColor GateColor = FLinearColor(0.0f, 0.94f, 1.0f);
};

/**
 * Data-driven configuration of a space sector / gauntlet environment.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FSectorDefinition
{
	GENERATED_BODY()

	/** Unique numerical index of the sector. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	int32 SectorIndex = 1;

	/** Descriptive astronomical designation (e.g. "Sector 1: Perseus Shallows"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	FString SectorName = TEXT("Sector 1: Perseus Shallows");

	/** Thematic lore / mission briefing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	FString BriefingText = TEXT("Navigate the initial orbital debris corridor and breach the Jump Portal.");

	/** Total track length from launch asteroid (X = 0) to Jump Portal (X = CorridorLengthCm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	float CorridorLengthCm = 120000.0f; // 1.2 km = 40 seconds at 30 m/s

	/** Radial boundary cylinder radius in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	float CorridorRadiusCm = 800.0f;

	/** Target par completion time in seconds for high-grade kinetic multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	float ParTimeSeconds = 38.0f;

	/** Relative hazard density multiplier (1.0 = standard, 1.5 = high APM gauntlet). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	float HazardDensityMultiplier = 1.0f;

	/** Base forward drift velocity for this sector (cm/s). Defaults to 3000 cm/s = 108 km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	float BaseForwardDriftSpeedCmPerSec = 3000.0f;

	/** Jump Portal event horizon color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sector")
	FLinearColor PortalThemeColor = FLinearColor(0.0f, 0.95f, 1.0f);
};

/**
 * Comprehensive kinetic flight telemetry score and post-run debrief.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FSectorFlightScore
{
	GENERATED_BODY()

	/** Sector index where this run took place. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 SectorIndex = 1;

	/** Sector title. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	FString SectorName = TEXT("Perseus Shallows");

	/** Whether the player successfully reached and breached the Exit Jump Portal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	bool bIsCompleted = false;

	/** Total active flight time (seconds) in Running phase. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float ElapsedTimeSeconds = 0.0f;

	/** Total linear forward distance traversed in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float TotalDistanceTraveledCm = 0.0f;

	/** Average velocity along track (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float AverageSpeedCmPerSec = 3000.0f;

	/** Peak instantaneous velocity reached (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float PeakSpeedCmPerSec = 3000.0f;

	/** Number of calibrated slalom navigation gates cleanly evaded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 CleanGatesCleared = 0;

	/** Total number of slalom navigation gates placed in the sector. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 TotalGatesCount = 5;

	/** Number of successful external mace strikes executed against obstacles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 ExternalStrikesLanded = 0;

	/** Number of close boundary grazes or corridor scrapes penalizing score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 GrazeBoundaryPenalties = 0;

	/** Composite final score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 CompositeScore = 0;

	/** Overall performance grade awarded. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	EKineticFlightGrade PerformanceGrade = EKineticFlightGrade::Grade_A;

	/**
	 * Computes the unified Google DeepMind-calibrated kinetic composite score:
	 * CompositeScore = (DistanceMeters * 10) + (AvgSpeed * 0.2) + (CleanGates * 500)
	 *                + (Strikes * 300) - (ElapsedSeconds * 25) - (Grazes * 200)
	 */
	static int32 ComputeCompositeScore(
		float InDistanceCm,
		float InElapsedTimeSec,
		float InAvgSpeedCmPerSec,
		int32 InCleanGates,
		int32 InStrikes,
		int32 InGrazes,
		bool bInCompleted)
	{
		const float distMeters = InDistanceCm * 0.01f;
		float score = (distMeters * 10.0f)
			+ (InAvgSpeedCmPerSec * 0.2f)
			+ (InCleanGates * 500.0f)
			+ (InStrikes * 300.0f)
			- (InElapsedTimeSec * 25.0f)
			- (InGrazes * 200.0f);

		if (bInCompleted)
		{
			score += 2500.0f; // Completion bonus for breaching the Jump Portal
		}

		return FMath::Max(0, FMath::RoundToInt(score));
	}

	/**
	 * Evaluates performance grade based on score and completion status.
	 */
	static EKineticFlightGrade EvaluateGrade(int32 InScore, bool bInCompleted)
	{
		if (!bInCompleted)
		{
			return EKineticFlightGrade::Failed;
		}

		if (InScore >= 16000) return EKineticFlightGrade::Grade_S_Plus;
		if (InScore >= 13500) return EKineticFlightGrade::Grade_S;
		if (InScore >= 10500) return EKineticFlightGrade::Grade_A;
		if (InScore >= 7500)  return EKineticFlightGrade::Grade_B;
		return EKineticFlightGrade::Grade_C;
	}
};

/**
 * Single high-score leaderboard entry for persistence.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FSectorLeaderboardEntry
{
	GENERATED_BODY()

	/** Player callsign or astronaut designation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString PlayerCallsign = TEXT("Astronaut-77");

	/** Final composite score achieved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 CompositeScore = 0;

	/** Performance grade achieved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	EKineticFlightGrade PerformanceGrade = EKineticFlightGrade::Grade_A;

	/** Total flight duration in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float FlightTimeSeconds = 0.0f;

	/** Peak flight speed in km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	float PeakSpeedKmh = 108.0f;

	/** Clean gates cleared ratio (e.g. "5/5"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString GatesRatio = TEXT("5/5");

	/** Formatted timestamp string when record was set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	FString TimestampString = TEXT("2026-10-14 16:00 UTC");
};

/**
 * High-score table for a specific sector.
 */
USTRUCT(BlueprintType)
struct SPACEMENTUMCORE_API FSectorLeaderboardTable
{
	GENERATED_BODY()

	/** Sector index this table belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	int32 SectorIndex = 1;

	/** Top recorded runs sorted in descending order of composite score. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaderboard")
	TArray<FSectorLeaderboardEntry> TopEntries;
};
