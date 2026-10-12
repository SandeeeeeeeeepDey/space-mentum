/**
 * @file SpaceGreyboxArenaActor.h
 * @brief Google DeepMind-grade procedural testing arena and greybox level harness (L_Prototype_Greybox).
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction to move away from obsticles."
 * "lets go for quality and hence unreal engine make most sense to me"
 *
 * @emotional_intent
 * The greybox arena is our experimental laboratory. It provides immediate spatial grounding
 * in the vastness of deep space. Cylindrical reference arches and distance milestones give
 * the eye an unambiguous speed anchor for the 30 m/s forward rush, while calibrated slalom
 * gates provide concrete evasion challenges to verify that the mace's centrifugal kick
 * provides crisp, predictable, satisfying trajectory shifts.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceGreyboxArenaActor.generated.h"

class USceneComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UInstancedStaticMeshComponent;
class ASpaceAstronautCharacter;

/**
 * Procedural parameters for the cylindrical reference corridor.
 */
USTRUCT(BlueprintType)
struct FGreyboxTunnelConfig
{
	GENERATED_BODY()

	/** Radius of the reference corridor cylinder in centimeters. Default = 800 cm (8 meters). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float CorridorRadiusCm = 800.0f;

	/** Total active length of the runway corridor in centimeters. Default = 60,000 cm (600 meters = 20s run). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float CorridorLengthCm = 60000.0f;

	/** Spacing between individual reference rings along the +X axis. Default = 3,000 cm (30m = 1 second at 30 m/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float RingIntervalCm = 3000.0f;

	/** Interval for high-visibility milestone arches in centimeters. Default = 15,000 cm (150m). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	float MilestoneIntervalCm = 15000.0f;

	/** Number of vertices / segments in each circular reference ring. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	int32 RingSegments = 32;

	/** Color of regular corridor reference rings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	FColor RegularRingColor = FColor(30, 45, 70);

	/** Color of milestone distance gates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corridor")
	FColor MilestoneRingColor = FColor(0, 180, 255);
};

/**
 * A calibrated slalom hazard target placed in space to test lateral dodging precision.
 */
USTRUCT(BlueprintType)
struct FGreyboxSlalomTarget
{
	GENERATED_BODY()

	/** Descriptive label for telemetry (e.g. "Gate 1: Hard Right"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slalom")
	FString TargetName = TEXT("Slalom Gate");

	/** Forward distance along the track (+X) in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slalom")
	float ForwardDistanceCm = 6000.0f;

	/** Calibrated lateral offset in the YZ plane (Y = right, Z = up) in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slalom")
	FVector2D LateralOffsetCm = FVector2D(250.0f, 0.0f);

	/** Clearance radius around the target in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slalom")
	float TargetRadiusCm = 120.0f;

	/** Target ring color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slalom")
	FColor TargetColor = FColor(255, 60, 60);

	/** Runtime flag: has the player already passed this gate? */
	UPROPERTY(Transient)
	bool bHasBeenEvaluated = false;
};

/**
 * Atmosphere and celestial lighting parameters for high-contrast readability against deep space.
 */
USTRUCT(BlueprintType)
struct FGreyboxLightingConfig
{
	GENERATED_BODY()

	/** Key cosmic rim light color (cyan cosmic tint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor KeyCosmicLightColor = FLinearColor(0.0f, 0.9f, 1.0f);

	/** Key light intensity (Lux). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	float KeyCosmicLightIntensity = 3.0f;

	/** Fill rim light color (ultraviolet nebula accent). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor RimLightColor = FLinearColor(0.7f, 0.0f, 1.0f);

	/** Sky ambient tint for deep space shadows. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor AmbientSkyColor = FLinearColor(0.02f, 0.03f, 0.06f);
};

/** Delegate fired when the astronaut clears or grazes a slalom gate. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnGreyboxSlalomGateEvaluated,
	const FString&, GateName,
	float, ClearanceDistanceCm,
	bool, bWasSuccessfulEvasion,
	FVector, AstronautVelocity);

/**
 * Procedural Greybox Level Arena Actor.
 * Automatically constructs the cylindrical reference corridor, milestone gates, and slalom targets.
 */
UCLASS()
class SPACEMENTUMCORE_API ASpaceGreyboxArenaActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceGreyboxArenaActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Resets all slalom gate evaluation states (useful on run restart). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Greybox")
	void ResetSlalomEvaluations();

	/** Event fired when the player passes a slalom gate. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Greybox")
	FOnGreyboxSlalomGateEvaluated OnSlalomGateEvaluated;

	// --- Configuration Accessors ---

	const FGreyboxTunnelConfig& GetTunnelConfig() const { return TunnelConfig; }
	const TArray<FGreyboxSlalomTarget>& GetSlalomTargets() const { return SlalomTargets; }

protected:
	/** Scene root component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Key celestial directional light (cyan star). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	TObjectPtr<UDirectionalLightComponent> KeyCosmicLight;

	/** Rim directional light (purple nebula). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	TObjectPtr<UDirectionalLightComponent> RimCosmicLight;

	/** Ambient space skylight. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	TObjectPtr<USkyLightComponent> CosmicSkyLight;

	/** Atmospheric deep space fog for depth cueing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	TObjectPtr<UExponentialHeightFogComponent> DeepSpaceFog;

	/** Corridor geometry parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Greybox")
	FGreyboxTunnelConfig TunnelConfig;

	/** Curated slalom test targets for mace kick evasion verification. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Greybox")
	TArray<FGreyboxSlalomTarget> SlalomTargets;

	/** Atmosphere and lighting configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Greybox")
	FGreyboxLightingConfig LightingConfig;

	/** Whether to draw in-viewport procedural debug visualization of the rings each frame. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Greybox")
	bool bEnableRuntimeDebugDrawing = true;

private:
	/** Applies the lighting configuration to the child lighting components. */
	void ApplyLightingConfiguration();

	/** Generates the default slalom calibration targets if empty. */
	void PopulateDefaultSlalomTargets();

	/** Evaluates player proximity against slalom gates each tick. */
	void EvaluateSlalomGates(float DeltaSeconds);

	/** Renders procedural tunnel rings in viewport using DrawDebugHelpers. */
	void DrawProceduralCorridorRings();

	/** Cached pointer to active astronaut pawn. */
	UPROPERTY(Transient)
	TWeakObjectPtr<ASpaceAstronautCharacter> CachedAstronaut;
};
