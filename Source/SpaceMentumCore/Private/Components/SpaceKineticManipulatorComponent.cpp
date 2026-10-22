/**
 * @file SpaceKineticManipulatorComponent.cpp
 * @brief Implementation of astronaut kinetic aiming, locking, and push execution mechanics.
 *
 * @creator_feel
 * "we aim an asteroid and then lock it and then we decide how hard we will push it unless
 *  it is too close, that we need to decide if we still want to push or it will push it.
 *  lets implement the push first, then we can work on the pull"
 *
 * @emotional_intent
 * Delivers immediate tactile agency and thrilling tension. The player has full control
 * over aiming, capacitor charging, and deployment. Hesitation in critical proximity carries
 * real consequences: weak pushes are violently overpowered by oncoming celestial boulders.
 */

#include "Components/SpaceKineticManipulatorComponent.h"
#include "Environment/SpaceColossalAsteroidActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

USpaceKineticManipulatorComponent::USpaceKineticManipulatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USpaceKineticManipulatorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USpaceKineticManipulatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// 1. Accumulate Push Charge if actively holding charge
	if (bIsChargingPush)
	{
		CurrentPushChargeRatio = FMath::Min(1.0f, CurrentPushChargeRatio + DeltaTime * PushTuning.ChargeRatePerSec);
		const float PotentialImpulse = CurrentPushChargeRatio * PushTuning.MaxPushImpulseNs;
		OnPushChargeUpdated.Broadcast(CurrentPushChargeRatio, PotentialImpulse);
	}

	// 2. Update Target Lock Tracking
	UpdateTargetLockTracking(DeltaTime);
}

void USpaceKineticManipulatorComponent::SetAimDirection(const FVector& NewAimDirection)
{
	if (!NewAimDirection.IsNearlyZero())
	{
		AimDirectionWorld = NewAimDirection.GetSafeNormal();
	}
}

bool USpaceKineticManipulatorComponent::LockBestCandidateInAim()
{
	ASpaceColossalAsteroidActor* BestCandidate = FindBestCandidateInAimCone();
	if (!BestCandidate)
	{
		return false;
	}

	// Unlock previous target if different
	if (CurrentLockInfo.TargetActor.IsValid() && CurrentLockInfo.TargetActor.Get() != BestCandidate)
	{
		UnlockCurrentTarget();
	}

	CurrentLockInfo.TargetActor = BestCandidate;
	CurrentLockInfo.TargetWorldLocation = BestCandidate->GetActorLocation();
	CurrentLockInfo.TargetVelocityWorld = BestCandidate->GetAsteroidVelocity();
	CurrentLockInfo.TargetRadiusCm = BestCandidate->GetAsteroidRadiusCm();
	CurrentLockInfo.TargetMassKg = BestCandidate->GetAsteroidMassKg();

	LockStatus = ETargetLockStatus::TargetLocked;
	BestCandidate->SetTargetLockHighlight(true, false);

	OnTargetLockChanged.Broadcast(LockStatus, CurrentLockInfo);
	return true;
}

void USpaceKineticManipulatorComponent::UnlockCurrentTarget()
{
	if (CurrentLockInfo.TargetActor.IsValid())
	{
		ASpaceColossalAsteroidActor* Asteroid = Cast<ASpaceColossalAsteroidActor>(CurrentLockInfo.TargetActor.Get());
		if (Asteroid)
		{
			Asteroid->SetTargetLockHighlight(false, false);
		}
	}

	CurrentLockInfo = FKineticTargetLockInfo();
	LockStatus = ETargetLockStatus::NoTarget;

	OnTargetLockChanged.Broadcast(LockStatus, CurrentLockInfo);
}

void USpaceKineticManipulatorComponent::StartChargingPush()
{
	bIsChargingPush = true;
}

FKineticPushExecutionResult USpaceKineticManipulatorComponent::ReleaseAndDeployPush()
{
	bIsChargingPush = false;
	const float FinalCharge = CurrentPushChargeRatio;
	CurrentPushChargeRatio = 0.0f;
	OnPushChargeUpdated.Broadcast(0.0f, 0.0f);

	FKineticPushExecutionResult Result;

	if (!CurrentLockInfo.TargetActor.IsValid())
	{
		return Result;
	}

	ASpaceColossalAsteroidActor* LockedAsteroid = Cast<ASpaceColossalAsteroidActor>(CurrentLockInfo.TargetActor.Get());
	if (!LockedAsteroid)
	{
		return Result;
	}

	AActor* OwnerActor = GetOwner();
	const float AstronautMassKg = 200.0f; // Default astronaut + mace mass

	// Evaluate Newtonian solution via pure solver
	FKineticPushSolver::EvaluateKineticPushExecution(
		PushTuning,
		FinalCharge,
		CurrentLockInfo,
		AstronautMassKg,
		Result
	);

	if (Result.bImpulseDelivered)
	{
		if (Result.Outcome == EProximityDecisionOutcome::AsteroidOverpoweredPush)
		{
			// "IT WILL PUSH IT": The oncoming asteroid's inertia overpowers the push!
			OnAsteroidOverpoweredPlayer.Broadcast(LockedAsteroid, Result.ShuntImpulseMagnitude);

			// Apply shunt kick to astronaut
			ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor);
			if (OwnerChar && OwnerChar->GetCharacterMovement())
			{
				OwnerChar->GetCharacterMovement()->AddImpulse(Result.ReactionImpulseToAstronaut, false);
			}
		}
		else
		{
			// Successful Push Deflection (Safe range or heroic point-blank deflection)
			LockedAsteroid->ReceiveKineticPushImpulse(Result.TransferredImpulseToAsteroid);

			// Apply reaction recoil impulse to astronaut (Newton's 3rd Law)
			ACharacter* OwnerChar = Cast<ACharacter>(OwnerActor);
			if (OwnerChar && OwnerChar->GetCharacterMovement())
			{
				OwnerChar->GetCharacterMovement()->AddImpulse(Result.ReactionImpulseToAstronaut, false);
			}
		}

		OnKineticPushFired.Broadcast(Result);
	}

	return Result;
}

