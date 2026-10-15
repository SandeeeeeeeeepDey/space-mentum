/**
 * @file SpaceStartingAsteroidActor.cpp
 * @brief Implementation of the celestial starting asteroid anchor.
 */

#include "Environment/SpaceStartingAsteroidActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

ASpaceStartingAsteroidActor::ASpaceStartingAsteroidActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	AsteroidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
	AsteroidMesh->SetupAttachment(RootComponent);
	AsteroidMesh->SetCollisionProfileName(TEXT("BlockAll"));
	AsteroidMesh->SetGenerateOverlapEvents(true);

	AnchorSocket = CreateDefaultSubobject<USceneComponent>(TEXT("AnchorSocket"));
	AnchorSocket->SetupAttachment(RootComponent);
	// Default socket is on the forward (+X) apex of the asteroid
	AnchorSocket->SetRelativeLocation(FVector(180.0f, 0.0f, 0.0f));

	AsteroidRadius = 320.0f;
}

FTransform ASpaceStartingAsteroidActor::GetAstronautAnchorTransform() const
{
	if (AnchorSocket)
	{
		return AnchorSocket->GetComponentTransform();
	}
	return GetActorTransform();
}

FVector ASpaceStartingAsteroidActor::GetAstronautAnchorLocation() const
{
	if (AnchorSocket)
	{
		return AnchorSocket->GetComponentLocation();
	}
	return GetActorLocation();
}

FVector ASpaceStartingAsteroidActor::ComputeDepartureVelocity(
	float PushChargeRatio,
	const FVector& AimDirection,
	const FLaunchPushOffParameters& LaunchParams) const
{
	const float ClampedCharge = FMath::Clamp(PushChargeRatio, 0.05f, 1.0f);
	FVector SafeAimDir = AimDirection.GetSafeNormal();

	if (SafeAimDir.IsNearlyZero())
	{
		SafeAimDir = FVector::ForwardVector;
	}

	// Departure speed scales linearly with charge up to MaxLaunchSpeedCmPerSec
	const float DepartureSpeed = ClampedCharge * LaunchParams.MaxLaunchSpeedCmPerSec;
	return SafeAimDir * DepartureSpeed;
}
