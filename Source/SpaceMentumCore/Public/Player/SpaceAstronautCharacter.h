/**
 * @file SpaceAstronautCharacter.h
 * @brief The space person: composes forward drift, mace orbit, momentum transfer, abilities, and the TPP chase camera.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction to move away from obsticles."
 *
 * @emotional_intent
 * The astronaut is small and vulnerable against vast space; the mace is heavy and powerful.
 * The chase camera sits behind and slightly above, keeping the full mace arc visible so the
 * player can READ the spin and time the release (GDD §2.1 F-5).
 *
 * Design: pure composition root.
 *  - Wires MaceOrbit release event to MomentumTransfer component.
 *  - Pushes equipped mace data asset parameters into physics components.
 *  - Hosts real-time telemetry rendering and debug trajectory gizmos.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Types/SpaceMentumPhysicsTypes.h"
#include "SpaceAstronautCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UForwardDriftComponent;
class UMaceOrbitComponent;
class UMomentumTransferComponent;
class UAbilitySystemBridgeComponent;
class UMaceDefinitionDataAsset;

UCLASS()
class SPACEMENTUMCORE_API ASpaceAstronautCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceAstronautCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Pushes a new mace definition data asset into the runtime physics components. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Mace")
	void EquipMaceDefinition(UMaceDefinitionDataAsset* NewMaceDefinition);

	// --- Component Getters ---

	USpringArmComponent* GetChaseSpringArm() const { return ChaseSpringArm; }
	UCameraComponent* GetChaseCamera() const { return ChaseCamera; }
	UMaceOrbitComponent* GetMaceOrbit() const { return MaceOrbit; }
	UMomentumTransferComponent* GetMomentumTransfer() const { return MomentumTransfer; }
	UForwardDriftComponent* GetForwardDrift() const { return ForwardDrift; }
	UAbilitySystemBridgeComponent* GetAbilityBridge() const { return AbilityBridge; }

	// --- Debug & Telemetry ---

	/** When true, renders 3D orbit gizmos, tangent arrows, and predicted corridor flight path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Debug")
	bool bEnableDebugVisualization = true;

	/** When true, renders on-screen physics telemetry measurements (RPM, speed, kick model). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Debug")
	bool bRenderOnScreenTelemetry = true;

protected:
	// --- Camera rig (GDD §3.1.1) ------------------------------------------------

	/** Elastic boom behind the astronaut: lag lets the player feel the kinetic "yank" without nausea. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Camera")
	TObjectPtr<USpringArmComponent> ChaseSpringArm;

	/** Cinematic TPP viewport. FOV shifts dynamically with speed and angular velocity. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Camera")
	TObjectPtr<UCameraComponent> ChaseCamera;

	// --- Physics feel (SpaceMentumPhysics) ---------------------------------------

	/** Relentless uniform forward speed. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Physics")
	TObjectPtr<UForwardDriftComponent> ForwardDrift;

	/** The heavy spinning mace. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Physics")
	TObjectPtr<UMaceOrbitComponent> MaceOrbit;

	/** Release -> whiplash kick. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Physics")
	TObjectPtr<UMomentumTransferComponent> MomentumTransfer;

	// --- Flavour (SpaceMentumAbilities) ------------------------------------------

	/** Socket for future abilities. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Abilities")
	TObjectPtr<UAbilitySystemBridgeComponent> AbilityBridge;

	/** Mace equipped at spawn (DA_Mace_Default). Its tuning is pushed into the physics components. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	TObjectPtr<UMaceDefinitionDataAsset> EquippedMace;

	// --- Delegate Callbacks -----------------------------------------------------

	/** Handles spin release from MaceOrbit and commands MomentumTransfer to calculate kick. */
	UFUNCTION()
	void HandleMaceOrbitReleased(
		float OrbitAngleRad,
		float AngularVelocityRadPerSec,
		float TetherLengthCm,
		float MaceMassKg);

	/** Handles kick execution to trigger camera FOV pulses and visceral response. */
	UFUNCTION()
	void HandleMomentumKickFired(
		FVector ImpartedImpulse,
		FVector ResultingLateralVelocity,
		EMomentumKickModel KickModel);

private:
	/** Tracks base field of view for dynamic kick pulse smoothing. */
	float CurrentFieldOfView = 80.0f;
	float TargetFieldOfView = 80.0f;
};
