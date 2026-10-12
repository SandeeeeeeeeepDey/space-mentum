/**
 * @file SpaceGameModeBase.cpp
 * @brief Wires the SpaceMentum framework classes together.
 */

#include "GameModes/SpaceGameModeBase.h"
#include "GameModes/SpaceGameState.h"
#include "Player/SpaceAstronautCharacter.h"
#include "Player/SpacePlayerController.h"

ASpaceGameModeBase::ASpaceGameModeBase()
{
	// C++ defaults. In Step 3, BP_GameMode (Content/Core/Blueprints) will subclass this and
	// swap DefaultPawnClass for BP_SpaceAstronaut so artists can attach meshes and VFX.
	DefaultPawnClass = ASpaceAstronautCharacter::StaticClass();
	PlayerControllerClass = ASpacePlayerController::StaticClass();
	GameStateClass = ASpaceGameState::StaticClass();
}
