/**
 * @file SpaceSectorProgressionSubsystem.cpp
 * @brief Implementation of sector progression, telemetry accumulation, and high-score persistence.
 */

#include "Subsystems/SpaceSectorProgressionSubsystem.h"
#include "SaveGame/SpaceMentumSaveGame.h"

USpaceSectorProgressionSubsystem::USpaceSectorProgressionSubsystem()
{
	ActiveSectorIndex = 1;
	SpeedSampleSum = 0.0f;
	SpeedSampleCount = 0;
}

void USpaceSectorProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PopulateDefaultSectorCatalog();
	ActiveSaveProfile = USpaceMentumSaveGame::LoadOrCreateSaveGameProfile();
	StartNewSectorRun(1);

	UE_LOG(LogTemp, Log, TEXT("[SpaceSectorProgressionSubsystem] Initialized with %d sectors. Active Pilot: %s"),
		SectorCatalog.Num(), ActiveSaveProfile ? *ActiveSaveProfile->PlayerCallsign : TEXT("Unknown"));
}

void USpaceSectorProgressionSubsystem::Deinitialize()
{
	if (ActiveSaveProfile)
	{
		USpaceMentumSaveGame::SaveProfileToDisk(ActiveSaveProfile);
	}

	Super::Deinitialize();
}

void USpaceSectorProgressionSubsystem::PopulateDefaultSectorCatalog()
{
	SectorCatalog.Empty();

	// Sector 1: Perseus Shallows
	{
		FSectorDefinition Sec1;
		Sec1.SectorIndex = 1;
		Sec1.SectorName = TEXT("Sector 1: Perseus Shallows");
		Sec1.BriefingText = TEXT("Calibrated orbital testing grounds with spaced debris and 5 slalom gates.");
		Sec1.CorridorLengthCm = 120000.0f; // 1.2 km
		Sec1.CorridorRadiusCm = 800.0f;
		Sec1.ParTimeSeconds = 38.0f;
		Sec1.HazardDensityMultiplier = 1.0f;
		Sec1.BaseForwardDriftSpeedCmPerSec = 3000.0f;
		Sec1.PortalThemeColor = FLinearColor(0.0f, 0.95f, 1.0f); // Cyan
		SectorCatalog.Add(1, Sec1);
	}

	// Sector 2: Kepler Graveyard
	{
		FSectorDefinition Sec2;
		Sec2.SectorIndex = 2;
		Sec2.SectorName = TEXT("Sector 2: Kepler Graveyard");
		Sec2.BriefingText = TEXT("Dense orbital wreckage and shattered solar arrays requiring precise deflection strikes.");
		Sec2.CorridorLengthCm = 150000.0f; // 1.5 km
		Sec2.CorridorRadiusCm = 750.0f;
		Sec2.ParTimeSeconds = 48.0f;
		Sec2.HazardDensityMultiplier = 1.35f;
		Sec2.BaseForwardDriftSpeedCmPerSec = 3200.0f;
		Sec2.PortalThemeColor = FLinearColor(1.0f, 0.45f, 0.05f); // Solar Amber
		SectorCatalog.Add(2, Sec2);
	}

	// Sector 3: Tartarus Belt
	{
		FSectorDefinition Sec3;
		Sec3.SectorIndex = 3;
		Sec3.SectorName = TEXT("Sector 3: Tartarus Belt");
		Sec3.BriefingText = TEXT("Claustrophobic iron-nickel asteroid gauntlet. Zero error tolerance.");
		Sec3.CorridorLengthCm = 180000.0f; // 1.8 km
		Sec3.CorridorRadiusCm = 700.0f;
		Sec3.ParTimeSeconds = 56.0f;
		Sec3.HazardDensityMultiplier = 1.75f;
		Sec3.BaseForwardDriftSpeedCmPerSec = 3400.0f;
		Sec3.PortalThemeColor = FLinearColor(0.85f, 0.05f, 0.95f); // Deep Magenta
		SectorCatalog.Add(3, Sec3);
	}
}