ASpaceColossalAsteroidActor* USpaceKineticManipulatorComponent::FindBestCandidateInAimCone() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AActor* OwnerActor = GetOwner();
	const FVector AimOrigin = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;

	TArray<AActor*> FoundAsteroids;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceColossalAsteroidActor::StaticClass(), FoundAsteroids);

	ASpaceColossalAsteroidActor* BestCandidate = nullptr;
	float SmallestAngleOffAim = 999.0f;

	for (AActor* Actor : FoundAsteroids)
	{
		ASpaceColossalAsteroidActor* Asteroid = Cast<ASpaceColossalAsteroidActor>(Actor);
		if (!Asteroid)
		{
			continue;
		}

		float AngleOffAimDeg = 0.0f;
		const bool bInCone = FKineticPushSolver::IsTargetWithinAimCone(
			AimOrigin,
			AimDirectionWorld,
			Asteroid->GetActorLocation(),
			PushTuning.MaxLockRangeCm,
			PushTuning.AimConeHalfAngleDeg,
			AngleOffAimDeg
		);

		if (bInCone && AngleOffAimDeg < SmallestAngleOffAim)
		{
			SmallestAngleOffAim = AngleOffAimDeg;
			BestCandidate = Asteroid;
		}
	}

	return BestCandidate;
}

void USpaceKineticManipulatorComponent::UpdateTargetLockTracking(float DeltaTime)
{
	AActor* OwnerActor = GetOwner();
	const FVector PlayerPos = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
	ACharacter* PlayerChar = Cast<ACharacter>(OwnerActor);
	const FVector PlayerVel = PlayerChar ? PlayerChar->GetVelocity() : FVector::ZeroVector;

	if (CurrentLockInfo.TargetActor.IsValid())
	{
		ASpaceColossalAsteroidActor* Asteroid = Cast<ASpaceColossalAsteroidActor>(CurrentLockInfo.TargetActor.Get());
		if (!Asteroid)
		{
			UnlockCurrentTarget();
			return;
		}

		CurrentLockInfo.TargetWorldLocation = Asteroid->GetActorLocation();
		CurrentLockInfo.TargetVelocityWorld = Asteroid->GetAsteroidVelocity();

		// Calculate relative kinematics
		FKineticPushSolver::CalculateTargetTrajectoryMetrics(
			CurrentLockInfo.TargetWorldLocation,
			CurrentLockInfo.TargetVelocityWorld,
			PlayerPos,
			PlayerVel,
			CurrentLockInfo.TargetRadiusCm,
			CurrentLockInfo.RelativeVelocity,
			CurrentLockInfo.SurfaceDistanceCm,
			CurrentLockInfo.TimeToImpactSeconds
		);

		CurrentLockInfo.CenterDistanceCm = FVector::Dist(CurrentLockInfo.TargetWorldLocation, PlayerPos);

		// Check critical proximity breach (< 18m)
		const bool bWasCritical = CurrentLockInfo.bInCriticalZone;
		const bool bNowCritical = CurrentLockInfo.SurfaceDistanceCm <= PushTuning.CriticalProximityThresholdCm;
		CurrentLockInfo.bInCriticalZone = bNowCritical;

		if (bNowCritical && !bWasCritical)
		{
			LockStatus = ETargetLockStatus::CriticalProximity;
			Asteroid->SetTargetLockHighlight(true, true);
			OnCriticalProximityEntered.Broadcast(CurrentLockInfo);
		}
		else if (!bNowCritical && bWasCritical)
		{
			LockStatus = ETargetLockStatus::TargetLocked;
			Asteroid->SetTargetLockHighlight(true, false);
		}

		OnTargetLockChanged.Broadcast(LockStatus, CurrentLockInfo);
	}
	else
	{
		// Passive aim sweep to update status
		ASpaceColossalAsteroidActor* Candidate = FindBestCandidateInAimCone();
		if (Candidate)
		{
			LockStatus = ETargetLockStatus::AcquiringTarget;
		}
		else
		{
			LockStatus = ETargetLockStatus::NoTarget;
		}
	}
}
