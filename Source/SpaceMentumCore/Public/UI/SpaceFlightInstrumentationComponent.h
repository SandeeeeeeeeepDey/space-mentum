/**
 * @file SpaceFlightInstrumentationComponent.h
 * @brief Cockpit flight instrumentation engine delivering telemetry for Radar, Speedometer, Direction Arrow, and Push Power.
 *
 * @creator_feel
 * "the gane starts with me pushing on an asteroid towards the tragactory i want.
 *  al arrow to the direction, a map and speed meter is also needed and the push power too"
 *
 * @emotional_intent
 * Rule 4 "Blind Math" Compliance: The instrumentation provides rich, tactile feedback of
 * CURRENT physical states (instantaneous speed, nearby radar contacts, heading vector, push charge)
 * without ever drawing predictive future trajectory arcs or ghost curves. The puzzle remains pure:
 * players must learn the intuition of Newtonian mechanics.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/SpaceMentumCoreTypes.h"
#include "SpaceFlightInstrumentationComponent.generated.h"

class ASpaceAstronautCharacter;
class ASpaceGameState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFlightTelemetryUpdated,
	const FFlightTelemetryData&, Telemetry
);

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMCORE_API USpaceFlightInstrumentationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceFlightInstrumentationComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Returns current live flight telemetry snapshot. */
	UFUNCTION(BlueprintPure, Category = "SpaceMentum|Instrumentation")
	const FFlightTelemetryData& GetCurrentTelemetry() const { return CurrentTelemetry; }

	/** Maximum search distance for tactical radar contacts (cm). Default: 12000 cm = 120m. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Radar")
	float RadarDetectionRangeCm = 12000.0f;

	/** Delegate broadcast every telemetry tick with updated instrumentation values. */
	UPROPERTY(BlueprintAssignable, Category = "SpaceMentum|Instrumentation")
	FOnFlightTelemetryUpdated OnFlightTelemetryUpdated;

private:
	/** Cached reference to owning astronaut character. */
	TWeakObjectPtr<ASpaceAstronautCharacter> OwningCharacter;

	/** Cached reference to game state for phase tracking. */
	TWeakObjectPtr<ASpaceGameState> CachedGameState;

	/** Active telemetry data payload. */
	FFlightTelemetryData CurrentTelemetry;

	/** Accumulator for radar spatial sweep frequency (scans at 20 Hz to optimize CPU). */
	float RadarScanAccumulator = 0.0f;

	/** Queries proximate obstacles and populates radar contacts array. */
	void RefreshRadarContacts();
};
