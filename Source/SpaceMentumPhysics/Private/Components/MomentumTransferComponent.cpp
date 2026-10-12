/**
 * @file MomentumTransferComponent.cpp
 * @brief Momentum transfer component — Step 1 scaffold.
 *
 * @emotional_intent See header. Crisp, punchy, instantaneous whiplash.
 */

#include "Components/MomentumTransferComponent.h"

UMomentumTransferComponent::UMomentumTransferComponent()
{
	// Enabled in Step 2: damping of lateral velocity runs every physics tick.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
