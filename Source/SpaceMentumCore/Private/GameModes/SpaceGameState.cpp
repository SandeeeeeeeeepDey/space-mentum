/**
 * @file SpaceGameState.cpp
 * @brief Implementation of run state, phase management, and live progression coordination.
 */

#include "GameModes/SpaceGameState.h"
#include "Subsystems/SpaceSectorProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

ASpaceGameState::ASpaceGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	RunPhase = ESpaceRunPhase::LaunchPushOff;
	ElapsedRunTimeSeconds = 0.0f;
}

void ASpaceGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (RunPhase == ESpaceRunPhase::Running)
	{
		ElapsedRunTimeSeconds += DeltaSeconds;

		if (UGameInstance* GI = GetGameInstance())
		{
			if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
			{
				APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
				const float CurrentX = PlayerPawn ? PlayerPawn->GetActorLocation().X : 0.0f;
				const float Speed = PlayerPawn ? PlayerPawn->GetVelocity().Size() : 3000.0f;

				Progression->UpdateFlightProgress(DeltaSeconds, CurrentX);
				Progression->SampleFlightSpeed(Speed);
			}
		}
	}
}

void ASpaceGameState::SetRunPhase(ESpaceRunPhase NewPhase)
{
	if (RunPhase == NewPhase)
	{
		return;
	}

	const ESpaceRunPhase OldPhase = RunPhase;
	RunPhase = NewPhase;

	UE_LOG(LogTemp, Log, TEXT("[SpaceGameState] Run Phase Transition: %s -> %s"),
		*UEnum::GetValueAsString(OldPhase),
		*UEnum::GetValueAsString(NewPhase));

	if (OnRunPhaseChanged.IsBound())
	{
		OnRunPhaseChanged.Broadcast(NewPhase, OldPhase);
	}
}

void ASpaceGameState::CompleteRunSuccess()
{
	if (RunPhase == ESpaceRunPhase::Completed)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpaceGameState] Run SUCCESS! Completed in %.2f seconds!"), ElapsedRunTimeSeconds);
	SetRunPhase(ESpaceRunPhase::Completed);
}

void ASpaceGameState::ResetRun()
{
	ElapsedRunTimeSeconds = 0.0f;
	SetRunPhase(ESpaceRunPhase::LaunchPushOff);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
		{
			Progression->StartNewSectorRun(Progression->GetActiveSectorIndex());
		}
	}
}

FSectorFlightScore ASpaceGameState::GetLiveFlightScore() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
		{
			return Progression->GetCurrentFlightScore();
		}
	}

	FSectorFlightScore DefaultScore;
	DefaultScore.ElapsedTimeSeconds = ElapsedRunTimeSeconds;
	return DefaultScore;
}
