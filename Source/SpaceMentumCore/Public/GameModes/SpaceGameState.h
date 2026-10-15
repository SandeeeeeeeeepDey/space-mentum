/**
 * @file SpaceGameState.h
 * @brief Replicated, observable state of the current run (phase, elapsed time, live score telemetry).
 *
 * @creator_feel
 * "ill include abilities and obsticles through time with flavour."
 *
 * @emotional_intent
 * Time and momentum are the core dimensions of the flight. This state tracks the live run phase,
 * flight duration, and coordinates with USpaceSectorProgressionSubsystem for score accumulation
 * and victory portal completion.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Types/SpaceMentumCoreTypes.h"
#include "Types/SpaceMentumProgressionTypes.h"
#include "SpaceGameState.generated.h"

/** Event broadcast whenever the run phase transitions. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnRunPhaseChanged,
	ESpaceRunPhase, NewPhase,
	ESpaceRunPhase, OldPhase
);

UCLASS()
class SPACEMENTUMCORE_API ASpaceGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASpaceGameState();

	virtual void Tick(float DeltaSeconds) override;

	/** Current phase of the run. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Run")
	ESpaceRunPhase GetRunPhase() const { return RunPhase; }

	/** Alias for GetRunPhase. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Run")
	ESpaceRunPhase GetCurrentRunPhase() const { return RunPhase; }

	/** Transitions the active run phase. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Run")
	void SetRunPhase(ESpaceRunPhase NewPhase);

	/** Total active flight time (seconds) in Running phase. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Run")
	float GetElapsedRunTimeSeconds() const { return ElapsedRunTimeSeconds; }

	/** Concludes the run in victory upon breaching the sector exit portal. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Run")
	void CompleteRunSuccess();

	/** Resets run timer and returns to LaunchPushOff. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Run")
	void ResetRun();

	/** Returns live flight score breakdown from the progression subsystem. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Run")
	FSectorFlightScore GetLiveFlightScore() const;

	/** Event broadcast on run phase transition. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Run")
	FOnRunPhaseChanged OnRunPhaseChanged;

protected:
	/** Current phase of the run. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Run")
	ESpaceRunPhase RunPhase = ESpaceRunPhase::LaunchPushOff;

	/** Total time spent in Running phase. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Run")
	float ElapsedRunTimeSeconds = 0.0f;
};
