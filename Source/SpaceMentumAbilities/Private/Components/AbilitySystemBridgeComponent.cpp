/**
 * @file AbilitySystemBridgeComponent.cpp
 * @brief Ability bridge — Step 1 scaffold.
 */

#include "Components/AbilitySystemBridgeComponent.h"
#include "Data/AbilityDataAsset.h"

UAbilitySystemBridgeComponent::UAbilitySystemBridgeComponent()
{
	// No per-frame work until abilities are implemented (post-prototype).
	PrimaryComponentTick.bCanEverTick = false;
}
