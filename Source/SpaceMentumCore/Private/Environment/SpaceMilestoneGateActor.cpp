/**
 * @file SpaceMilestoneGateActor.cpp
 * @brief Implementation of celestial milestone checkpoint actor.
 */

#include "Environment/SpaceMilestoneGateActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Audio/SpaceProceduralFlightAudioComponent.h"

ASpaceMilestoneGateActor::ASpaceMilestoneGateActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ArchMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArchMeshComponent"));
	ArchMeshComponent->SetupAttachment(SceneRoot);
	ArchMeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetBoxExtent(FVector(100.0f, 900.0f, 900.0f));
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetGenerateOverlapEvents(true);

	MilestoneLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MilestoneLight"));
	MilestoneLight->SetupAttachment(SceneRoot);
	MilestoneLight->SetIntensity(3500.0f);
	MilestoneLight->SetAttenuationRadius(1400.0f);
	MilestoneLight->SetLightColor(FLinearColor(0.0f, 0.94f, 1.0f));

	bHasBeenCrossed = false;
}

void ASpaceMilestoneGateActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (MilestoneLight)
	{
		MilestoneLight->SetLightColor(MilestoneConfig.GateColor);
	}
}

void ASpaceMilestoneGateActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ASpaceMilestoneGateActor::OnTriggerBeginOverlap);
	}
}

void ASpaceMilestoneGateActor::SetupMilestone(const FSectorMilestoneDefinition& InDefinition)
{
	MilestoneConfig = InDefinition;
	SetActorLocation(FVector(InDefinition.DistanceCm, 0.0f, 0.0f));

	if (MilestoneLight)
	{
		MilestoneLight->SetLightColor(InDefinition.GateColor);
	}
}

void ASpaceMilestoneGateActor::ResetMilestone()
{
	bHasBeenCrossed = false;
	if (MilestoneLight)
	{
		MilestoneLight->SetLightColor(MilestoneConfig.GateColor);
		MilestoneLight->SetIntensity(3500.0f);
	}
}

void ASpaceMilestoneGateActor::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bHasBeenCrossed)
	{
		return;
	}

	ASpaceAstronautCharacter* Astronaut = Cast<ASpaceAstronautCharacter>(OtherActor);
	if (!Astronaut)
	{
		return;
	}

	bHasBeenCrossed = true;

	UE_LOG(LogTemp, Log, TEXT("[SpaceMilestoneGateActor] Astronaut reached Milestone %d: %s at X=%.1f m (+%d pts)!"),
		MilestoneConfig.MilestoneIndex,
		*MilestoneConfig.MilestoneTitle,
		MilestoneConfig.DistanceCm * 0.01f,
		MilestoneConfig.ScoreBonus);

	// Pulse light intensity on crossing
	if (MilestoneLight)
	{
		MilestoneLight->SetIntensity(8000.0f);
	}

	// Play audio chime
	if (MilestoneCrossedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MilestoneCrossedSound, GetActorLocation());
	}

	if (Astronaut->GetFlightAudio())
	{
		Astronaut->GetFlightAudio()->PlayMilestoneHarmonicChime(MilestoneConfig.MilestoneIndex);
	}

	// Broadcast crossing
	if (OnMilestoneCrossed.IsBound())
	{
		OnMilestoneCrossed.Broadcast(MilestoneConfig.MilestoneIndex, MilestoneConfig.MilestoneTitle, MilestoneConfig.DistanceCm);
	}
}
