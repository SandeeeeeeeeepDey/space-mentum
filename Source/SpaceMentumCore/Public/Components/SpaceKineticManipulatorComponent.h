/**
 * @file SpaceKineticManipulatorComponent.h
 * @brief Astronaut player component driving kinetic aiming, locking, and push/pull physics.
 *
 * @creator_feel
 * "we aim an asteroid and then lock it and then we decide how hard we will push it unless
 *  it is too close, that we need to decide if we still want to push or it will push it.
 *  lets implement the push first, then we can work on the pull"
 *
 * @emotional_intent
 * Pure agency, visceral reaction forces, and adrenaline-pumping proximity decisions.
 * The player aims at oncoming colossal asteroids, locks on with cybernetic targeting telemetry,
 * and charges the kinetic capacitor to blast rocks out of their path—all while managing the
 * terrifying danger of hesitation, where an incoming behemoth will overpower their push
 * and shove them violently into the void.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumKineticManipulationTypes.h"
#include "Solvers/KineticPushSolver.h"
#include "SpaceKineticManipulatorComponent.generated.h"

class ASpaceColossalAsteroidActor;
class ASpaceAstronautCharacter;

/** Delegate fired when the target lock status or locked entity changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnKineticTargetLockChanged,
	ETargetLockStatus, NewStatus,
	const FKineticTargetLockInfo&, LockInfo
);

/** Delegate fired when the push power charge level updates. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnKineticPushChargeUpdated,
	float, ChargeRatio,
	float, PotentialImpulseNs
);

/** Delegate fired when a kinetic push impulse is deployed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnKineticPushFired,
	const FKineticPushExecutionResult&, ExecutionResult
);

/** Delegate fired when an incoming locked asteroid enters the critical danger zone (< 18m). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCriticalProximityEntered,
	const FKineticTargetLockInfo&, CriticalTarget
);

/**
 * Delegate fired when an oncoming asteroid overpowers the push and shunts the player!
 * "unless it is too close, that we need to decide if we still want to push or it will push it."
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAsteroidOverpoweredPlayer,
	class ASpaceColossalAsteroidActor*, OverpoweringAsteroid,
	float, ShuntImpulseNs
);

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMCORE_API USpaceKineticManipulatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceKineticManipulatorComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Aiming & Locking Interface ---

	/** Updates the active aiming direction vector (from camera or input heading). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Kinetic")
	void SetAimDirection(const FVector& NewAimDirection);

	/** Locks onto the best candidate asteroid currently within the aiming cone. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Kinetic")
	bool LockBestCandidateInAim();

	/** Releases the active target lock. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Kinetic")
	void UnlockCurrentTarget();

	// --- Push Manipulation Interface ---

	/** Begins charging the kinetic repulsion capacitor. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Kinetic")
	void StartChargingPush();

	/** Releases the stored kinetic charge, delivering impulse to target and reaction to player. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Kinetic")
	FKineticPushExecutionResult ReleaseAndDeployPush();

	// --- Mode Configuration (Push vs future Pull) ---

	/** Sets the kinetic manipulation mode (Push active; Pull architected for next step). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Kinetic")
	void SetManipulationMode(EKineticManipulationMode NewMode) { ActiveMode = NewMode; }

	// --- Getters & Status ---

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Kinetic")
	EKineticManipulationMode GetActiveMode() const { return ActiveMode; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Kinetic")
	ETargetLockStatus GetLockStatus() const { return LockStatus; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Kinetic")
	const FKineticTargetLockInfo& GetLockInfo() const { return CurrentLockInfo; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Kinetic")
	float GetPushChargeRatio() const { return CurrentPushChargeRatio; }

	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Kinetic")
	bool IsChargingPush() const { return bIsChargingPush; }

	// --- Public Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Kinetic")
	FOnKineticTargetLockChanged OnTargetLockChanged;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Kinetic")
	FOnKineticPushChargeUpdated OnPushChargeUpdated;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Kinetic")
	FOnKineticPushFired OnKineticPushFired;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Kinetic")
	FOnCriticalProximityEntered OnCriticalProximityEntered;

	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Kinetic")
	FOnAsteroidOverpoweredPlayer OnAsteroidOverpoweredPlayer;

protected:
	/** Active manipulation mode (Push default). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic")
	EKineticManipulationMode ActiveMode = EKineticManipulationMode::Push;

	/** Designer feel knobs for kinetic pushing and locking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Kinetic")
	FKineticPushTuning PushTuning;

	/** Target lock telemetry snapshot. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FKineticTargetLockInfo CurrentLockInfo;

	/** Current lock status. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	ETargetLockStatus LockStatus = ETargetLockStatus::NoTarget;

	/** Current push capacitor charge [0.0, 1.0]. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	float CurrentPushChargeRatio = 0.0f;

	/** Whether the push capacitor is actively accumulating charge. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	bool bIsChargingPush = false;

	/** Aim direction vector in world space. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	FVector AimDirectionWorld = FVector::ForwardVector;

private:
	/** Scans active asteroids in world to find best candidate in aim cone. */
	ASpaceColossalAsteroidActor* FindBestCandidateInAimCone() const;

	/** Updates active target lock tracking metrics on tick. */
	void UpdateTargetLockTracking(float DeltaTime);
};
