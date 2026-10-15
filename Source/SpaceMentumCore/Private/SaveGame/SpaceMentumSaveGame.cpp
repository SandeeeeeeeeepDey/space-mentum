/**
 * @file SpaceMentumSaveGame.cpp
 * @brief Implementation of native SaveGame persistence for sector runs, scores, and flight settings.
 */

#include "SaveGame/SpaceMentumSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"

const FString USpaceMentumSaveGame::DefaultSaveSlotName = TEXT("SpaceMentumFlightProfile");
const int32 USpaceMentumSaveGame::DefaultUserIndex = 0;

USpaceMentumSaveGame::USpaceMentumSaveGame()
{
	PlayerCallsign = TEXT("Astronaut-77");
	HighestUnlockedSectorIndex = 1;
	bBlindMathEnforced = true;
	MasterAudioVolume = 1.0f;
	FOVDilationScale = 1.0f;
}

USpaceMentumSaveGame* USpaceMentumSaveGame::LoadOrCreateSaveGameProfile()
{
	if (UGameplayStatics::DoesSaveGameExist(DefaultSaveSlotName, DefaultUserIndex))
	{
		USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(DefaultSaveSlotName, DefaultUserIndex);
		USpaceMentumSaveGame* Profile = Cast<USpaceMentumSaveGame>(LoadedGame);
		if (Profile)
		{
			UE_LOG(LogTemp, Log, TEXT("[SpaceMentumSaveGame] Successfully loaded profile: Pilot %s | Unlocked Sector: %d"),
				*Profile->PlayerCallsign, Profile->HighestUnlockedSectorIndex);
			return Profile;
		}
	}

	// Create fresh profile
	USaveGame* NewGame = UGameplayStatics::CreateSaveGameObject(USpaceMentumSaveGame::StaticClass());
	USpaceMentumSaveGame* NewProfile = Cast<USpaceMentumSaveGame>(NewGame);
	if (NewProfile)
	{
		UE_LOG(LogTemp, Log, TEXT("[SpaceMentumSaveGame] Constructing fresh pilot profile: %s"), *NewProfile->PlayerCallsign);
		SaveProfileToDisk(NewProfile);
		return NewProfile;
	}

	return nullptr;
}

bool USpaceMentumSaveGame::SaveProfileToDisk(USpaceMentumSaveGame* SaveGameInstance)
{
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpaceMentumSaveGame] SaveProfileToDisk failed: null instance."));
		return false;
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, DefaultSaveSlotName, DefaultUserIndex);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[SpaceMentumSaveGame] Successfully persisted profile to slot '%s'."), *DefaultSaveSlotName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SpaceMentumSaveGame] Failed to write profile to slot '%s'."), *DefaultSaveSlotName);
	}

	return bSuccess;
}

bool USpaceMentumSaveGame::RecordRunScore(const FSectorFlightScore& NewScore, bool& bOutIsNewPersonalBest)
{
	bOutIsNewPersonalBest = false;

	// 1. Evaluate Personal Best
	const FSectorFlightScore* ExistingPB = SectorPersonalBests.Find(NewScore.SectorIndex);
	if (!ExistingPB || NewScore.CompositeScore > ExistingPB->CompositeScore)
	{
		SectorPersonalBests.Add(NewScore.SectorIndex, NewScore);
		bOutIsNewPersonalBest = true;
		UE_LOG(LogTemp, Log, TEXT("[SpaceMentumSaveGame] NEW PERSONAL BEST for Sector %d: %d pts (Grade %s)!"),
			NewScore.SectorIndex, NewScore.CompositeScore, *UEnum::GetValueAsString(NewScore.PerformanceGrade));
	}

	// 2. Insert into Leaderboard Table
	FSectorLeaderboardTable& Table = SectorLeaderboards.FindOrAdd(NewScore.SectorIndex);
	Table.SectorIndex = NewScore.SectorIndex;

	FSectorLeaderboardEntry Entry;
	Entry.PlayerCallsign = PlayerCallsign;
	Entry.CompositeScore = NewScore.CompositeScore;
	Entry.PerformanceGrade = NewScore.PerformanceGrade;
	Entry.FlightTimeSeconds = NewScore.ElapsedTimeSeconds;
	Entry.PeakSpeedKmh = NewScore.PeakSpeedCmPerSec * 0.036f;
	Entry.GatesRatio = FString::Printf(TEXT("%d/%d"), NewScore.CleanGatesCleared, NewScore.TotalGatesCount);
	Entry.TimestampString = FDateTime::UtcNow().ToString(TEXT("%Y-%m-%d %H:%M UTC"));

	Table.TopEntries.Add(Entry);

	// Sort descending by score
	Table.TopEntries.Sort([](const FSectorLeaderboardEntry& A, const FSectorLeaderboardEntry& B) {
		return A.CompositeScore > B.CompositeScore;
	});

	// Keep top 15 records
	if (Table.TopEntries.Num() > 15)
	{
		Table.TopEntries.SetNum(15);
	}

	// 3. Unlock Next Sector on Successful Completion
	if (NewScore.bIsCompleted && NewScore.SectorIndex >= HighestUnlockedSectorIndex)
	{
		HighestUnlockedSectorIndex = NewScore.SectorIndex + 1;
		UE_LOG(LogTemp, Log, TEXT("[SpaceMentumSaveGame] Sector %d completed! Unlocked Sector %d!"),
			NewScore.SectorIndex, HighestUnlockedSectorIndex);
	}

	// Persist changes
	return SaveProfileToDisk(this);
}

bool USpaceMentumSaveGame::IsSectorUnlocked(int32 SectorIndex) const
{
	return SectorIndex <= HighestUnlockedSectorIndex;
}

void USpaceMentumSaveGame::UnlockSector(int32 SectorIndex)
{
	if (SectorIndex > HighestUnlockedSectorIndex)
	{
		HighestUnlockedSectorIndex = SectorIndex;
		SaveProfileToDisk(this);
	}
}

FSectorFlightScore USpaceMentumSaveGame::GetPersonalBestForSector(int32 SectorIndex) const
{
	if (const FSectorFlightScore* PB = SectorPersonalBests.Find(SectorIndex))
	{
		return *PB;
	}

	FSectorFlightScore DefaultPB;
	DefaultPB.SectorIndex = SectorIndex;
	return DefaultPB;
}

TArray<FSectorLeaderboardEntry> USpaceMentumSaveGame::GetLeaderboardForSector(int32 SectorIndex) const
{
	if (const FSectorLeaderboardTable* Table = SectorLeaderboards.Find(SectorIndex))
	{
		return Table->TopEntries;
	}

	return TArray<FSectorLeaderboardEntry>();
}
