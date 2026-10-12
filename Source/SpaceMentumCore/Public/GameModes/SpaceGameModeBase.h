/**
 * @file SpaceGameModeBase.h
 * @brief Server-authoritative rules of a SpaceMentum run: which pawn, controller, and state classes are used.
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity"
 *
 * @emotional_intent
 * The game mode is the quiet stage manager. It puts the astronaut in space and gets
 * out of the way — the drama belongs to the mace.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SpaceGameModeBase.generated.h"

UCLASS()
class SPACEMENTUMCORE_API ASpaceGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASpaceGameModeBase();
};
