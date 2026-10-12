/**
 * @file SpacePlayerController.cpp
 * @brief Player controller implementation — zero-latency two-button Enhanced Input handling and SOCD resolution.
 */

#include "Player/SpacePlayerController.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Components/MaceOrbitComponent.h"
#include "Components/MomentumTransferComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/LocalPlayer.h"

ASpacePlayerController::ASpacePlayerController()
{
	bShowMouseCursor = false;
}

void ASpacePlayerController::EnsureDefaultInputBindingsExist()
{
	// Build input actions procedurally if not assigned in editor details
	if (!SpinCounterClockwiseAction)
	{
		SpinCounterClockwiseAction = NewObject<UInputAction>(this, TEXT("IA_SpinCounterClockwise_Procedural"));
		SpinCounterClockwiseAction->ValueType = EInputActionValueType::Boolean;
	}

	if (!SpinClockwiseAction)
	{
		SpinClockwiseAction = NewObject<UInputAction>(this, TEXT("IA_SpinClockwise_Procedural"));
		SpinClockwiseAction->ValueType = EInputActionValueType::Boolean;
	}

	if (!ToggleKickModelAction)
	{
		ToggleKickModelAction = NewObject<UInputAction>(this, TEXT("IA_ToggleKickModel_Procedural"));
		ToggleKickModelAction->ValueType = EInputActionValueType::Boolean;
	}

	// Build default mapping context (A = CCW, D = CW, M = Toggle Kick Model)
	if (!DefaultMappingContext)
	{
		DefaultMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_SpaceDiving_Procedural"));
		DefaultMappingContext->MapKey(SpinCounterClockwiseAction, EKeys::A);
		DefaultMappingContext->MapKey(SpinClockwiseAction, EKeys::D);
		DefaultMappingContext->MapKey(ToggleKickModelAction, EKeys::M);
	}
}

void ASpacePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	EnsureDefaultInputBindingsExist();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// A key: Counter-Clockwise spin
		EnhancedInputComponent->BindAction(
			SpinCounterClockwiseAction,
			ETriggerEvent::Started,
			this,
			&ASpacePlayerController::OnSpinCounterClockwiseStarted
		);
		EnhancedInputComponent->BindAction(
			SpinCounterClockwiseAction,
			ETriggerEvent::Completed,
			this,
			&ASpacePlayerController::OnSpinCounterClockwiseCompleted
		);

		// D key: Clockwise spin
		EnhancedInputComponent->BindAction(
			SpinClockwiseAction,
			ETriggerEvent::Started,
			this,
			&ASpacePlayerController::OnSpinClockwiseStarted
		);
		EnhancedInputComponent->BindAction(
			SpinClockwiseAction,
			ETriggerEvent::Completed,
			this,
			&ASpacePlayerController::OnSpinClockwiseCompleted
		);

		// M key: Toggle kick model for feel calibration
		EnhancedInputComponent->BindAction(
			ToggleKickModelAction,
			ETriggerEvent::Started,
			this,
			&ASpacePlayerController::OnToggleKickModelTriggered
		);
	}
}

void ASpacePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	EvaluateActiveSpin();
}

void ASpacePlayerController::OnSpinCounterClockwiseStarted()
{
	bIsCounterClockwisePressed = true;
	LastCounterClockwisePressTime = FPlatformTime::Seconds();
	EvaluateActiveSpin();
}

void ASpacePlayerController::OnSpinCounterClockwiseCompleted()
{
	bIsCounterClockwisePressed = false;
	EvaluateActiveSpin();
}

void ASpacePlayerController::OnSpinClockwiseStarted()
{
	bIsClockwisePressed = true;
	LastClockwisePressTime = FPlatformTime::Seconds();
	EvaluateActiveSpin();
}

void ASpacePlayerController::OnSpinClockwiseCompleted()
{
	bIsClockwisePressed = false;
	EvaluateActiveSpin();
}

void ASpacePlayerController::OnToggleKickModelTriggered()
{
	ToggleKickModel();
}

void ASpacePlayerController::ToggleKickModel()
{
	if (ASpaceAstronautCharacter* Astronaut = Cast<ASpaceAstronautCharacter>(GetPawn()))
	{
		if (UMomentumTransferComponent* Transfer = Astronaut->GetMomentumTransfer())
		{
			Transfer->ToggleKickModel();
		}
	}
}

void ASpacePlayerController::EvaluateActiveSpin()
{
	EMaceSpinDirection TargetDirection = EMaceSpinDirection::None;

	if (bIsCounterClockwisePressed && bIsClockwisePressed)
	{
		// SOCD: Last-Input-Priority resolution
		TargetDirection = (LastCounterClockwisePressTime > LastClockwisePressTime)
			? EMaceSpinDirection::CounterClockwise
			: EMaceSpinDirection::Clockwise;
	}
	else if (bIsCounterClockwisePressed)
	{
		TargetDirection = EMaceSpinDirection::CounterClockwise;
	}
	else if (bIsClockwisePressed)
	{
		TargetDirection = EMaceSpinDirection::Clockwise;
	}
	else
	{
		TargetDirection = EMaceSpinDirection::None;
	}

	// Transmit resolved intention immediately to the astronaut's orbit component
	if (ASpaceAstronautCharacter* Astronaut = Cast<ASpaceAstronautCharacter>(GetPawn()))
	{
		if (UMaceOrbitComponent* Orbit = Astronaut->GetMaceOrbit())
		{
			Orbit->SetHeldSpinDirection(TargetDirection);
		}
	}
}
