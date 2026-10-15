/**
 * @file SpaceGreyboxGameMode.h
 * @brief Testbed game mode specifically configured for greybox calibration and in-editor feel validation.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity and the person has a mace in his hand."
 * "lets go for quality and hence unreal engine make most sense to me"
 *
 * @emotional_intent
 * Instant playability with zero setup friction. When an engineer or designer opens any test map
 * or hits Play In Editor (PIE), this game mode ensures the astronaut, controller, HUD telemetry,
 * procedural greybox arena, milestone arches, and sector exit jump portal are immediately active and calibrated.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameModes/SpaceGameModeBase.h"
#include "SpaceGreyboxGameMode.generated.h"

class ASpaceGreyboxArenaActor;
class ASpaceObstacleSpawner;
class ASpaceStartingAsteroidActor;
class ASpaceSectorExitPortalActor;
class ASpaceMilestoneGateActor;

UCLASS()
class SPACEMENTUMCORE_API ASpaceGreyboxGameMode : public ASpaceGameModeBase
{
	GENERATED_BODY()

public:
	ASpaceGreyboxGameMode();

	virtual void BeginPlay() override;

	/** Resets the run: repositions the astronaut at the start gate and resets slalom evaluations. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Greybox")
	void RestartRun();

	/** Advances to the next sequential sector and reconfigures track length and portal. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Greybox")
	void AdvanceToNextSector();

	/** Returns the active greybox arena actor in the world. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Greybox")
	ASpaceGreyboxArenaActor* GetGreyboxArena() const { return ActiveArena; }

	/** Returns the active obstacle spawner in the world. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Greybox")
	ASpaceObstacleSpawner* GetObstacleSpawner() const { return ActiveObstacleSpawner; }

	/** Returns the starting celestial asteroid in the world. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Greybox")
	ASpaceStartingAsteroidActor* GetStartingAsteroid() const { return ActiveStartingAsteroid; }

	/** Returns the active sector exit portal terminating the run. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Greybox")
	ASpaceSectorExitPortalActor* GetSectorExitPortal() const { return ActiveExitPortal; }

	/** Returns active milestone gate actors along the corridor. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Greybox")
	TArray<ASpaceMilestoneGateActor*> GetMilestoneGates() const { return ActiveMilestones; }

protected:
	/** Active greybox arena actor. Automatically spawned if none was manually placed in the level. */
	UPROPERTY(Transient)
	TObjectPtr<ASpaceGreyboxArenaActor> ActiveArena;

	/** Active obstacle spawner. Automatically spawned if none was manually placed in the level. */
	UPROPERTY(Transient)
	TObjectPtr<ASpaceObstacleSpawner> ActiveObstacleSpawner;

	/** Active celestial starting asteroid anchor at X = 0. */
	UPROPERTY(Transient)
	TObjectPtr<ASpaceStartingAsteroidActor> ActiveStartingAsteroid;

	/** Active celestial jump portal terminating successful gauntlet runs. */
	UPROPERTY(Transient)
	TObjectPtr<ASpaceSectorExitPortalActor> ActiveExitPortal;

	/** Active milestone checkpoint gates. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<ASpaceMilestoneGateActor>> ActiveMilestones;

private:
	void SpawnOrFindExitPortal();
	void SpawnOrFindMilestones();

	UFUNCTION()
	void HandleSlalomGateEvaluated(const FString& GateName, float ClearanceDistanceCm, bool bWasSuccessfulEvasion, FVector Velocity);
};
