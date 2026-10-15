/**
 * @file SpaceGreyboxGameMode.cpp
 * @brief Implementation of the greybox testbed game mode with Sector Exit Portal and Milestone Gates.
 */

#include "GameModes/SpaceGreyboxGameMode.h"
#include "Environment/SpaceGreyboxArenaActor.h"
#include "Environment/SpaceObstacleSpawner.h"
#include "Environment/SpaceStartingAsteroidActor.h"
#include "Environment/SpaceSectorExitPortalActor.h"
#include "Environment/SpaceMilestoneGateActor.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Player/SpacePlayerController.h"
#include "GameModes/SpaceGameState.h"
#include "Components/MomentumTransferComponent.h"
#include "Subsystems/SpaceSectorProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"

ASpaceGreyboxGameMode::ASpaceGreyboxGameMode()
{
	DefaultPawnClass = ASpaceAstronautCharacter::StaticClass();
	PlayerControllerClass = ASpacePlayerController::StaticClass();
	GameStateClass = ASpaceGameState::StaticClass();
}

void ASpaceGreyboxGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1. Search for or spawn greybox arena actor
	TArray<AActor*> FoundArenas;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceGreyboxArenaActor::StaticClass(), FoundArenas);

	if (FoundArenas.Num() > 0)
	{
		ActiveArena = Cast<ASpaceGreyboxArenaActor>(FoundArenas[0]);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ActiveArena = World->SpawnActor<ASpaceGreyboxArenaActor>(
			ASpaceGreyboxArenaActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
	}

	if (ActiveArena)
	{
		ActiveArena->OnSlalomGateEvaluated.AddDynamic(this, &ASpaceGreyboxGameMode::HandleSlalomGateEvaluated);
	}

	// 2. Search for or spawn celestial starting asteroid anchor at X = 0
	TArray<AActor*> FoundStartingAsteroids;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceStartingAsteroidActor::StaticClass(), FoundStartingAsteroids);

	if (FoundStartingAsteroids.Num() > 0)
	{
		ActiveStartingAsteroid = Cast<ASpaceStartingAsteroidActor>(FoundStartingAsteroids[0]);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ActiveStartingAsteroid = World->SpawnActor<ASpaceStartingAsteroidActor>(
			ASpaceStartingAsteroidActor::StaticClass(),
			FVector(-200.0f, 0.0f, 0.0f),
			FRotator::ZeroRotator,
			SpawnParams
		);
	}

	// 3. Search for or spawn procedural hazard spawner
	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceObstacleSpawner::StaticClass(), FoundSpawners);

	if (FoundSpawners.Num() > 0)
	{
		ActiveObstacleSpawner = Cast<ASpaceObstacleSpawner>(FoundSpawners[0]);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ActiveObstacleSpawner = World->SpawnActor<ASpaceObstacleSpawner>(
			ASpaceObstacleSpawner::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
	}

	// 4. Spawn or find Sector Exit Portal Actor
	SpawnOrFindExitPortal();

	// 5. Spawn or find Milestone Gates
	SpawnOrFindMilestones();
}

void ASpaceGreyboxGameMode::SpawnOrFindExitPortal()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> FoundPortals;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceSectorExitPortalActor::StaticClass(), FoundPortals);

	if (FoundPortals.Num() > 0)
	{
		ActiveExitPortal = Cast<ASpaceSectorExitPortalActor>(FoundPortals[0]);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Default position at 120,000 cm = 1.2 km terminus
		ActiveExitPortal = World->SpawnActor<ASpaceSectorExitPortalActor>(
			ASpaceSectorExitPortalActor::StaticClass(),
			FVector(120000.0f, 0.0f, 0.0f),
			FRotator::ZeroRotator,
			SpawnParams
		);
	}

	// Synchronize with active sector definition
	if (ActiveExitPortal)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
			{
				const FSectorDefinition Def = Progression->GetSectorDefinition(Progression->GetActiveSectorIndex());
				ActiveExitPortal->SetupPortal(Def);
			}
		}
	}
}

