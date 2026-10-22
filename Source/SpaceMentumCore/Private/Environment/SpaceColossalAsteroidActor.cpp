/**
 * @file SpaceColossalAsteroidActor.cpp
 * @brief Implementation of colossal celestial asteroid actor.
 *
 * @creator_feel
 * "the game needs a screen where the asteroids are much bigger too.
 *  the belt of the asteroid needs to be much bigger and outside the view.
 *  the asteroids should be comming from any angle which has atleast one component from the forward."
 *
 * @emotional_intent
 * Monumental celestial hazards with authentic zero-g inertia and tumbling dynamics.
 * When struck with a kinetic push, the actor absorbs impulse and deflects realistically
 * while broadcasting state changes to dynamic visual and audio systems.
 */

#include "Environment/SpaceColossalAsteroidActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

ASpaceColossalAsteroidActor::ASpaceColossalAsteroidActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(SceneRoot);
	CollisionSphere->InitSphereRadius(AsteroidRadiusCm);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->bUseCCD = true;

	AsteroidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AsteroidMesh"));
	AsteroidMesh->SetupAttachment(CollisionSphere);
	AsteroidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsteroidMesh->bUseCCD = true;

	// Default fallback mesh if available
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (MeshFinder.Succeeded())
	{
		AsteroidMesh->SetStaticMesh(MeshFinder.Object);
		const float Scale = (AsteroidRadiusCm / 50.0f);
		AsteroidMesh->SetWorldScale3D(FVector(Scale));
	}
}

void ASpaceColossalAsteroidActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASpaceColossalAsteroidActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// 1. Linear Translation Integration
	if (!LinearVelocityWorld.IsNearlyZero())
	{
		const FVector DeltaLocation = LinearVelocityWorld * DeltaTime;
		AddActorWorldOffset(DeltaLocation, true);
	}

	// 2. Rotational Tumbling Integration
	if (!TumbleRateDegPerSec.IsNearlyZero())
	{
		const FRotator DeltaRotation = TumbleRateDegPerSec * DeltaTime;
		AddActorWorldRotation(DeltaRotation);
	}
}

void ASpaceColossalAsteroidActor::InitializeColossalAsteroid(
	float InRadiusCm,
	const FVector& InVelocityWorld,
	const FRotator& InTumbleRateRot)
{
	AsteroidRadiusCm = FMath::Max(300.0f, InRadiusCm);
	LinearVelocityWorld = InVelocityWorld;
	TumbleRateDegPerSec = InTumbleRateRot;

	// Scale mass proportionally to volume: M proportional to R^3
	// Reference: 650 cm radius = 45,000 kg
	const float VolumeRatio = FMath::Pow(AsteroidRadiusCm / 650.0f, 3.0f);
	AsteroidMassKg = FMath::Clamp(45000.0f * VolumeRatio, 5000.0f, 500000.0f);

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(AsteroidRadiusCm);
	}

	if (AsteroidMesh)
	{
		const float Scale = (AsteroidRadiusCm / 50.0f);
		AsteroidMesh->SetWorldScale3D(FVector(Scale));
	}
}

bool ASpaceColossalAsteroidActor::ReceiveKineticPushImpulse(const FVector& ImpulseVectorNs)
{
	if (AsteroidMassKg <= 1.0f)
	{
		return false;
	}

	// Delta velocity in m/s = Impulse (N*s) / Mass (kg)
	// Convert m/s to cm/s (* 100)
	const FVector DeltaVelocityCms = (ImpulseVectorNs / AsteroidMassKg) * 100.0f;
	LinearVelocityWorld += DeltaVelocityCms;

	OnColossalAsteroidPushed.Broadcast(this, ImpulseVectorNs, DeltaVelocityCms);
	return true;
}

void ASpaceColossalAsteroidActor::SetTargetLockHighlight(bool bLocked, bool bCriticalProximity)
{
	bIsCurrentlyLocked = bLocked;
	// Dynamic material parameter adjustment if dynamic material instances are present
}
