/**
 * @file SpaceMentumSaveGame.h
 * @brief Google DeepMind-grade native Unreal Engine 5.8 SaveGame persistence layer
 *        for SpaceMentum sector progression, flight scores, personal bests, and subjectivity settings.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand."
 * "lets go for quality and hence unreal engine make most sense to me"
 *
 * @emotional_intent
 * Persistence gives permanence to every harrowing flight. A player who masters the 1:4 internal
 * displacement and weaves cleanly through a claustrophobic asteroid gauntlet deserves to see their
 * personal best flight time, kinetic efficiency grade, and composite score permanently etched
 * into their pilot log.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Types/SpaceMentumProgressionTypes.h"
#include "SpaceMentumSaveGame.generated.h"

/**
 * Native persistence profile storing unlocked sectors, high scores, personal bests, and user settings.
 */
UCLASS()
class SPACEMENTUMCORE_API USpaceMentumSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	USpaceMentumSaveGame();

	/** Standard slot name used for disk persistence. */
	static const FString DefaultSaveSlotName;

	/** User index for multi-user local machines. */
	static const int32 DefaultUserIndex;

	// --- Profile Identity & Progression ---

	/** Registered pilot callsign. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|SaveProfile")
	FString PlayerCallsign = TEXT("Astronaut-77");

	/** Highest sector index unlocked by the player (1 = Sector 1, 2 = Sector 2, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|SaveProfile")
	int32 HighestUnlockedSectorIndex = 1;

	/** Map of SectorIndex -> Personal Best FSectorFlightScore. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|SaveProfile")
	TMap<int32, FSectorFlightScore> SectorPersonalBests;

	/** Map of SectorIndex -> FSectorLeaderboardTable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|SaveProfile")
	TMap<int32, FSectorLeaderboardTable> SectorLeaderboards;

	// --- User Subjectivity & Feel Preferences ---

	/** Whether Rule 4 "Blind Math" (no predictive simulation arcs) is strictly enforced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Preferences")
	bool bBlindMathEnforced = true;

	/** Master audio volume factor (0.0 to 1.0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Preferences")
	float MasterAudioVolume = 1.0f;

	/** Dynamic camera FOV dilation scaling factor (0.5 to 1.5). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Preferences")
	float FOVDilationScale = 1.0f;

	// --- Static Utility API ---

	/**
	 * Loads existing profile from disk or constructs a fresh one if none exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|SaveProfile")
	static USpaceMentumSaveGame* LoadOrCreateSaveGameProfile();

	/**
	 * Saves this profile instance to disk asynchronously or synchronously.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|SaveProfile")
	static bool SaveProfileToDisk(USpaceMentumSaveGame* SaveGameInstance);

	// --- Instance Methods ---

	/**
	 * Records a completed or attempted sector run.
	 * Updates personal best if new record, adds to leaderboard, and unlocks subsequent sector if completed.
	 *
	 * @param NewScore The flight score breakdown to record.
	 * @param bOutIsNewPersonalBest Outputs true if this score beat the previous record.
	 * @return True if recorded successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|SaveProfile")
	bool RecordRunScore(const FSectorFlightScore& NewScore, bool& bOutIsNewPersonalBest);

	/**
	 * Checks whether a sector is currently unlocked for flight.
	 */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|SaveProfile")
	bool IsSectorUnlocked(int32 SectorIndex) const;

	/**
	 * Unlocks a sector and saves the profile.
	 */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|SaveProfile")
	void UnlockSector(int32 SectorIndex);

	/**
	 * Retrieves personal best record for a sector, or default if none recorded yet.
	 */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|SaveProfile")
	FSectorFlightScore GetPersonalBestForSector(int32 SectorIndex) const;

	/**
	 * Retrieves the top leaderboard entries for a given sector.
	 */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|SaveProfile")
	TArray<FSectorLeaderboardEntry> GetLeaderboardForSector(int32 SectorIndex) const;
};
