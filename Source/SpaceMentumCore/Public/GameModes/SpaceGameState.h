/**
 * @file SpaceGameState.h
 * @brief Replicated, observable state of the current run (phase, elapsed time).
 *
 * @creator_feel
 * "ill include abilities and obsticles through time with flavour."
 *
 * @emotional_intent
 * Time is the flavour dial. Elapsed run time will drive which obstacle archetypes
 * enter the corridor (UObstacleArchetypeDataAsset::MinRunTimeSeconds), so the run
 * escalates naturally.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Types/SpaceMentumCoreTypes.h"
#include "SpaceGameState.generated.h"

UCLASS()
class SPACEMENTUMCORE_API ASpaceGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** Current phase of the run. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Run")
	ESpaceRunPhase GetRunPhase() const { return RunPhase; }

protected:
	/** Phase transitions are driven by the game mode in Step 4 (obstacles & fail state). */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Run")
	ESpaceRunPhase RunPhase = ESpaceRunPhase::PreLaunch;
};
