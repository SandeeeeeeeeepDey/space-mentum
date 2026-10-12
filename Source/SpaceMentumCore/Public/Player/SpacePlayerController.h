/**
 * @file SpacePlayerController.h
 * @brief Translates the player's two spin buttons (A and D) into responsive, zero-latency mace commands.
 *
 * @creator_feel
 * "for pc A and D looks good. only two buttons for now to spin the mace, moment of
 *  stopping is the moment of release. everything can be controles by this i think."
 *
 * @emotional_intent
 * Two buttons, one gesture. Hold A or D to wind the mace; let go and the stored spin
 * becomes your escape. The controller adds ZERO latency or smoothing between the
 * finger lifting and the kick firing (GDD §2.1 F-2 "sub-16ms deterministic response").
 *
 * Features:
 *  - Procedural Enhanced Input setup in C++ (works out of the box without editor assets).
 *  - SOCD Last-Input-Priority resolution when both A and D are pressed simultaneously.
 *  - Instant fallback when one button is released while the other is still held.
 *  - Optional 'M' key binding to toggle between ReactionRecoil and CarryAlong kick models.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "SpacePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ASpaceAstronautCharacter;

UCLASS()
class SPACEMENTUMCORE_API ASpacePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASpacePlayerController();

	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Toggles between ReactionRecoil and CarryAlong momentum kick models on possessed astronaut. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Input")
	void ToggleKickModel();

protected:
	// --- Enhanced Input Actions & Context ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Input")
	TObjectPtr<UInputAction> SpinCounterClockwiseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Input")
	TObjectPtr<UInputAction> SpinClockwiseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Input")
	TObjectPtr<UInputAction> ToggleKickModelAction;

	// --- Internal Action Handlers ---

	void OnSpinCounterClockwiseStarted();
	void OnSpinCounterClockwiseCompleted();
	void OnSpinClockwiseStarted();
	void OnSpinClockwiseCompleted();
	void OnToggleKickModelTriggered();

	/** Resolves simultaneous button holds using Last-Input-Priority and updates the pawn. */
	void EvaluateActiveSpin();

private:
	/** Tracks whether Counter-Clockwise (A key) is currently held. */
	bool bIsCounterClockwisePressed = false;

	/** Tracks whether Clockwise (D key) is currently held. */
	bool bIsClockwisePressed = false;

	/** Timestamp of last Counter-Clockwise press for SOCD resolution. */
	double LastCounterClockwisePressTime = 0.0;

	/** Timestamp of last Clockwise press for SOCD resolution. */
	double LastClockwisePressTime = 0.0;

	/** Helper to build default Enhanced Input assets procedurally in memory if not assigned. */
	void EnsureDefaultInputBindingsExist();
};