FSectorDefinition USpaceSectorProgressionSubsystem::GetSectorDefinition(int32 SectorIndex) const
{
	if (const FSectorDefinition* Def = SectorCatalog.Find(SectorIndex))
	{
		return *Def;
	}

	// Default fallback to Sector 1
	if (const FSectorDefinition* Def1 = SectorCatalog.Find(1))
	{
		return *Def1;
	}

	FSectorDefinition Fallback;
	Fallback.SectorIndex = SectorIndex;
	return Fallback;
}

TArray<FSectorDefinition> USpaceSectorProgressionSubsystem::GetAllSectors() const
{
	TArray<FSectorDefinition> Sectors;
	SectorCatalog.GenerateValueArray(Sectors);
	Sectors.Sort([](const FSectorDefinition& A, const FSectorDefinition& B) {
		return A.SectorIndex < B.SectorIndex;
	});
	return Sectors;
}

void USpaceSectorProgressionSubsystem::SetActiveSectorIndex(int32 NewSectorIndex)
{
	if (SectorCatalog.Contains(NewSectorIndex))
	{
		ActiveSectorIndex = NewSectorIndex;
		StartNewSectorRun(NewSectorIndex);
	}
}

void USpaceSectorProgressionSubsystem::StartNewSectorRun(int32 SectorIndex)
{
	ActiveSectorIndex = SectorIndex;
	const FSectorDefinition Def = GetSectorDefinition(SectorIndex);

	ActiveScore = FSectorFlightScore();
	ActiveScore.SectorIndex = SectorIndex;
	ActiveScore.SectorName = Def.SectorName;
	ActiveScore.bIsCompleted = false;
	ActiveScore.TotalGatesCount = 5;

	SpeedSampleSum = 0.0f;
	SpeedSampleCount = 0;

	UE_LOG(LogTemp, Log, TEXT("[SpaceSectorProgressionSubsystem] Begun new run on %s (Target Distance: %.1f m)"),
		*Def.SectorName, Def.CorridorLengthCm * 0.01f);
}

void USpaceSectorProgressionSubsystem::RecordSlalomGateResult(const FString& GateName, bool bCleanEvasion, float ClearanceCm)
{
	if (bCleanEvasion)
	{
		ActiveScore.CleanGatesCleared++;
		UE_LOG(LogTemp, Log, TEXT("[SpaceSectorProgressionSubsystem] Gate %s Clean Evasion! (+%.1f cm) Total Cleared: %d"),
			*GateName, ClearanceCm, ActiveScore.CleanGatesCleared);
	}
	else
	{
		ActiveScore.GrazeBoundaryPenalties++;
		UE_LOG(LogTemp, Warning, TEXT("[SpaceSectorProgressionSubsystem] Gate %s Graze Penalty! (-%.1f cm) Total Grazes: %d"),
			*GateName, ClearanceCm, ActiveScore.GrazeBoundaryPenalties);
	}
}

void USpaceSectorProgressionSubsystem::RecordMilestoneReached(int32 MilestoneIndex, const FString& MilestoneTitle, float DistanceCm)
{
	const int32 ScoreBonus = 500 * MilestoneIndex;

	UE_LOG(LogTemp, Log, TEXT("[SpaceSectorProgressionSubsystem] Milestone %d Reached: %s at %.1f m (+%d pts)"),
		MilestoneIndex, *MilestoneTitle, DistanceCm * 0.01f, ScoreBonus);

	if (OnMilestoneAchieved.IsBound())
	{
		OnMilestoneAchieved.Broadcast(MilestoneIndex, MilestoneTitle, ScoreBonus);
	}
}

void USpaceSectorProgressionSubsystem::RecordExternalStrikeLanded(float StrikeImpulseMag, const FString& ObstacleName)
{
	ActiveScore.ExternalStrikesLanded++;
	UE_LOG(LogTemp, Log, TEXT("[SpaceSectorProgressionSubsystem] Strike Landed on %s! Total Strikes: %d"),
		*ObstacleName, ActiveScore.ExternalStrikesLanded);
}

