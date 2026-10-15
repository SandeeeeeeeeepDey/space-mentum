/**
 * @file SpaceSectorProgressionSubsystem.h
 * @brief Google DeepMind-grade GameInstance Subsystem managing multi-sector catalog,
 *        flight score telemetry accumulation, milestone objectives, and SaveGame persistence.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand."
 * "ill include abilities and obsticles through time with flavour."
 *
 * @emotional_intent
 * Persisting across map unloads and sector jumps, this subsystem is the overarching mission director.
 * It tracks the live kinetic telemetry of the astronaut, evaluates clean evasion streaks and momentum
 * strike multipliers, grades performance against par times, and etches records into the pilot profile.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/SpaceMentumProgressionTypes.h"
#include "SpaceSectorProgressionSubsystem.generated.h"

class USpaceMentumSaveGame;

/** Broadcast when an active sector run finishes (completed or terminated). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSectorRunCompletedSignature,
	const FSectorFlightScore&, FinalScore,
	bool, bIsNewPersonalBest
);

/** Broadcast when a milestone gate is achieved in the corridor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMilestoneAchievedSignature,
	int32, MilestoneIndex,
	const FString&, MilestoneTitle,
	int32, ScoreBonus
);

UCLASS()
class SPACEMENTUMCORE_API USpaceSectorProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USpaceSectorProgressionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Sector Catalog & Progression ---

	/** Returns definition for a specific sector index. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Progression")
	FSectorDefinition GetSectorDefinition(int32 SectorIndex) const;

	/** Returns all registered sectors in the catalog. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Progression")
	TArray<FSectorDefinition> GetAllSectors() const;

	/** Gets active sector index. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Progression")
	int32 GetActiveSectorIndex() const { return ActiveSectorIndex; }

	/** Sets active sector index. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void SetActiveSectorIndex(int32 NewSectorIndex);

	// --- Run Telemetry Accumulation ---

	/** Initializes a fresh run for the target sector. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void StartNewSectorRun(int32 SectorIndex);

	/** Records an evasion gate crossing (clean or grazed). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void RecordSlalomGateResult(const FString& GateName, bool bCleanEvasion, float ClearanceCm);

	/** Records milestone checkpoint achieved. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void RecordMilestoneReached(int32 MilestoneIndex, const FString& MilestoneTitle, float DistanceCm);

	/** Records external strike landed on an obstacle. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void RecordExternalStrikeLanded(float StrikeImpulseMag, const FString& ObstacleName);

	/** Samples instantaneous flight speed for average calculation. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void SampleFlightSpeed(float SpeedCmPerSec);

	/** Updates active elapsed time and total distance traveled. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	void UpdateFlightProgress(float DeltaSeconds, float ForwardDistanceCm);

	/** Concludes the run upon entering exit portal or reaching terminus. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	FSectorFlightScore CompleteActiveSectorRun(bool bBreachedExitPortal);

	/** Terminates run on lethal crash or void doom. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Progression")
	FSectorFlightScore TerminateActiveSectorRunFailed(bool bLostToVoid);

	/** Returns current live score snapshot. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Progression")
	FSectorFlightScore GetCurrentFlightScore() const { return ActiveScore; }

	/** Returns persistent SaveGame profile. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Progression")
	USpaceMentumSaveGame* GetSaveProfile() const { return ActiveSaveProfile; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Progression")
	FOnSectorRunCompletedSignature OnSectorRunCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Progression")
	FOnMilestoneAchievedSignature OnMilestoneAchieved;

protected:
	/** Catalog of all available celestial sectors. */
	UPROPERTY(EditDefaultsOnly, Category = "SpaceMentum|Progression")
	TMap<int32, FSectorDefinition> SectorCatalog;

	/** Active sector currently being played. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Progression")
	int32 ActiveSectorIndex = 1;

	/** In-flight telemetry and scoring accumulator. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Progression")
	FSectorFlightScore ActiveScore;

	/** Speed sampling accumulator for average velocity computation. */
	float SpeedSampleSum = 0.0f;
	int32 SpeedSampleCount = 0;

	/** Cached active SaveGame profile. */
	UPROPERTY(Transient)
	TObjectPtr<USpaceMentumSaveGame> ActiveSaveProfile;

private:
	void PopulateDefaultSectorCatalog();
};
