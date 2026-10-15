// ==============================================================================
// SpaceFlightVFXComponent.h — Multi-system Niagara particle coordinator.
//
// @creator_feel
//   "a space person zooming in the forward direction in a uniform velocity and
//    the person has a mace in his hand."
//
// @emotional_intent
//   Orchestrates high-octane cosmic visuals: speed-reactive plasma ribbon trails
//   shifting from calm cyan to supersonic ultraviolet with angular velocity,
//   directional hyperspace dust streaks providing acute velocity feedback,
//   explosive kinetic impact spark bursts on obstacle strikes, and pneumatic launch exhausts.
// ==============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "SpaceFlightVFXComponent.generated.h"

class UMaceOrbitComponent;
class UMomentumTransferComponent;
class USpaceFlightInstrumentationComponent;

/**
 * Visual feel tuning parameters for Niagara particle systems.
 */
USTRUCT(BlueprintType)
struct FSpaceVFXTuning
{
	GENERATED_BODY()

	/** Trail color at resting / low angular velocity (cold electric cyan). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Trail")
	FLinearColor LowSpeedTrailColor = FLinearColor(0.0f, 0.96f, 0.83f, 1.0f);

	/** Trail color at peak 3.0 rev/s angular velocity (high-energy magenta/ultraviolet). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Trail")
	FLinearColor PeakSpeedTrailColor = FLinearColor(0.97f, 0.15f, 0.52f, 1.0f);

	/** Baseline ribbon width in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Trail")
	float BaseRibbonWidthCm = 20.0f;

	/** Ribbon width at maximum centrifugal tension. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Trail")
	float MaxRibbonWidthCm = 42.0f;

	/** Base spawn rate for hyperspace dust streaks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Dust")
	float BaseDustSpawnRate = 180.0f;

	/** Maximum spawn rate for hyperspace dust at peak velocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Dust")
	float MaxDustSpawnRate = 650.0f;

	/** Minimum forward speed cm/s before hyperspace dust streak stretch activates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFXFeel|Dust")
	float DustStreakThresholdSpeedCmPerSec = 800.0f;
};

/**
 * Component managing runtime Niagara systems and parameter feeds for the astronaut.
 */
UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMCORE_API USpaceFlightVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceFlightVFXComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Binds components and activates ribbon and dust emitters. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|VFX")
	void InitializeVFX(
		UMaceOrbitComponent* InOrbitComp,
		USceneComponent* InMaceHeadAttachSocket,
		USceneComponent* InAstronautRoot);

	/** Triggers a directional kinetic impact spark/plasma burst at obstacle collision point. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|VFX")
	void TriggerKineticImpactBurst(
		FVector ImpactPoint,
		FVector ImpactNormal,
		float ImpactEnergyRatio);

	/** Triggers an explosive hydraulic/pneumatic exhaust burst against the starting asteroid. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|VFX")
	void TriggerLaunchExhaustBurst(
		FVector AnchorPoint,
		FVector DepartureDirection,
		float ChargeRatio);

	// --- Asset References ---

	/** Soft pointer to Niagara plasma trail ribbon system (NS_MacePlasmaTrail). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX|Assets")
	TSoftObjectPtr<UNiagaraSystem> MacePlasmaTrailSystem;

	/** Soft pointer to Niagara hyperspace velocity streak dust (NS_HyperspaceDust). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX|Assets")
	TSoftObjectPtr<UNiagaraSystem> HyperspaceDustSystem;

	/** Soft pointer to Niagara kinetic spark burst system (NS_KineticImpactBurst). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX|Assets")
	TSoftObjectPtr<UNiagaraSystem> KineticImpactBurstSystem;

	/** Soft pointer to Niagara launch exhaust burst system (NS_LaunchExhaustBurst). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX|Assets")
	TSoftObjectPtr<UNiagaraSystem> LaunchExhaustBurstSystem;

	/** Tuning feel knobs for particle behaviour. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|VFX")
	FSpaceVFXTuning VFXTuning;

protected:
	virtual void BeginPlay() override;

private:
	/** Cached pointer to orbit component. */
	UPROPERTY(Transient)
	TObjectPtr<UMaceOrbitComponent> OrbitComp;

	/** Attached active Niagara component trailing the mace head. */
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveMaceTrailComponent;

	/** Attached active Niagara component generating directional hyperspace dust. */
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveHyperspaceDustComponent;
};
