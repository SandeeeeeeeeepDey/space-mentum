/**
 * @file AbilityDataAsset.h
 * @brief Designer-authored definition of a modular ability (tether extension, gravity pulse, magnetic pull, ...).
 *
 * @creator_feel
 * "ill include abilities and obsticles through time with flavour."
 *
 * @emotional_intent
 * Abilities are SPICE layered on top of the core spin-and-release — never a replacement
 * for it. Every ability should amplify the feeling of mastering heavy kinetic power
 * (GDD §1.2 Pillar 3: "Modular Subjectivity & Progression").
 *
 * Activation logic is intentionally absent until the ability system lands (post-prototype);
 * this asset defines the data contract so designers can start drafting abilities now.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityDataAsset.generated.h"

UCLASS(BlueprintType)
class SPACEMENTUMABILITIES_API UAbilityDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Human-readable name shown in UI. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Flavour text — write it in the Creator's voice. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	/** Unique tag identifying this ability, e.g. Ability.Mace.TetherExtension. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag AbilityTag;

	/** Seconds before the ability can be used again. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0", Units = "Seconds"))
	float CooldownSeconds = 5.0f;

	/** Seconds the effect lasts once activated (0 = instantaneous). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0", Units = "Seconds"))
	float DurationSeconds = 0.0f;
};
