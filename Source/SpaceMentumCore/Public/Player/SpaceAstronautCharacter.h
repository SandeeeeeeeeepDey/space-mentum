/**
 * @file SpaceAstronautCharacter.h
 * @brief The space person: composes forward drift, mace orbit, momentum transfer, abilities, and the TPP chase camera.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand."
 *
 * @emotional_intent
 * The astronaut is small and vulnerable against vast space; the mace is heavy and powerful.
 * The chase camera sits behind and slightly above, keeping the full mace arc visible so the
 * player can READ the spin and time the release (GDD §2.1 F-5).
 *
 * Design: pure composition. This class owns components and wires them together; it holds
 * no physics math itself. All feel lives in SpaceMentumPhysics and the mace data asset.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	USpringArmComponent* GetChaseSpringArm() const { return ChaseSpringArm; }
	UCameraComponent* GetChaseCamera() const { return ChaseCamera; }
	UMaceOrbitComponent* GetMaceOrbit() const { return MaceOrbit; }
	UMomentumTransferComponent* GetMomentumTransfer() const { return MomentumTransfer; }
	UForwardDriftComponent* GetForwardDrift() const { return ForwardDrift; }

protected:
	// --- Camera rig (GDD §3.1.1) ------------------------------------------------

	/** Elastic boom behind the astronaut: lag lets the player feel the kinetic "yank" without nausea. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Camera")
	TObjectPtr<USpringArmComponent> ChaseSpringArm;

	/** Cinematic TPP viewport. FOV widens 80° -> 105° with speed + spin (Step 5). */
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

	/** Mace equipped at spawn (DA_Mace_Default). Its tuning is pushed into the physics components in Step 2. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	TObjectPtr<UMaceDefinitionDataAsset> EquippedMace;
};
