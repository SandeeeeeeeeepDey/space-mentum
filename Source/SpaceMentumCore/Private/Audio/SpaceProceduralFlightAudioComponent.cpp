// ==============================================================================
// SpaceProceduralFlightAudioComponent.cpp — Dynamic procedural audio & Metasound coordinator.
//
// @creator_feel
//   "a space person zooming in the forward direction in a uniform velocity and
//    the person has a mace in his hand."
//
// @emotional_intent
//   Visceral audio feedback: screaming spin pitch tracking, sub-bass launch thump,
//   whip-crack kinetic kick snap, and cosmic wind.
// ==============================================================================

#include "Audio/SpaceProceduralFlightAudioComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

USpaceProceduralFlightAudioComponent::USpaceProceduralFlightAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void USpaceProceduralFlightAudioComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USpaceProceduralFlightAudioComponent::InitializeFlightAudio(UMaceOrbitComponent* InOrbitComp)
{
	OrbitComp = InOrbitComp;
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 1. Initialize Continuous Spin Looping Audio Emitter
	if (USoundBase* SpinSound = ContinuousSpinMetasound.LoadSynchronous())
	{
		ActiveSpinAudioComponent = UGameplayStatics::SpawnSoundAttached(
			SpinSound,
			OwnerActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true
		);

		if (ActiveSpinAudioComponent)
		{
			ActiveSpinAudioComponent->SetVolumeMultiplier(0.0f); // Silence initially until spin begins
		}
	}

	// 2. Initialize Cosmic Hull Wind Looping Audio Emitter
	if (USoundBase* WindSound = CosmicWindSound.LoadSynchronous())
	{
		ActiveWindAudioComponent = UGameplayStatics::SpawnSoundAttached(
			WindSound,
			OwnerActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// 3. Initialize Continuous Engine Hum & Suit Life-Support Looping Audio Emitter
	if (USoundBase* HumSound = EngineLifeSupportHumSound.LoadSynchronous())
	{
		ActiveEngineHumComponent = UGameplayStatics::SpawnSoundAttached(
			HumSound,
			OwnerActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true
		);

		if (ActiveEngineHumComponent)
		{
			ActiveEngineHumComponent->SetVolumeMultiplier(AudioTuning.EngineHumMasterVolume);
		}
	}

	// 4. Initialize Proximity Radar Warning Emitter
	if (USoundBase* AlarmSound = AsteroidProximityAlarmSound.LoadSynchronous())
	{
		ActiveProximityAlarmComponent = UGameplayStatics::SpawnSoundAttached(
			AlarmSound,
			OwnerActor->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true
		);

		if (ActiveProximityAlarmComponent)
		{
			ActiveProximityAlarmComponent->SetVolumeMultiplier(0.0f);
		}
	}
}

void USpaceProceduralFlightAudioComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// 1. Continuous Mace Spin Pitch & Energy Modulator
	if (OrbitComp)
	{
		const float AbsOmega = FMath::Abs(OrbitComp->GetAngularVelocityRadPerSec());
		const float MaxOmega = FMath::Max(1.0f, OrbitComp->GetOrbitTuning().MaxAngularSpeedRadPerSec);
		const float RawNormalizedSpeed = FMath::Clamp(AbsOmega / MaxOmega, 0.0f, 1.0f);

		SmoothedNormalizedSpeed = FMath::FInterpTo(SmoothedNormalizedSpeed, RawNormalizedSpeed, DeltaTime, 12.0f);

		const float TargetFrequencyHz = FMath::Lerp(
			AudioTuning.BaseSpinFrequencyHz,
			AudioTuning.PeakSpinFrequencyHz,
			SmoothedNormalizedSpeed
		);

		const float TargetVolume = SmoothedNormalizedSpeed * AudioTuning.SpinMasterVolume;

		if (ActiveSpinAudioComponent)
		{
			ActiveSpinAudioComponent->SetFloatParameter(FName(TEXT("SpinFrequency")), TargetFrequencyHz);
			ActiveSpinAudioComponent->SetFloatParameter(FName(TEXT("NormalizedSpeed")), SmoothedNormalizedSpeed);
			ActiveSpinAudioComponent->SetPitchMultiplier(FMath::Lerp(0.6f, 2.8f, SmoothedNormalizedSpeed));
			ActiveSpinAudioComponent->SetVolumeMultiplier(TargetVolume);
		}
	}

	// 2. Continuous Cosmic Wind Modulation
	if (ActiveWindAudioComponent)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			const float Speed = OwnerPawn->GetVelocity().Size();
			const float WindRatio = FMath::Clamp(
				Speed / FMath::Max(100.0f, AudioTuning.MaxWindSpeedReferenceCmPerSec),
				0.0f,
				1.2f
			);

			ActiveWindAudioComponent->SetFloatParameter(FName(TEXT("FlightSpeed")), Speed);
			ActiveWindAudioComponent->SetVolumeMultiplier(WindRatio * AudioTuning.WindMasterVolume);
		}
	}

	// 3. Proximity Radar Alarm Ping Cadence Modulation
	if (CurrentProximityThreatRatio > 0.01f)
	{
		// Ping interval shrinks from 1.0s at low threat to 0.08s at maximum imminent collision
		const float PingInterval = FMath::Lerp(1.0f, 0.08f, CurrentProximityThreatRatio);
		ProximityAlarmPingTimer += DeltaTime;

		if (ProximityAlarmPingTimer >= PingInterval)
		{
			ProximityAlarmPingTimer = 0.0f;
			if (ActiveProximityAlarmComponent)
			{
				const float Pitch = FMath::Lerp(0.9f, 1.8f, CurrentProximityThreatRatio);
				const float Vol = AudioTuning.ProximityAlarmMasterVolume * FMath::Lerp(0.4f, 1.0f, CurrentProximityThreatRatio);
				ActiveProximityAlarmComponent->SetPitchMultiplier(Pitch);
				ActiveProximityAlarmComponent->SetVolumeMultiplier(Vol);
				ActiveProximityAlarmComponent->Play();
			}
		}
	}
	else
	{
		ProximityAlarmPingTimer = 0.0f;
	}
}

