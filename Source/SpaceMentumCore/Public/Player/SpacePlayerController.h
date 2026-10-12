/**
 * @file SpacePlayerController.h
 * @brief Translates the player's two spin buttons into mace commands.
 *
 * @creator_feel
 * "for pc A and D looks good. only two buttons for now to spin the mace, moment of
 *  stopping is the moment of release. everything can be controles by this i think."
 *
 * @emotional_intent
 * Two buttons, one gesture. Hold A or D to wind the mace; let go and the stored spin
 * becomes your escape. The controller must add ZERO latency or smoothing between the
 * finger lifting and the kick firing (GDD §2.1 F-2 "sub-16ms deterministic response").
 *
 * Input contract (Step 2 implementation):
 *  - IA_SpinCounterClockwise (bool, hold) <- A   | mobile: left-thumb touch zone (if mobile is chosen)
 *  - IA_SpinClockwise        (bool, hold) <- D   | mobile: right-thumb touch zone
 *  - Released (no spin action held)        -> UMaceOrbitComponent release -> momentum kick
 * Input Actions / Mapping Context are created in C++ in Step 2 (no editor assets required),
 * then mirrored as IA_ / IMC_ assets in Content/Core/Input once the editor is in use.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpacePlayerController.generated.h"

UCLASS()
class SPACEMENTUMCORE_API ASpacePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASpacePlayerController();
};
