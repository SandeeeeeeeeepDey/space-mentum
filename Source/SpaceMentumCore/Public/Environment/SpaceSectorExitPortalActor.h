/**
 * @file SpaceSectorExitPortalActor.h
 * @brief Google DeepMind-grade celestial jump portal actor terminating successful gauntlet runs.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand."
 * "abandon realistic space spacing. The levels need to be incredibly dense navigational hazards..."
 *
 * @emotional_intent
 * The Exit Jump Portal is the ultimate salvation. Hurtling at 108 km/h through dense asteroids
 * and shattered wreckage, seeing the swirling event horizon of the Jump Portal pierce the darkness
 * provides the ultimate triumphant climax to a sector run.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SpaceMentumProgressionTypes.h"
#include "SpaceSectorExitPortalActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPointLightComponent;
class USoundBase;

/** Delegate fired when the astronaut enters the jump portal event horizon. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSectorExitPortalBreached,
	int32, SectorIndex,
	const FVector&, BreachLocation,
	float, BreachSpeedCmPerSec
);

UCLASS()
class SPACEMENTUMCORE_API ASpaceSectorExitPortalActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceSectorExitPortalActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Configures portal parameters for the target sector. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Portal")
	void SetupPortal(const FSectorDefinition& InSectorDef);

	/** Resets portal state for a fresh run. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Portal")
	void ResetPortal();

	/** Event broadcast when astronaut enters event horizon. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Portal")
	FOnSectorExitPortalBreached OnPortalBreached;

	/** Returns whether portal has already been breached in this run. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Portal")
	bool HasBeenBreached() const { return bHasBeenBreached; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Massive outer metallic warp ring. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WarpRingMesh;

	/** Swirling inner cosmic accretion disk. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AccretionDiskMesh;

	/** Central glowing event horizon threshold. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> EventHorizonMesh;

	/** Overlap trigger volume detecting astronaut arrival. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Pulsating celestial illumination. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> PortalLight;

	/** Resonant hyperspace warp sound cue / Metasound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> WarpJumpSound;

	/** Active sector index associated with this portal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Portal")
	int32 TargetSectorIndex = 1;

	/** Emissive color tint of the event horizon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Portal")
	FLinearColor PortalColor = FLinearColor(0.0f, 0.95f, 1.0f);

	/** Rotation speed of the accretion disk (deg/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Portal")
	float AccretionDiskSpinSpeedDegPerSec = 45.0f;

	/** Has the portal already triggered in this run? */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Portal")
	bool bHasBeenBreached = false;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
