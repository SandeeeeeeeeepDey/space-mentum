/**
 * @file SpaceMentumCoreTypes.h
 * @brief Game-flow level enums and structs shared across the Core module.
 *
 * @creator_feel
 * "thrilling vulnerability mixed with heavy kinetic power" (GDD §0.1)
 *
 * @emotional_intent
 * A run is a single, unbroken plunge through space. These phases exist to frame that
 * plunge — a breath before, the relentless run, and an honest end — never to interrupt it.
 */

#pragma once

#include "CoreMinimal.h"
#include "SpaceMentumCoreTypes.generated.h"

/** High-level phase of a single run. Owned by ASpaceGameState. */
UENUM(BlueprintType)
enum class ESpaceRunPhase : uint8
{
	/** Before the plunge: camera settles, player grips the mace. */
	PreLaunch UMETA(DisplayName = "Pre-Launch"),
	/** Relentless forward drift — the actual game. */
	Running   UMETA(DisplayName = "Running"),
	/** The astronaut hit something. Show the moment, then offer a retry. */
	Crashed   UMETA(DisplayName = "Crashed"),
};
