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
 * and procedural greybox arena are immediately active and calibrated.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameModes/SpaceGameModeBase.h"
#include "SpaceGreyboxGameMode.generated.h"

class ASpaceGreyboxArenaActor;

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

	/** Returns the active greybox arena actor in the world. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Greybox")
	ASpaceGreyboxArenaActor* GetGreyboxArena() const { return ActiveArena; }

protected:
	/** Active greybox arena actor. Automatically spawned if none was manually placed in the level. */
	UPROPERTY(Transient)
	TObjectPtr<ASpaceGreyboxArenaActor> ActiveArena;
};