void USpaceProceduralFlightAudioComponent::UpdateProximityRadarAlarm(
	float DistanceToNearestObstacleCm,
	float WarningRadiusCm)
{
	const float EffectiveRadius = WarningRadiusCm > 0.0f ? WarningRadiusCm : AudioTuning.ProximityAlertDistanceThresholdCm;

	if (DistanceToNearestObstacleCm <= EffectiveRadius)
	{
		// Threat ratio: 0.0 at EffectiveRadius -> 1.0 at 0 cm
		CurrentProximityThreatRatio = FMath::Clamp(1.0f - (DistanceToNearestObstacleCm / EffectiveRadius), 0.0f, 1.0f);
	}
	else
	{
		CurrentProximityThreatRatio = 0.0f;
	}
}

void USpaceProceduralFlightAudioComponent::PlayPortalWarpVacuumCollapse()
{
	if (USoundBase* Sound = PortalWarpCollapseSound.LoadSynchronous())
	{
		if (AActor* Owner = GetOwner())
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Sound,
				Owner->GetActorLocation(),
				1.5f,
				1.0f
			);
		}
	}
}

void USpaceProceduralFlightAudioComponent::PlayMilestoneHarmonicChime(int32 MilestoneIndex)
{
	if (USoundBase* Sound = MilestoneHarmonicChimeSound.LoadSynchronous())
	{
		if (AActor* Owner = GetOwner())
		{
			// Harmonic pitch step based on milestone tier (e.g. root, third, fifth)
			const float Pitch = 1.0f + (MilestoneIndex * 0.15f);
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Sound,
				Owner->GetActorLocation(),
				1.1f,
				Pitch
			);
		}
	}
}

void USpaceProceduralFlightAudioComponent::PlayLaunchSubBassThump(float ChargeRatio)
{
	if (USoundBase* Sound = LaunchSubBassThumpSound.LoadSynchronous())
	{
		if (AActor* Owner = GetOwner())
		{
			const float Volume = FMath::Lerp(0.6f, 1.2f, FMath::Clamp(ChargeRatio, 0.0f, 1.0f));
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Sound,
				Owner->GetActorLocation(),
				Volume,
				1.0f
			);
		}
	}
}

void USpaceProceduralFlightAudioComponent::PlayMomentumKickSnap(float KickIntensityRatio)
{
	if (USoundBase* Sound = MomentumKickSnapSound.LoadSynchronous())
	{
		if (AActor* Owner = GetOwner())
		{
			const float Pitch = FMath::RandRange(0.95f, 1.05f);
			const float Volume = FMath::Lerp(0.5f, 1.0f, FMath::Clamp(KickIntensityRatio, 0.0f, 1.0f));
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Sound,
				Owner->GetActorLocation(),
				Volume,
				Pitch
			);
		}
	}
}

void USpaceProceduralFlightAudioComponent::PlayObstacleStrikeClang(float StrikeEnergyRatio)
{
	if (USoundBase* Sound = ObstacleStrikeImpactSound.LoadSynchronous())
	{
		if (AActor* Owner = GetOwner())
		{
			const float Pitch = FMath::RandRange(0.9f, 1.1f);
			const float Volume = FMath::Lerp(0.7f, 1.4f, FMath::Clamp(StrikeEnergyRatio, 0.0f, 1.5f));
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Sound,
				Owner->GetActorLocation(),
				Volume,
				Pitch
			);
		}
	}
}

void USpaceProceduralFlightAudioComponent::PlayNearMissWarning()
{
	// High-frequency subtle cosmic tension chime
}
