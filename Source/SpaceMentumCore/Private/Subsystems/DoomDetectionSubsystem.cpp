/**
 * @file DoomDetectionSubsystem.cpp
 * @brief Implementation of continuous background void doom trajectory lookahead.
 *
 * @creator_feel
 * "If the engine calculates the player's trajectory will not intersect... trigger a
 *  dramatic failure state... in the next 13 to 40 seconds."
 */

#include "Subsystems/DoomDetectionSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Components/ForwardDriftComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "Environment/SpaceObstacleSpawner.h"
#include "GameModes/SpaceGameState.h"
#include "Subsystems/SpaceSectorProgressionSubsystem.h"

UDoomDetectionSubsystem::UDoomDetectionSubsystem()
{
}

void UDoomDetectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TimeSinceLastEvaluation = 0.0f;
	bHasFiredDoomForRun = false;
	LatestSnapshot = FDoomPredictionSnapshot();
}

void UDoomDetectionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UDoomDetectionSubsystem::Tick(float DeltaTime)
{
	if (!bEnableDoomDetection || bHasFiredDoomForRun)
	{
		return;
	}

	TimeSinceLastEvaluation += DeltaTime;
	if (TimeSinceLastEvaluation >= EvaluationIntervalSeconds)
	{
		TimeSinceLastEvaluation = 0.0f;
		EvaluateCurrentTrajectory(LatestSnapshot);
	}
}

TStatId UDoomDetectionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDoomDetectionSubsystem, STATGROUP_Tickables);
}

bool UDoomDetectionSubsystem::EvaluateCurrentTrajectory(FDoomPredictionSnapshot& OutSnapshot)
{
	ASpaceAstronautCharacter* Astronaut = FindAstronautPawn();
	if (!IsValid(Astronaut))
	{
		return false;
	}

	// Verify the run is currently in the active Running phase
	UWorld* World = GetWorld();
	if (World)
	{
		ASpaceGameState* GameState = World->GetGameState<ASpaceGameState>();
		if (GameState && GameState->GetCurrentRunPhase() != ESpaceRunPhase::Running)
		{
			return false;
		}
	}

	const FVector CoMPosition = Astronaut->GetActorLocation();

	// Total system Center of Mass velocity = ForwardDrift + LateralKick + ExternalStrikeDeflection
	FVector CoMVelocity = FVector::ZeroVector;
	if (Astronaut->GetForwardDrift())
	{
		CoMVelocity += Astronaut->GetForwardDrift()->GetForwardVelocityWorld();
	}
	if (Astronaut->GetMomentumTransfer())
	{
		CoMVelocity += Astronaut->GetMomentumTransfer()->GetLateralVelocityWorld();
		CoMVelocity += Astronaut->GetMomentumTransfer()->GetSystemCoMVelocity();
	}

	// Calculate maximum reach envelope of mace
	float MaceReachRadius = 220.0f; // Default fallback
	if (Astronaut->GetMaceOrbit())
	{
		MaceReachRadius = Astronaut->GetMaceOrbit()->GetOrbitTuning().TetherLengthCm + ReachToleranceMarginCm;
	}

	// Query obstacle proxies from active spawner
	TArray<FObstacleSpatialProxy> CandidateProxies;
	ASpaceObstacleSpawner* Spawner = FindObstacleSpawner();
	if (IsValid(Spawner))
	{
		const float ForwardSpeed = FMath::Max(CoMVelocity.Size(), 1000.0f);
		const float MaxLookaheadDistance = ForwardSpeed * MaxLookaheadSeconds;

		CandidateProxies = Spawner->GetObstacleProxiesInLookahead(
			CoMPosition,
			0.0f,
			MaxLookaheadDistance
		);
	}

	// Execute pure mathematical lookahead solver
	const bool bDoomed = FDoomTrajectoryEvaluator::EvaluateDoomTrajectory(
		CoMPosition,
		CoMVelocity,
		MaceReachRadius,
		CandidateProxies,
		MinLookaheadSeconds,
		MaxLookaheadSeconds,
		StallSpeedThresholdCmPerSec,
		OutSnapshot
	);

	if (bDoomed && !bHasFiredDoomForRun)
	{
		bHasFiredDoomForRun = true;

		UE_LOG(LogTemp, Warning,
			TEXT("[DoomDetectionSubsystem] VOID DOOM TRIGGERED! Reason: %s (Horizon: %0.1fs, Reachable: %d)."),
			*UEnum::GetValueAsString(OutSnapshot.Reason),
			OutSnapshot.LookaheadHorizonSeconds,
			OutSnapshot.ReachableObstacleCount);

		// Broadcast dramatic failure state
		if (OnDoomDetected.IsBound())
		{
			OnDoomDetected.Broadcast(OutSnapshot);
		}

		// Transition game state to LostToTheVoid
		if (World)
		{
			ASpaceGameState* GameState = World->GetGameState<ASpaceGameState>();
			if (GameState)
			{
				GameState->SetRunPhase(ESpaceRunPhase::LostToTheVoid);
			}

			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
				{
					Progression->TerminateActiveSectorRunFailed(true);
				}
			}
		}
	}

	return bDoomed;
}

ASpaceAstronautCharacter* UDoomDetectionSubsystem::FindAstronautPawn() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		return Cast<ASpaceAstronautCharacter>(PC->GetPawn());
	}

	return nullptr;
}

ASpaceObstacleSpawner* UDoomDetectionSubsystem::FindObstacleSpawner() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ASpaceObstacleSpawner> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}
