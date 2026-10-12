/**
 * @file MaceOrbitComponent.cpp
 * @brief Mace orbit component — Step 1 scaffold.
 *
 * @emotional_intent See header. Heavy wind-up, readable sweep, rhythmic release.
 */

#include "Components/MaceOrbitComponent.h"

UMaceOrbitComponent::UMaceOrbitComponent()
{
	// Ticking is enabled in Step 2, when orbit integration is implemented.
	// It will run in TG_PrePhysics so the mace arc is resolved before Chaos substeps.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