void ASpaceGreyboxGameMode::SpawnOrFindMilestones()
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> FoundMilestones;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceMilestoneGateActor::StaticClass(), FoundMilestones);

	if (FoundMilestones.Num() > 0)
	{
		ActiveMilestones.Empty();
		for (AActor* Actor : FoundMilestones)
		{
			if (ASpaceMilestoneGateActor* Gate = Cast<ASpaceMilestoneGateActor>(Actor))
			{
				ActiveMilestones.Add(Gate);
			}
		}
	}
	else
	{
		// Spawn 3 milestone arches: 300m (25%), 600m (50%), 900m (75%)
		ActiveMilestones.Empty();
		const float MilestoneDistances[] = { 30000.0f, 60000.0f, 90000.0f };
		const FString MilestoneTitles[] = { TEXT("Alpha Horizon (300m)"), TEXT("Debris Midpoint (600m)"), TEXT("Portal Approach (900m)") };
		const FLinearColor Colors[] = {
			FLinearColor(0.0f, 0.94f, 1.0f),
			FLinearColor(0.95f, 0.85f, 0.0f),
			FLinearColor(1.0f, 0.0f, 0.85f)
		};

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		for (int32 i = 0; i < 3; i++)
		{
			ASpaceMilestoneGateActor* Gate = World->SpawnActor<ASpaceMilestoneGateActor>(
				ASpaceMilestoneGateActor::StaticClass(),
				FVector(MilestoneDistances[i], 0.0f, 0.0f),
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (Gate)
			{
				FSectorMilestoneDefinition Def;
				Def.MilestoneIndex = i + 1;
				Def.DistanceCm = MilestoneDistances[i];
				Def.DistanceRatio = (i + 1) * 0.25f;
				Def.MilestoneTitle = MilestoneTitles[i];
				Def.GateColor = Colors[i];
				Def.ScoreBonus = 500 * (i + 1);

				Gate->SetupMilestone(Def);
				ActiveMilestones.Add(Gate);
			}
		}
	}
}

void ASpaceGreyboxGameMode::HandleSlalomGateEvaluated(const FString& GateName, float ClearanceDistanceCm, bool bWasSuccessfulEvasion, FVector Velocity)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
		{
			Progression->RecordSlalomGateResult(GateName, bWasSuccessfulEvasion, ClearanceDistanceCm);
		}
	}
}

void ASpaceGreyboxGameMode::RestartRun()
{
	if (ASpaceAstronautCharacter* Astronaut = Cast<ASpaceAstronautCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		// Reset astronaut to start origin anchor
		Astronaut->SetActorLocation(FVector::ZeroVector);
		Astronaut->SetActorRotation(FRotator::ZeroRotator);
		if (UMomentumTransferComponent* Transfer = Astronaut->GetMomentumTransfer())
		{
			Transfer->ResetLateralVelocity();
			Transfer->SetSystemCoMVelocity(FVector::ZeroVector);
		}
	}

	if (ActiveArena)
	{
		ActiveArena->ResetSlalomEvaluations();
	}

	if (ActiveObstacleSpawner)
	{
		ActiveObstacleSpawner->GenerateHazardCorridor();
	}

	if (ActiveExitPortal)
	{
		ActiveExitPortal->ResetPortal();
	}

	for (ASpaceMilestoneGateActor* Gate : ActiveMilestones)
	{
		if (Gate)
		{
			Gate->ResetMilestone();
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (ASpaceGameState* GameState = World->GetGameState<ASpaceGameState>())
		{
			GameState->ResetRun();
			GameState->SetRunPhase(ESpaceRunPhase::LaunchPushOff);
		}
	}
}

void ASpaceGreyboxGameMode::AdvanceToNextSector()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
		{
			const int32 NextSectorIndex = Progression->GetActiveSectorIndex() + 1;
			Progression->SetActiveSectorIndex(NextSectorIndex);

			const FSectorDefinition Def = Progression->GetSectorDefinition(NextSectorIndex);
			if (ActiveExitPortal)
			{
				ActiveExitPortal->SetupPortal(Def);
			}

			RestartRun();
		}
	}
}
