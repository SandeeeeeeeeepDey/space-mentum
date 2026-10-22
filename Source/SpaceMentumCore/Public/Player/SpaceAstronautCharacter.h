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
#include "Types/SpaceMentumCoreTypes.h"
#include "SpaceAstronautCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UForwardDriftComponent;
class UMaceOrbitComponent;
class UMomentumTransferComponent;
class UAbilitySystemBridgeComponent;
class UMaceDefinitionDataAsset;
class USpaceFlightInstrumentationComponent;
class USpaceFlightCameraComponent;
class USpaceFlightVFXComponent;
class USpaceProceduralFlightAudioComponent;
class UNiagaraSystem;
class UPhysicsConstraintComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class USpaceKineticManipulatorComponent;

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

	// --- Launch Push-Off Controls (§1.4 & Rule 1.1) ---

	/** Begins charging the mechanical appendage ram against the starting asteroid. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Launch")
	void StartChargingLaunchPush();

	/** Releases the stored kinetic charge, firing the ram and launching into the gauntlet. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Launch")
	void ReleaseLaunchPushAndFire();

	/** Steers the departure direction arrow around the starting asteroid tangent plane. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Launch")
	void AdjustLaunchAim(float DeltaDegrees);

	/** Current charge ratio (0.0 to 1.0) of the push accumulator. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Launch")
	float GetCurrentPushChargeRatio() const { return CurrentPushChargeRatio; }

	/** Current departure trajectory direction vector. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Launch")
	FVector GetLaunchAimDirection() const;

	// --- Component Getters ---

	USpringArmComponent* GetChaseSpringArm() const { return ChaseSpringArm; }
	UCameraComponent* GetChaseCamera() const { return ChaseCamera; }
	UMaceOrbitComponent* GetMaceOrbit() const { return MaceOrbit; }
	UMomentumTransferComponent* GetMomentumTransfer() const { return MomentumTransfer; }
	UForwardDriftComponent* GetForwardDrift() const { return ForwardDrift; }
	UAbilitySystemBridgeComponent* GetAbilityBridge() const { return AbilityBridge; }
	USpaceFlightInstrumentationComponent* GetFlightInstrumentation() const { return FlightInstrumentation; }
	USpaceFlightCameraComponent* GetFlightCamera() const { return FlightCamera; }
	USpaceFlightVFXComponent* GetFlightVFX() const { return FlightVFX; }
	USpaceProceduralFlightAudioComponent* GetFlightAudio() const { return FlightAudio; }
	UStaticMeshComponent* GetMechanicalArmMesh() const { return MechanicalArmMesh; }
	UStaticMeshComponent* GetMaceHeadMesh() const { return MaceHeadMesh; }
	UPhysicsConstraintComponent* GetArmPhysicsConstraint() const { return ArmPhysicsConstraint; }
	UMaterialInstanceDynamic* GetSuitDynamicMaterial() const { return SuitDynamicMaterial; }
	UMaterialInstanceDynamic* GetArmDynamicMaterial() const { return ArmDynamicMaterial; }
	UMaterialInstanceDynamic* GetMaceDynamicMaterial() const { return MaceDynamicMaterial; }
	USpaceKineticManipulatorComponent* GetKineticManipulator() const { return KineticManipulator; }

	// --- Debug & Telemetry (Enforcing Rule 4 Blind Math) ---

	/** When true, renders 3D debug gizmos. Defaults to FALSE per Rule 4 "Blind Math" policy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Debug")
	bool bEnableDebugVisualization = false;

	/** When true, renders on-screen physics telemetry measurements (RPM, speed, kick model). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Debug")
	bool bRenderOnScreenTelemetry = false;

	/** Strict Blind Math: Ensures zero predictive trajectory lines or aim reticles exist in viewport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Debug")
	bool bStrictBlindMathEnforced = true;

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

	/** Cockpit flight instrumentation delivering real-time telemetry (Radar, Speedometer, Direction Arrow, Push Power). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Instrumentation")
	TObjectPtr<USpaceFlightInstrumentationComponent> FlightInstrumentation;

	/** Cinematic flight camera rig managing dynamic FOV, Dutch roll, and trauma shake. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Camera")
	TObjectPtr<USpaceFlightCameraComponent> FlightCamera;

	/** Multi-system Niagara particle coordinator managing plasma ribbon trails and hyperspace dust. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|VFX")
	TObjectPtr<USpaceFlightVFXComponent> FlightVFX;

	/** Dynamic procedural audio & Metasound coordinator driving spin frequency and launch transients. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Audio")
	TObjectPtr<USpaceProceduralFlightAudioComponent> FlightAudio;

	/** Kinetic aiming, locking, and push manipulation component for colossal asteroids. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Kinetic")
	TObjectPtr<USpaceKineticManipulatorComponent> KineticManipulator;

	// --- Visual & Particle Pipeline (Step 5 Niagara) ----------------------------

	/** Orbital plasma ribbon trail (NS_MacePlasmaTrail): color shifts cyan -> ultraviolet with omega. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX")
	TSoftObjectPtr<UNiagaraSystem> MacePlasmaTrailSystem;

	/** Directional hyperspace velocity cues (NS_HyperspaceDust). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX")
	TSoftObjectPtr<UNiagaraSystem> HyperspaceDustSystem;

	/** High-energy kinetic spark burst on obstacle strikes (NS_KineticImpactBurst). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|VFX")
	TSoftObjectPtr<UNiagaraSystem> KineticImpactBurstSystem;

	// --- Step 7: High-Fidelity Asset Pipeline & Mechanical Rigging ---

	/** Articulated mechanical arm mesh attached to astronaut socket (Socket_AppendageMount). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Rigging")
	TObjectPtr<UStaticMeshComponent> MechanicalArmMesh;

	/** Heavy kinetic ram mace head mesh with faceted armor plates and glowing plasma core. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Rigging")
	TObjectPtr<UStaticMeshComponent> MaceHeadMesh;

	/** Physical constraint simulating joint compliance and hydraulic shock absorption. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Rigging")
	TObjectPtr<UPhysicsConstraintComponent> ArmPhysicsConstraint;

	/** Dynamic material instance for astronaut spacesuit emissive telemetry and thruster glow. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "SpaceMentum|Materials")
	TObjectPtr<UMaterialInstanceDynamic> SuitDynamicMaterial;

	/** Dynamic material instance for mechanical arm hydraulic heat glow. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "SpaceMentum|Materials")
	TObjectPtr<UMaterialInstanceDynamic> ArmDynamicMaterial;

	/** Dynamic material instance for mace head kinetic energy plasma core. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "SpaceMentum|Materials")
	TObjectPtr<UMaterialInstanceDynamic> MaceDynamicMaterial;

	/** Default spacesuit PBR material reference (M_Astronaut_Suit_PBR). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Materials")
	TSoftObjectPtr<UMaterialInterface> SuitBaseMaterial;

	/** Default mechanical arm hard-surface material reference (M_MechanicalArm_HardSurface). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Materials")
	TSoftObjectPtr<UMaterialInterface> ArmBaseMaterial;

	/** Default mace head heavy PBR material reference (M_MaceHead_Heavy). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Materials")
	TSoftObjectPtr<UMaterialInterface> MaceBaseMaterial;

	// --- Flavour (SpaceMentumAbilities) ------------------------------------------

	/** Socket for future abilities. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Abilities")
	TObjectPtr<UAbilitySystemBridgeComponent> AbilityBridge;

	/** Mace equipped at spawn (DA_Mace_Default). Its tuning is pushed into the physics components. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Mace")
	TObjectPtr<UMaceDefinitionDataAsset> EquippedMace;

	// --- Launch Push-Off Parameters ---------------------------------------------

	/** Tunings for starting asteroid mechanical ram push-off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Launch")
	FLaunchPushOffParameters LaunchParameters;

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

	/** Callback when astronaut body capsule collides with an obstacle. */
	UFUNCTION()
	void HandleCapsuleHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	/** Sweeps the entire mechanical appendage (telescoping arm + kinetic ram head) against scene obstacles each tick. */
	void PerformAppendageCollisionSweep(float DeltaSeconds);

	/** Previous frame's mace head location for continuous sweep interpolation. */
	FVector PreviousMaceHeadLocation = FVector::ZeroVector;

	/** Previous frame's astronaut body location for swept arm interpolation. */
	FVector PreviousAstronautLocation = FVector::ZeroVector;

	/** Timestamp of last appendage deflection to prevent immediate body collision on deflected obstacles. */
	float LastAppendageDeflectionTime = -1.0f;

	/** Tracks base field of view for dynamic kick pulse smoothing. */
	float CurrentFieldOfView = 80.0f;
	float TargetFieldOfView = 80.0f;

	/** Current push charge accumulator ratio (0.0 to 1.0). */
	float CurrentPushChargeRatio = 0.0f;

	/** True while player holds the launch push charge button. */
	bool bIsChargingLaunchPush = false;

	/** Current departure trajectory steering angle in degrees. */
	float LaunchAimAngleDeg = 0.0f;

	/** Updates dynamic material instances with live physics metrics (angular velocity, launch charge, proximity). */
	void UpdateHighFidelityDynamicMaterials(float DeltaSeconds);
};
