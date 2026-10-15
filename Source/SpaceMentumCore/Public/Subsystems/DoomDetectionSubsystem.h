/**
 * @file DoomDetectionSubsystem.h
 * @brief World subsystem running continuous background predictive 13s–40s trajectory lookahead ("The Void").
 *
 * @creator_feel
 * "If the engine calculates the player's trajectory will not intersect... trigger a dramatic
 *  failure state... in the next 13 to 40 seconds."
 * "The terror of space isn't just crashing; it's drifting helplessly. The game must mercilessly
 *  cut off doomed runs to keep the adrenaline high and pacing fast."
 *
 * @emotional_intent
 * Existential Dread. Drifting in dead space is an immediate death sentence. This subsystem
 * evaluates the player's future trajectory and snuffs out doomed runs instantly, preserving
 * the game's high APM and relentless tension.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "Solvers/DoomTrajectoryEvaluator.h"
#include "DoomDetectionSubsystem.generated.h"

class ASpaceAstronautCharacter;
class ASpaceObstacleSpawner;

/**
 * Delegate fired the exact frame predictive lookahead flags an unavoidable void soft-lock.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDoomDetected,
	const FDoomPredictionSnapshot&, DoomSnapshot
);

UCLASS()
class SPACEMENTUMCORE_API UDoomDetectionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UDoomDetectionSubsystem();

	// --- FTickableGameObject & Subsystem Lifecycle ---
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Manually triggers an evaluation pass of the player's trajectory. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Doom")
	bool EvaluateCurrentTrajectory(FDoomPredictionSnapshot& OutSnapshot);

	/** Returns the most recently computed predictive snapshot. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Doom")
	FDoomPredictionSnapshot GetLatestSnapshot() const { return LatestSnapshot; }

	/** True if the active run has been diagnosed as doomed to the void. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Doom")
	bool IsDoomImminent() const { return LatestSnapshot.bIsDoomImminent; }

	/** Sets whether the background doom lookahead checker is active. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Doom")
	void SetDoomDetectionEnabled(bool bEnabled) { bEnableDoomDetection = bEnabled; }

	/** Event broadcast upon void doom or velocity stall detection. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Doom")
	FOnDoomDetected OnDoomDetected;

protected:
	/** Minimum lookahead horizon in seconds (default 13.0s per Creator mandate). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Doom", meta = (ClampMin = "1.0"))
	float MinLookaheadSeconds = 13.0f;

	/** Maximum lookahead horizon in seconds (default 40.0s per Creator mandate). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Doom", meta = (ClampMin = "5.0"))
	float MaxLookaheadSeconds = 40.0f;

	/** Minimum forward/overall speed (cm/s) below which player is stalled in space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Doom", meta = (ClampMin = "0.0"))
	float StallSpeedThresholdCmPerSec = 100.0f;

	/** Extra reach tolerance (cm) added to tether radius for forgiving reach sweeps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Doom", meta = (ClampMin = "0.0"))
	float ReachToleranceMarginCm = 60.0f;

	/** Frequency of trajectory evaluations (seconds). Evaluates 5 times per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Doom", meta = (ClampMin = "0.05"))
	float EvaluationIntervalSeconds = 0.20f;

	/** Master toggle for doom detection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Doom")
	bool bEnableDoomDetection = true;

	/** Most recent predictive diagnosis snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Doom")
	FDoomPredictionSnapshot LatestSnapshot;

private:
	/** Accumulated time since last trajectory sweep. */
	float TimeSinceLastEvaluation = 0.0f;

	/** True once doom has fired for this run to avoid duplicate broadcasts. */
	bool bHasFiredDoomForRun = false;

	/** Helper to locate the active player pawn in the world. */
	ASpaceAstronautCharacter* FindAstronautPawn() const;

	/** Helper to locate the obstacle spawner in the world. */
	ASpaceObstacleSpawner* FindObstacleSpawner() const;
};
