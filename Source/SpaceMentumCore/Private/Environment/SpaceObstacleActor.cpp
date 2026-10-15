/**
 * @file SpaceObstacleActor.cpp
 * @brief Implementation of continuous-collision space obstacles.
 *
 * @emotional_intent Heavy, brutal presence; satisfying kinetic strikes; lethal body crashes.
 */

#include "Environment/SpaceObstacleActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Data/ObstacleArchetypeDataAsset.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Components/MomentumTransferComponent.h"
#include "Solvers/CenterOfMassKinematicsSolver.h"

ASpaceObstacleActor::ASpaceObstacleActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// Collision sphere configured for continuous collision detection (CCD)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(SceneRoot);
	CollisionSphere->InitSphereRadius(CollisionRadiusCm);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetUseCCD(true); // Eliminate high-speed tunneling along +X corridor

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(CollisionSphere);
	ObstacleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Sphere owns collision
}

void ASpaceObstacleActor::BeginPlay()
{
	Super::BeginPlay();

	if (ArchetypeData)
	{
		ApplyArchetypeData(ArchetypeData);
	}
}

void ASpaceObstacleActor::ApplyArchetypeData(UObstacleArchetypeDataAsset* InArchetypeData)
{
	if (!InArchetypeData)
	{
		return;
	}

	ArchetypeData = InArchetypeData;
	bIsMaceDestructible = (InArchetypeData->Behaviour == EObstacleBehaviour::MaceDestructible);
	ShatterSpeedThresholdCmPerSec = InArchetypeData->BreakSpeedThresholdCmPerSec;

	if (InArchetypeData->Mesh.IsValid())
	{
		ObstacleMesh->SetStaticMesh(InArchetypeData->Mesh.Get());
	}
}

bool ASpaceObstacleActor::ProcessMaceStrike(
	const FVector& MaceVelocityWorld,
	const FVector& HitLocationWorld,
	const FVector& HitNormalWorld,
	float MaceMassKg,
	float TotalSystemMassKg,
	ASpaceAstronautCharacter* StrikingAstronaut)
{
	if (bIsDestroyed)
	{
		return false;
	}

	const FVector ObstacleVelocity = FVector::ZeroVector; // Fixed cosmic debris
	const float RelativeImpactSpeed = (MaceVelocityWorld - ObstacleVelocity).Size();

	// Check if this impact shatters a destructible barrier
	if (bIsMaceDestructible && RelativeImpactSpeed >= ShatterSpeedThresholdCmPerSec)
	{
		HandleObstacleShattered();
	}

	// Transfer external collision momentum to the astronaut's system Center of Mass
	if (StrikingAstronaut && StrikingAstronaut->GetMomentumTransfer())
	{
		StrikingAstronaut->GetMomentumTransfer()->ExecuteExternalStrike(
			MaceVelocityWorld,
			ObstacleVelocity,
			HitLocationWorld,
			HitNormalWorld,
			MaceMassKg,
			ObstacleMassKg,
			Restitution,
			bIsAnchored,
			this
		);
	}

	// Prepare result for local delegate listeners
	FExternalStrikeImpulse LocalImpulse;
	FCenterOfMassKinematicsSolver::SolveExternalStrikeCollision(
		MaceVelocityWorld,
		ObstacleVelocity,
		HitLocationWorld,
		HitNormalWorld,
		TotalSystemMassKg,
		ObstacleMassKg,
		Restitution,
		bIsAnchored,
		LocalImpulse
	);

	if (OnObstacleMaceStruck.IsBound())
	{
		OnObstacleMaceStruck.Broadcast(this, LocalImpulse);
	}

	return true;
}

bool ASpaceObstacleActor::ProcessAstronautBodyCollision(
	const FVector& RelativeVelocityWorld,
	ASpaceAstronautCharacter* ImpactedAstronaut)
{
	if (bIsDestroyed)
	{
		return false;
	}

	const bool bIsFatal = FCenterOfMassKinematicsSolver::IsKineticImpactLethal(RelativeVelocityWorld);

	if (OnObstacleAstronautImpact.IsBound())
	{
		OnObstacleAstronautImpact.Broadcast(this, bIsFatal);
	}

	if (bIsFatal && ImpactedAstronaut)
	{
		HandleFatalAstronautCrash(ImpactedAstronaut);
	}

	return bIsFatal;
}

FObstacleSpatialProxy ASpaceObstacleActor::GetSpatialProxy() const
{
	return FObstacleSpatialProxy(
		GetActorLocation(),
		CollisionRadiusCm,
		!bIsDestroyed
	);
}

void ASpaceObstacleActor::HandleFatalAstronautCrash(ASpaceAstronautCharacter* Astronaut)
{
	// Log fatal kinetic collision
	UE_LOG(LogTemp, Warning, TEXT("[SpaceObstacleActor] Fatal Kinetic Impact at %s with %s"),
		*GetActorLocation().ToString(), *GetName());
}

void ASpaceObstacleActor::HandleObstacleShattered()
{
	bIsDestroyed = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (OnObstacleDestroyed.IsBound())
	{
		OnObstacleDestroyed.Broadcast(this);
	}
}
