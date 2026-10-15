/**
 * @file SpaceSectorExitPortalActor.cpp
 * @brief Implementation of celestial jump portal terminating successful gauntlet runs.
 */

#include "Environment/SpaceSectorExitPortalActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SpaceAstronautCharacter.h"
#include "GameModes/SpaceGameState.h"
#include "Subsystems/SpaceSectorProgressionSubsystem.h"
#include "Audio/SpaceProceduralFlightAudioComponent.h"

ASpaceSectorExitPortalActor::ASpaceSectorExitPortalActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	WarpRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarpRingMesh"));
	WarpRingMesh->SetupAttachment(SceneRoot);
	WarpRingMesh->SetCollisionProfileName(TEXT("NoCollision"));

	AccretionDiskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccretionDiskMesh"));
	AccretionDiskMesh->SetupAttachment(SceneRoot);
	AccretionDiskMesh->SetCollisionProfileName(TEXT("NoCollision"));

	EventHorizonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EventHorizonMesh"));
	EventHorizonMesh->SetupAttachment(SceneRoot);
	EventHorizonMesh->SetCollisionProfileName(TEXT("NoCollision"));

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetBoxExtent(FVector(150.0f, 950.0f, 950.0f));
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetGenerateOverlapEvents(true);

	PortalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PortalLight"));
	PortalLight->SetupAttachment(SceneRoot);
	PortalLight->SetIntensity(12000.0f);
	PortalLight->SetAttenuationRadius(2500.0f);
	PortalLight->SetLightColor(FLinearColor(0.0f, 0.95f, 1.0f));

	TargetSectorIndex = 1;
	PortalColor = FLinearColor(0.0f, 0.95f, 1.0f);
	AccretionDiskSpinSpeedDegPerSec = 45.0f;
	bHasBeenBreached = false;
}

void ASpaceSectorExitPortalActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (PortalLight)
	{
		PortalLight->SetLightColor(PortalColor);
	}
}

void ASpaceSectorExitPortalActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ASpaceSectorExitPortalActor::OnTriggerBeginOverlap);
	}
}

void ASpaceSectorExitPortalActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (AccretionDiskMesh)
	{
		AccretionDiskMesh->AddLocalRotation(FRotator(0.0f, 0.0f, AccretionDiskSpinSpeedDegPerSec * DeltaSeconds));
	}
}

void ASpaceSectorExitPortalActor::SetupPortal(const FSectorDefinition& InSectorDef)
{
	TargetSectorIndex = InSectorDef.SectorIndex;
	PortalColor = InSectorDef.PortalThemeColor;
	SetActorLocation(FVector(InSectorDef.CorridorLengthCm, 0.0f, 0.0f));

	if (PortalLight)
	{
		PortalLight->SetLightColor(PortalColor);
	}
}

void ASpaceSectorExitPortalActor::ResetPortal()
{
	bHasBeenBreached = false;
	if (PortalLight)
	{
		PortalLight->SetLightColor(PortalColor);
		PortalLight->SetIntensity(12000.0f);
	}
}

void ASpaceSectorExitPortalActor::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bHasBeenBreached)
	{
		return;
	}

	ASpaceAstronautCharacter* Astronaut = Cast<ASpaceAstronautCharacter>(OtherActor);
	if (!Astronaut)
	{
		return;
	}

	bHasBeenBreached = true;

	const FVector BreachLocation = Astronaut->GetActorLocation();
	const float BreachSpeed = Astronaut->GetVelocity().Size();

	UE_LOG(LogTemp, Log, TEXT("[SpaceSectorExitPortalActor] VICTORY! Astronaut breached Sector %d Jump Portal at X=%.1f m (Speed: %.1f cm/s = %.1f km/h)!"),
		TargetSectorIndex,
		BreachLocation.X * 0.01f,
		BreachSpeed,
		BreachSpeed * 0.036f);

	// 1. Play massive warp departure sound
	if (WarpJumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WarpJumpSound, BreachLocation);
	}

	if (Astronaut->GetFlightAudio())
	{
		Astronaut->GetFlightAudio()->PlayPortalWarpVacuumCollapse();
	}

	// 2. Pulse portal light to white super-nova flash
	if (PortalLight)
	{
		PortalLight->SetLightColor(FLinearColor::White);
		PortalLight->SetIntensity(35000.0f);
	}

	// 3. Complete run in GameState
	if (UWorld* World = GetWorld())
	{
		if (ASpaceGameState* GameState = World->GetGameState<ASpaceGameState>())
		{
			GameState->CompleteRunSuccess();
		}

		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (USpaceSectorProgressionSubsystem* Progression = GI->GetSubsystem<USpaceSectorProgressionSubsystem>())
			{
				Progression->CompleteActiveSectorRun(true);
			}
		}
	}

	// 4. Broadcast delegate
	if (OnPortalBreached.IsBound())
	{
		OnPortalBreached.Broadcast(TargetSectorIndex, BreachLocation, BreachSpeed);
	}
}
