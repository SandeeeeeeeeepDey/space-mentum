/**
 * @file ForwardDriftComponent.cpp
 * @brief Forward drift component implementation — uniform forward propulsion and velocity composition.
 *
 * @emotional_intent Relentless, uniform, sacred forward velocity.
 */

#include "Components/ForwardDriftComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"

UForwardDriftComponent::UForwardDriftComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UForwardDriftComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// Cache sibling momentum transfer component for lateral velocity composition
	CachedTransferComponent = OwnerActor->FindComponentByClass<UMomentumTransferComponent>();

	// Cache and configure character movement component for pure zero-g flying
	CachedMovementComponent = OwnerActor->FindComponentByClass<UCharacterMovementComponent>();
	if (CachedMovementComponent)
	{
		CachedMovementComponent->SetMovementMode(MOVE_Flying);
		CachedMovementComponent->bCheatFlying = true;
		CachedMovementComponent->BrakingDecelerationFlying = 0.0f;
		CachedMovementComponent->AirControl = 0.0f;
		CachedMovementComponent->GravityScale = 0.0f;
		CachedMovementComponent->MaxFlySpeed = 50000.0f; // High cap so it never throttles drift + lateral kick
	}
}

void UForwardDriftComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// Re-cache transfer component lazily if not found during BeginPlay
	if (!CachedTransferComponent)
	{
		CachedTransferComponent = OwnerActor->FindComponentByClass<UMomentumTransferComponent>();
	}

	// Compose local velocity: +X forward drift, YZ lateral dodging
	const FVector ComposedLocal = GetComposedLocalVelocity();

	// Transform local velocity vector to world space
	const FVector ComposedWorld = OwnerActor->GetActorTransform().TransformVector(ComposedLocal);

	// Apply composed velocity to movement component or physics root
	if (CachedMovementComponent)
	{
		CachedMovementComponent->Velocity = ComposedWorld;
		CachedMovementComponent->UpdateComponentVelocity();
	}
	else if (UPrimitiveComponent* PrimitiveRoot = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent()))
	{
		if (PrimitiveRoot->IsSimulatingPhysics())
		{
			PrimitiveRoot->SetPhysicsLinearVelocity(ComposedWorld);
		}
	}
}

void UForwardDriftComponent::SetDriftTuning(const FForwardDriftTuning& InTuning)
{
	DriftTuning = InTuning;
}

FVector UForwardDriftComponent::GetComposedLocalVelocity() const
{
	const FVector Lateral = CachedTransferComponent ? CachedTransferComponent->GetLateralVelocity() : FVector::ZeroVector;
	return FVector(DriftTuning.ForwardSpeedCmPerSec, Lateral.Y, Lateral.Z);
}

FVector UForwardDriftComponent::GetComposedWorldVelocity() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return GetComposedLocalVelocity();
	}

	return OwnerActor->GetActorTransform().TransformVector(GetComposedLocalVelocity());
}
