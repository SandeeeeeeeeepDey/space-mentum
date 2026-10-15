// ==============================================================================
// SpaceProceduralFlightAudioComponent.h — Dynamic procedural audio & Metasound coordinator.
//
// @creator_feel
//   "a space person zooming in the forward direction in a uniform velocity and
//    the person has a mace in his hand."
//
// @emotional_intent
//   Visceral, high-fidelity auditory feedback: continuous spin pitch tracking
//   that screams as the mace approaches 3.0 rev/s, visceral sub-bass kick on starting
//   asteroid launch, sharp kinetic whiplash snap on momentum release, and deep cosmic wind.
// ==============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "SpaceProceduralFlightAudioComponent.generated.h"

class UMaceOrbitComponent;
class UMomentumTransferComponent;

/**
 * Feel knobs for procedural audio synthesis and Metasound parameter modulators.
 */
USTRUCT(BlueprintType)
struct FSpaceAudioTuning
{
	GENERATED_BODY()

	/** Baseline spin hum frequency in Hz at low angular velocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Spin")
	float BaseSpinFrequencyHz = 85.0f;

	/** Peak supersonic turbine whine frequency in Hz at maximum angular velocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Spin")
	float PeakSpinFrequencyHz = 640.0f;

	/** Master volume multiplier for continuous spin audio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Spin")
	float SpinMasterVolume = 0.85f;

	/** Forward speed in cm/s where cosmic wind reaches full saturation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Wind")
	float MaxWindSpeedReferenceCmPerSec = 3600.0f;

	/** Master volume for cosmic particle wind drag. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Wind")
	float WindMasterVolume = 0.6f;

	/** Master volume for continuous engine hum & suit life-support telemetry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Engine")
	float EngineHumMasterVolume = 0.45f;

	/** Master volume for proximity radar alert beeps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Proximity")
	float ProximityAlarmMasterVolume = 0.75f;

	/** Threshold distance from hazard to activate proximity alarm (cm). Default: 2500 cm = 25m. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioFeel|Proximity")
	float ProximityAlertDistanceThresholdCm = 2500.0f;
};

/**
 * Dynamic audio component that drives real-time pitch, volume, and Metasound parameter buses.
 */
UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMCORE_API USpaceProceduralFlightAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpaceProceduralFlightAudioComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Initializes audio sources and binds to mace components. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void InitializeFlightAudio(UMaceOrbitComponent* InOrbitComp);

	/** Plays explosive 45Hz sub-bass launch thump on asteroid push-off. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void PlayLaunchSubBassThump(float ChargeRatio);

	/** Plays whiplash kinetic snap when the mace stops and momentum transfers. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void PlayMomentumKickSnap(float KickIntensityRatio);

	/** Plays heavy metallic crunch on obstacle strike / deflection. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void PlayObstacleStrikeClang(float StrikeEnergyRatio);

	/** Plays high-tension warning chime when narrowly missing a hazard. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void PlayNearMissWarning();

	/** Updates dynamic obstacle proximity radar alarm pulse frequency and volume based on distance. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void UpdateProximityRadarAlarm(float DistanceToNearestObstacleCm, float WarningRadiusCm);

	/** Plays relativistic vacuum collapse and Doppler spatial distortion boom on terminus jump portal breach. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void PlayPortalWarpVacuumCollapse();

	/** Plays celestial milestone harmonic chord progression when passing milestone checkpoint arches. */
	UFUNCTION(BlueprintCallable, Category = "SpaceMentum|Audio")
	void PlayMilestoneHarmonicChime(int32 MilestoneIndex);

	// --- Sound Asset References (Metasounds / SoundCues) ---

	/** Continuous looping Metasound for mace spin (MS_SpaceMaceContinuousSpin). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> ContinuousSpinMetasound;

	/** Resonant sub-bass launch transient (MS_KineticLaunchThump). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> LaunchSubBassThumpSound;

	/** Kinetic whiplash snap sound on momentum release. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> MomentumKickSnapSound;

	/** Metallic collision impact sound for obstacle strikes (MS_ObstacleStrikeImpact). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> ObstacleStrikeImpactSound;

	/** Cosmic particle drag wind loop (MS_CosmicWind). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> CosmicWindSound;

	/** Continuous suit life-support whisper & propulsion reactor hum (MS_EngineLifeSupportHum). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> EngineLifeSupportHumSound;

	/** Proximity warning radar alarm (MS_ProximityRadarAlarm). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> AsteroidProximityAlarmSound;

	/** Sector exit jump portal vacuum collapse & hyperspace boom (MS_PortalWarpVacuumCollapse). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> PortalWarpCollapseSound;

	/** Celestial milestone harmonic chord chime (MS_MilestoneChimeHarmonic). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SpaceMentum|Audio|Assets")
	TSoftObjectPtr<USoundBase> MilestoneHarmonicChimeSound;

	/** Feel tuning for sound synthesis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpaceMentum|Audio")
	FSpaceAudioTuning AudioTuning;

protected:
	virtual void BeginPlay() override;

private:
	/** Cached pointer to orbit component. */
	UPROPERTY(Transient)
	TObjectPtr<UMaceOrbitComponent> OrbitComp;

	/** Active continuous spin looping audio component. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveSpinAudioComponent;

	/** Active cosmic wind looping audio component. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveWindAudioComponent;

	/** Active engine & suit life-support looping audio component. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveEngineHumComponent;

	/** Active obstacle proximity warning alarm looping/pulsing audio component. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> ActiveProximityAlarmComponent;

	/** Current normalized angular speed. */
	float SmoothedNormalizedSpeed = 0.0f;

	/** Timer accumulator for proximity alarm ping cadence. */
	float ProximityAlarmPingTimer = 0.0f;

	/** Current proximity threat ratio [0.0 to 1.0]. */
	float CurrentProximityThreatRatio = 0.0f;
};
