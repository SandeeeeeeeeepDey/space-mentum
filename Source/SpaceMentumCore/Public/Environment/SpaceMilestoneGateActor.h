/**
 * @file SpaceMilestoneGateActor.h
 * @brief Google DeepMind-grade celestial milestone checkpoint actor placed along the sector corridor.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand."
 *
 * @emotional_intent
 * Milestones provide rhythmic progress anchors through high-density debris fields.
 * Crossing each celestial milestone provides a visual and auditory adrenaline rush,
 * affirming that the astronaut's trajectory is on track to reach the sector portal.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/SpaceMentumProgressionTypes.h"
#include "SpaceMilestoneGateActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPointLightComponent;
class USoundBase;

/** Delegate broadcast when the astronaut crosses this milestone arch. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMilestoneGateCrossed,
	int32, MilestoneIndex,
	const FString&, MilestoneTitle,
	float, ForwardDistanceCm
);

UCLASS()
class SPACEMENTUMCORE_API ASpaceMilestoneGateActor : public AActor
{
	GENERATED_BODY()

public:
	ASpaceMilestoneGateActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** Configures this milestone with data-driven parameters. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Milestone")
	void SetupMilestone(const FSectorMilestoneDefinition& InDefinition);

	/** Resets evaluation state for run restarts. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Milestone")
	void ResetMilestone();

	/** Delegate fired on threshold crossing. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Milestone")
	FOnMilestoneGateCrossed OnMilestoneCrossed;

	/** Returns the configuration definition. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Milestone")
	const FSectorMilestoneDefinition& GetMilestoneDefinition() const { return MilestoneConfig; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** High-visibility neon milestone arch ring. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ArchMeshComponent;

	/** Overlap trigger sensor detecting astronaut crossing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Cosmic neon accent light illuminating passing debris. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> MilestoneLight;

	/** Sound cue / Metasound played when milestone is crossed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> MilestoneCrossedSound;

	/** Milestone configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Milestone")
	FSectorMilestoneDefinition MilestoneConfig;

	/** Has the player already crossed this gate in the active run? */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Milestone")
	bool bHasBeenCrossed = false;

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
