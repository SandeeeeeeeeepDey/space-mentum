/**
 * @file SpaceMentumDebugVisualizer.h
 * @brief Google DeepMind-style telemetry visualizer and real-time trajectory gizmo renderer.
 *
 * @creator_feel
 * "by spinning and stopping the spin in a pertiicular direction, one can stir their direction ... it has physics."
 *
 * @emotional_intent
 * Instant visual feedback for physical intuition. Designers and players should immediately see:
 * 1. The 360-degree orbit ring in space.
 * 2. The kinetic weight and speed of the mace head.
 * 3. The instantaneous kick vector that will fire if released right now.
 * 4. The projected future trajectory through the corridor ahead.
 *
 * Modular design: completely decoupled static helper struct. No overhead when debug drawing is disabled.
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/SpaceMentumPhysicsTypes.h"

class ASpaceAstronautCharacter;

/**
 * Telemetry and trajectory visualization helpers.
 */
struct SPACEMENTUMCORE_API FSpaceMentumDebugVisualizer
{
	/**
	 * Renders full 3D spatial debug gizmos for the mace orbit, tangent vector, and predicted flight path.
	 *
	 * @param WorldContextObject Actor or component with a valid UWorld.
	 * @param Character          Active astronaut character.
	 * @param DeltaTime          Frame delta time.
	 */
	static void DrawCharacterDebugGizmos(
		const UObject* WorldContextObject,
		const ASpaceAstronautCharacter* Character,
		float DeltaTime);

	/**
	 * Renders color-coded on-screen HUD telemetry lines with real-time physics measurements.
	 *
	 * @param Character Active astronaut character.
	 */
	static void RenderOnScreenTelemetry(const ASpaceAstronautCharacter* Character);
};
