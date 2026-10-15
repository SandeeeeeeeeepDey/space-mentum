// ==============================================================================
// SpaceFlightVFXComponent.cpp — Multi-system Niagara particle coordinator.
//
// @creator_feel
//   "a space person zooming in the forward direction in a uniform velocity and
//    the person has a mace in his hand."
//
// @emotional_intent
//   High-octane Niagara particle telemetry and kinetic impact bursts.
// ==============================================================================

#include "VFX/SpaceFlightVFXComponent.h"
#include "Components/MaceOrbitComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Pawn.h"

USpaceFlightVFXComponent::USpaceFlightVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void USpaceFlightVFXComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USpaceFlightVFXComponent::InitializeVFX(
	UMaceOrbitComponent* InOrbitComp,
	USceneComponent* InMaceHeadAttachSocket,
	USceneComponent* InAstronautRoot)
{
	OrbitComp = InOrbitComp;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1. Initialize Mace Plasma Trail Ribbon Emitter
	if (UNiagaraSystem* TrailAsset = MacePlasmaTrailSystem.LoadSynchronous())
	{
		if (InMaceHeadAttachSocket)
		{
			ActiveMaceTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				TrailAsset,
				InMaceHeadAttachSocket,
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}
	}

	// 2. Initialize Hyperspace Cosmic Dust Streaks Emitter
	if (UNiagaraSystem* DustAsset = HyperspaceDustSystem.LoadSynchronous())
	{
		if (InAstronautRoot)
		{
			ActiveHyperspaceDustComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				DustAsset,
				InAstronautRoot,
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				true
			);
		}
	}
}

void USpaceFlightVFXComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// 1. Update Speed-Reactive Mace Ribbon Trail Parameters
	if (ActiveMaceTrailComponent && OrbitComp)
	{
		const float Omega = FMath::Abs(OrbitComp->GetAngularVelocityRadPerSec());
		const float MaxOmega = FMath::Max(1.0f, OrbitComp->GetOrbitTuning().MaxAngularSpeedRadPerSec);
		const float NormalizedSpeed = FMath::Clamp(Omega / MaxOmega, 0.0f, 1.0f);

		const FLinearColor CurrentColor = FMath::Lerp(
			VFXTuning.LowSpeedTrailColor,
			VFXTuning.PeakSpeedTrailColor,
			NormalizedSpeed
		);

		const float CurrentWidth = FMath::Lerp(
			VFXTuning.BaseRibbonWidthCm,
			VFXTuning.MaxRibbonWidthCm,
			NormalizedSpeed
		);

		ActiveMaceTrailComponent->SetColorParameter(FName(TEXT("User.RibbonColor")), CurrentColor);
		ActiveMaceTrailComponent->SetFloatParameter(FName(TEXT("User.NormalizedAngularSpeed")), NormalizedSpeed);
		ActiveMaceTrailComponent->SetFloatParameter(FName(TEXT("User.RibbonWidth")), CurrentWidth);
	}

	// 2. Update Directional Hyperspace Dust Streaks Parameters
	if (ActiveHyperspaceDustComponent)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			const FVector Velocity = OwnerPawn->GetVelocity();
			const float Speed = Velocity.Size();

			const float SpawnRate = FMath::GetMappedRangeValueClamped(
				FVector2D(0.0f, 3600.0f),
				FVector2D(VFXTuning.BaseDustSpawnRate, VFXTuning.MaxDustSpawnRate),
				Speed
			);

			ActiveHyperspaceDustComponent->SetVectorParameter(FName(TEXT("User.VelocityVector")), Velocity);
			ActiveHyperspaceDustComponent->SetFloatParameter(FName(TEXT("User.SpawnRate")), SpawnRate);
			ActiveHyperspaceDustComponent->SetFloatParameter(FName(TEXT("User.LinearSpeed")), Speed);
		}
	}
}

void USpaceFlightVFXComponent::TriggerKineticImpactBurst(
	FVector ImpactPoint,
	FVector ImpactNormal,
	float ImpactEnergyRatio)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UNiagaraSystem* BurstAsset = KineticImpactBurstSystem.LoadSynchronous())
	{
		const FRotator BurstRotation = ImpactNormal.ToOrientationRotator();
		UNiagaraComponent* BurstComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			BurstAsset,
			ImpactPoint,
			BurstRotation,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		if (BurstComp)
		{
			BurstComp->SetFloatParameter(FName(TEXT("User.ImpactEnergyRatio")), FMath::Clamp(ImpactEnergyRatio, 0.1f, 2.0f));
		}
	}
}

void USpaceFlightVFXComponent::TriggerLaunchExhaustBurst(
	FVector AnchorPoint,
	FVector DepartureDirection,
	float ChargeRatio)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UNiagaraSystem* ExhaustAsset = LaunchExhaustBurstSystem.LoadSynchronous())
	{
		// Exhaust vents backwards against the starting asteroid
		const FRotator ExhaustRotation = (-DepartureDirection).ToOrientationRotator();
		UNiagaraComponent* ExhaustComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			ExhaustAsset,
			AnchorPoint,
			ExhaustRotation,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		if (ExhaustComp)
		{
			ExhaustComp->SetFloatParameter(FName(TEXT("User.LaunchChargeRatio")), FMath::Clamp(ChargeRatio, 0.15f, 1.0f));
		}
	}
}
