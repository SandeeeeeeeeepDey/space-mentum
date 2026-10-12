/**
 * @file AbilitySystemBridgeComponent.h
 * @brief Single attachment point between the astronaut and the modular ability layer.
 *
 * @creator_feel
 * "ill include abilities and obsticles through time with flavour."
 *
 * @emotional_intent
 * One clean socket for all future flavour. The astronaut never needs to know HOW an
 * ability works — it just owns this bridge. Whether we later adopt Unreal's Gameplay
 * Ability System or keep a lightweight custom one, only this component changes.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemBridgeComponent.generated.h"

class UAbilityDataAsset;

UCLASS(ClassGroup = (SpaceMentum), meta = (BlueprintSpawnableComponent))
class SPACEMENTUMABILITIES_API UAbilitySystemBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilitySystemBridgeComponent();

protected:
	/** Abilities granted at spawn. Empty for the first prototype — the core spin must stand alone. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpaceMentum|Abilities")
	TArray<TObjectPtr<UAbilityDataAsset>> StartingAbilities;
};
