/**
 * @file ForwardDriftComponent.h
 * @brief Maintains the astronaut's uniform, never-ending forward velocity along +X and composes total velocity.
 *
 * @creator_feel
 * "a space person zooming in the forward direction in a uniform velocity"
 *
 * @emotional_intent
 * Relentless. The player can never slow down, never stop, never reverse. That
 * inevitability is what makes every mace release matter — "thrilling vulnerability
 * mixed with heavy kinetic power." Speed boosts may come later as abilities, but
 * the base drift is sacred and uniform.
 *
 * Responsibilities:
 *  - Enforce forward speed = FForwardDriftTuning::ForwardSpeedCmPerSec every physics tick.
 *  - Compose final velocity = forward drift (+X) + lateral velocity from UMomentumTransferComponent (YZ).
 *  - Apply composed velocity to UCharacterMovementComponent / RootPrimitive.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "ForwardDriftComponent.generated.h"

class UCharacterMovementComponent;
class UMomentumTransferComponent;

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMPHYSICS_API UForwardDriftComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UForwardDriftComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Sets new drift tuning parameters. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Drift")
	void SetDriftTuning(const FForwardDriftTuning& InTuning);

	/** Read-only access to drift tuning. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Drift")
	FForwardDriftTuning GetDriftTuning() const { return DriftTuning; }

	/** Current composed velocity in local space (cm/s). X = forward speed, YZ = lateral dodging. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Drift")
	FVector GetComposedLocalVelocity() const;

	/** Current composed velocity in world space (cm/s). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Drift")
	FVector GetComposedWorldVelocity() const;

	/** Unyielding forward speed value (cm/s). */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Drift")
	float GetForwardSpeedCmPerSec() const { return DriftTuning.ForwardSpeedCmPerSec; }

	/** Sets the forward drift speed (cm/s). */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Drift")
	void SetForwardSpeed(float InForwardSpeedCmPerSec) { DriftTuning.ForwardSpeedCmPerSec = InForwardSpeedCmPerSec; }

protected:
	/** Feel knob for the relentless forward speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Drift")
	FForwardDriftTuning DriftTuning;

	/** Cached pointer to sibling momentum transfer component for lateral velocity queries. */
	UPROPERTY(Transient)
	TObjectPtr<UMomentumTransferComponent> CachedTransferComponent;

	/** Cached character movement component if owner is an ACharacter. */
	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CachedMovementComponent;
};
