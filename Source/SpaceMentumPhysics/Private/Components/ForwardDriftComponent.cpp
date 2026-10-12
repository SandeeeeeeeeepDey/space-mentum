/**
 * @file ForwardDriftComponent.cpp
 * @brief Forward drift component — Step 1 scaffold.
 *
 * @emotional_intent See header. Relentless, uniform, sacred.
 */

#include "Components/ForwardDriftComponent.h"

UForwardDriftComponent::UForwardDriftComponent()
{
	// Enabled in Step 2 once velocity composition is implemented.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