void USpaceSectorProgressionSubsystem::SampleFlightSpeed(float SpeedCmPerSec)
{
	SpeedSampleSum += SpeedCmPerSec;
	SpeedSampleCount++;
	ActiveScore.AverageSpeedCmPerSec = SpeedSampleSum / SpeedSampleCount;
	ActiveScore.PeakSpeedCmPerSec = FMath::Max(ActiveScore.PeakSpeedCmPerSec, SpeedCmPerSec);
}

void USpaceSectorProgressionSubsystem::UpdateFlightProgress(float DeltaSeconds, float ForwardDistanceCm)
{
	ActiveScore.ElapsedTimeSeconds += DeltaSeconds;
	ActiveScore.TotalDistanceTraveledCm = ForwardDistanceCm;
}

FSectorFlightScore USpaceSectorProgressionSubsystem::CompleteActiveSectorRun(bool bBreachedExitPortal)
{
	ActiveScore.bIsCompleted = bBreachedExitPortal;

	ActiveScore.CompositeScore = FSectorFlightScore::ComputeCompositeScore(
		ActiveScore.TotalDistanceTraveledCm,
		ActiveScore.ElapsedTimeSeconds,
		ActiveScore.AverageSpeedCmPerSec,
		ActiveScore.CleanGatesCleared,
		ActiveScore.ExternalStrikesLanded,
		ActiveScore.GrazeBoundaryPenalties,
		bBreachedExitPortal
	);

	ActiveScore.PerformanceGrade = FSectorFlightScore::EvaluateGrade(ActiveScore.CompositeScore, bBreachedExitPortal);

	bool bIsNewPB = false;
	if (ActiveSaveProfile)
	{
		ActiveSaveProfile->RecordRunScore(ActiveScore, bIsNewPB);
	}

	UE_LOG(LogTemp, Log, TEXT("[SpaceSectorProgressionSubsystem] Sector %d Run Completed! Score: %d | Grade: %s | New PB: %s"),
		ActiveScore.SectorIndex, ActiveScore.CompositeScore,
		*UEnum::GetValueAsString(ActiveScore.PerformanceGrade),
		bIsNewPB ? TEXT("YES") : TEXT("NO"));

	if (OnSectorRunCompleted.IsBound())
	{
		OnSectorRunCompleted.Broadcast(ActiveScore, bIsNewPB);
	}

	return ActiveScore;
}

FSectorFlightScore USpaceSectorProgressionSubsystem::TerminateActiveSectorRunFailed(bool bLostToVoid)
{
	ActiveScore.bIsCompleted = false;
	ActiveScore.PerformanceGrade = EKineticFlightGrade::Failed;

	ActiveScore.CompositeScore = FSectorFlightScore::ComputeCompositeScore(
		ActiveScore.TotalDistanceTraveledCm,
		ActiveScore.ElapsedTimeSeconds,
		ActiveScore.AverageSpeedCmPerSec,
		ActiveScore.CleanGatesCleared,
		ActiveScore.ExternalStrikesLanded,
		ActiveScore.GrazeBoundaryPenalties,
		false
	);

	bool bIsNewPB = false;
	if (ActiveSaveProfile)
	{
		ActiveSaveProfile->RecordRunScore(ActiveScore, bIsNewPB);
	}

	UE_LOG(LogTemp, Warning, TEXT("[SpaceSectorProgressionSubsystem] Sector %d Terminated (Reason: %s). Score: %d"),
		ActiveScore.SectorIndex, bLostToVoid ? TEXT("Lost to Void") : TEXT("Kinetic Impact"), ActiveScore.CompositeScore);

	if (OnSectorRunCompleted.IsBound())
	{
		OnSectorRunCompleted.Broadcast(ActiveScore, bIsNewPB);
	}

	return ActiveScore;
}
