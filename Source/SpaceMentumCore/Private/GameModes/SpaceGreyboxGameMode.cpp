/**
 * @file SpaceGreyboxGameMode.cpp
 * @brief Implementation of the greybox testbed game mode.
 */

#include "GameModes/SpaceGreyboxGameMode.h"
#include "Environment/SpaceGreyboxArenaActor.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Player/SpacePlayerController.h"
#include "GameModes/SpaceGameState.h"
#include "Components/MomentumTransferComponent.h"
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

	// 1. Search for an existing greybox arena actor in the map
	TArray<AActor*> FoundArenas;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceGreyboxArenaActor::StaticClass(), FoundArenas);

	if (FoundArenas.Num() > 0)
	{
		ActiveArena = Cast<ASpaceGreyboxArenaActor>(FoundArenas[0]);
	}
	else
	{
		// 2. Automatically spawn procedural greybox arena at origin if none was placed
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
}

void ASpaceGreyboxGameMode::RestartRun()
{
	if (ASpaceAstronautCharacter* Astronaut = Cast<ASpaceAstronautCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		// Reset astronaut to start origin
		Astronaut->SetActorLocation(FVector::ZeroVector);
		if (UMomentumTransferComponent* Transfer = Astronaut->GetMomentumTransfer())
		{
			Transfer->ResetLateralVelocity();
		}
	}

	if (ActiveArena)
	{
		ActiveArena->ResetSlalomEvaluations();
	}
}
