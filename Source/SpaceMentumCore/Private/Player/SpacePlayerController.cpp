/**
 * @file SpacePlayerController.cpp
 * @brief Player controller — Step 1 scaffold.
 */

#include "Player/SpacePlayerController.h"

ASpacePlayerController::ASpacePlayerController()
{
	// The mouse plays no part in steering — only A/D. Keep the cursor out of the way.
	bShowMouseCursor = false;
}
