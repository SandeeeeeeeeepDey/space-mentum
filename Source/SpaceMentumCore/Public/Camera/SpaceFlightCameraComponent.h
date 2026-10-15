// ==============================================================================
// SpaceFlightCameraComponent.h — Cinematic dynamic flight camera rig.
//
// @creator_feel
//   "a space person zooming in the forward direction in a uniform velocity and
//    the person has a mace in his hand."
//
// @emotional_intent
//   Cinematic chase camera delivering visceral physical feedback without inducing
//   simulator sickness. Features speed-reactive FOV widening (tunnels of hyperspace),
//   angular windup centrifugal pull, instantaneous whiplash kick FOV punch, dynamic
//   lateral Dutch roll during violent evasions, and trauma-based screen shake on kinetic impacts.
// ==============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceFlightCameraComponent.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ASpaceAstronautCharacter;

/**
 * Feel knobs for dynamic flight camera responsiveness.
 */
USTRUCT(BlueprintType)
struct FSpaceCameraTuning
{
	GENERATED_BODY()

	/** Baseline field of view when cruising at resting forward speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|FOV")
	float BaseFovDeg = 80.0f;

	/** Maximum additional FOV dilation at extreme forward or launch speeds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|FOV")
	float MaxSpeedFovDilationDeg = 25.0f;

	/** Forward speed in cm/s where speed-based FOV dilation reaches 100%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|FOV")
	float ReferenceHyperspaceSpeedCmPerSec = 3600.0f;

	/** Instantaneous FOV kick burst when releasing mace or launching from asteroid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|FOV")
	float KickFovPunchDeg = 14.0f;

	/** Smooth interpolation rate for FOV transitions (higher = snappier). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|FOV")
	float FovInterpSpeed = 4.5f;

	/** Baseline chase spring arm distance behind the astronaut. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Distance")
	float BaseArmLengthCm = 450.0f;

	/** Maximum camera pull-back distance at peak velocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Distance")
	float MaxSpeedArmLengthDilationCm = 120.0f;

	/** Smooth interpolation rate for spring arm length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Distance")
	float ArmLengthInterpSpeed = 3.0f;

	/** Maximum camera Dutch roll tilt in degrees during violent lateral momentum evasion kicks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Roll")
	float MaxDutchRollDeg = 4.0f;

	/** Lateral speed in cm/s yielding maximum Dutch roll tilt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Roll")
	float ReferenceLateralRollSpeedCmPerSec = 1600.0f;

	/** Smooth interpolation rate for camera roll recovery. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Roll")
	float RollInterpSpeed = 6.0f;

	/** Exponential decay rate for impact camera trauma. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Trauma")
	float TraumaDecayRate = 3.0f;

	/** Maximum rotational camera shake degrees from trauma. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraFeel|Trauma")
	float MaxTraumaShakeDeg = 2.5f;
};

/**
 * Dynamic chase camera component attached to the astronaut pawn.
 * Continuously evaluates forward velocity, rotational windup, and lateral whiplash
 * to animate the camera FOV, boom distance, roll tilt, and screen trauma.
 */
UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMCORE_API USpaceFlightCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceFlightCameraComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Initializes bindings with the astronaut's spring arm and camera components. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Camera")
	void InitializeCameraRig(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	/** Adds an instantaneous FOV punch (e.g. on momentum release or asteroid launch). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Camera")
	void TriggerFovPunch(float PunchMagnitudeRatio);

	/** Adds trauma (0.0 to 1.0) on kinetic obstacle strikes or close-call near misses. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Camera")
	void AddTrauma(float InTrauma);

	/** Returns current active FOV. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Camera")
	float GetCurrentFieldOfView() const { return CurrentFov; }

	/** Accessor for dynamic camera tuning parameters. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Camera")
	const FSpaceCameraTuning& GetCameraTuning() const { return CameraTuning; }

	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Camera")
	void SetCameraTuning(const FSpaceCameraTuning& InTuning) { CameraTuning = InTuning; }

protected:
	virtual void BeginPlay() override;

	/** Camera feel tuning parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Camera")
	FSpaceCameraTuning CameraTuning;

private:
	/** Cached pointer to the astronaut's chase spring arm. */
	UPROPERTY(Transient)
	TObjectPtr<USpringArmComponent> SpringArmComp;

	/** Cached pointer to the astronaut's chase camera. */
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CameraComp;

	/** Cached pointer to owning astronaut. */
	UPROPERTY(Transient)
	TObjectPtr<ASpaceAstronautCharacter> OwningAstronaut;

	/** Current smoothed FOV value. */
	float CurrentFov = 80.0f;

	/** Target FOV computed this tick. */
	float TargetFov = 80.0f;

	/** Additional FOV impulse from recent kicks decaying exponentially. */
	float FovPunchOffset = 0.0f;

	/** Current smoothed spring arm length. */
	float CurrentArmLength = 450.0f;

	/** Current smoothed Dutch roll angle in degrees. */
	float CurrentRollDeg = 0.0f;

	/** Camera trauma accumulator [0.0, 1.0] for non-linear screen shake ($shake = trauma^2$). */
	float Trauma = 0.0f;

	/** Trauma shake time accumulator. */
	float TraumaTime = 0.0f;
};
