Here is the updated **Concept, Rules, and Physics** section formatted perfectly to match your existing Game Design Document. You can copy and paste this directly to your agent so it understands exactly how to code the physics, the environment, and the win/loss conditions based on our newly refined mechanics.

---

### 1.3 Updated High Concept & Environmental Context

**The Premise:** The player controls a stranded spacecraft (or astronaut) whose main engines have died, leaving them drifting at a constant base velocity. Their only tool for survival is a heavy, tethered mechanical appendage (the mace).
**The Environment:** Space is not empty here. The player is drifting through incredibly dense navigational hazards—shattered orbital space stations, dense asteroid belts, and spaceship graveyards. The player is always only seconds away from their next obstacle or anchor point.

---

### 1.4 Creator Vision & Emotional Resonance (Log Update)

| Aspect | Creator's Vision & Direct Quote | Underlying Emotional Resonance & Design Meaning |
| --- | --- | --- |
| **Launch Push-Off** | *"the gane starts with me pushing on an asteroid towards the tragactory i want. al arrow to the direction, a map and speed meter is also needed and the push power too"* | **Agency & Physical Grounding.** Every run begins with muscular intention. The player establishes their initial line by pushing against a tangible rock, giving immediate tactile meaning to Newtonian reaction force. |
| **The Void / Soft-Lock** | *"If the engine calculates the player's trajectory will not intersect... trigger a dramatic failure state... in the next 13 to 40 seconds."* | **Existential Dread.** The terror of space isn't just crashing; it's drifting helplessly. The game must mercilessly cut off doomed runs to keep the adrenaline high and pacing fast. |
| **Environmental Density** | *"abandon realistic space spacing. The levels need to be incredibly dense navigational hazards... asteroid belt also can be a way."* | **Claustrophobia & High APM.** The player should never feel relaxed. Space is cluttered, dangerous, and requires constant kinetic rhythm to survive. |
| **Player Skill Ceiling** | *"The 'Blind Math' Frustration is not a problem it is to learn the intuition."* | **Mastery through Suffering.** No predictive curved simulation lines. The player must physically *feel* the weight and momentum through trial, error, and eventual mastery. |

---

## 2. Core Physics & Gameplay Rules (The Engine Mandates)

Your agent must strictly adhere to these physics rules to create the gameplay loop.

### Rule 1: The Physics of Movement (Internal vs. External)

The game engine must calculate two entirely different types of movement based on Newton's Laws:

* **Internal Forces (The Dodge):** Swinging the mace in a vacuum cannot change the system's global Center of Mass (CoM). If the player swings the heavy appendage to the left, the ship's body is violently forced to the right. The player uses this to dodge obstacles, but if they pull the appendage back to neutral, their body snaps back to the CoM line. This creates a "physics debt."
* **Rotational Forces (The Spin):** Conservation of angular momentum is strictly enforced. If the player spins the appendage clockwise, the main body of the ship must instantly counter-rotate counter-clockwise.
* **External Forces (The Strike & The Push):** The ONLY way to permanently change the trajectory of the ship's Center of Mass is to physically strike or push the appendage against an external obstacle (e.g., the starting asteroid or oncoming hazards). The collision/push transfers momentum, granting the player a new directional velocity vector.

### Rule 2: Win & Progression Conditions

* **Propagation:** The ultimate goal is to navigate the dense asteroid field/debris and reach a designated spatial coordinate (the endpoint of the level or sector) using only the momentum mechanics described above.
* **Survival:** If the player has a valid physical path to alter their trajectory (e.g., an asteroid is within reach to strike), the game remains active. It acts as a real-time puzzle to identify the correct sequence of spins and strikes.

### Rule 3: Failure States (Game Over Conditions)

The game features two distinct and absolute failure states.

1. **Kinetic Impact (Death by Crashing):** If the player's main body hitbox intersects with an obstacle at a relative speed exceeding the survivable threshold, the ship is destroyed. Game over.
2. **Doom Detection (Death by The Void):** The engine must run a continuous background predictive trajectory check.
* **Condition:** If the player's velocity drops to near zero relative to the environment, OR they are drifting into empty space.
* **Check:** The engine looks ahead 13 to 40 seconds.
* **Execution:** If there are zero objects within the mace's maximum reach within that time window (meaning the player has absolutely no way to alter their speed or trajectory), the game must instantly trigger a dramatic "Lost to the Void" failure state. Do not let the player drift aimlessly; kill the run immediately to preserve pacing.

### Rule 4: UI & Flight Instrumentation ("Blind Math" vs. Telemetry)

* **No Predictive Trajectory Lines ("Blind Math"):** The UI must NOT draw predictive physics arcs, ghost lines, or forward simulation curves showing where the ship will travel seconds into the future. The player must calculate the mass displacement and collision outcomes intuitively.
* **Mandatory Flight Telemetry (Astronaut Cockpit):**
  1. **Directional Arrow:** Dynamic vector arrow showing departure angle at launch, and instantaneous heading in flight.
  2. **Tactical Navigational Radar / Map (Mini-Map):** Spatial overview displaying the corridor, nearby asteroids/hazards, and upcoming slalom target gates.
  3. **Speedometer (Speed Meter):** Real-time speed gauge in cm/s and km/h, breaking down forward velocity and lateral drift.
  4. **Push Power Meter:** Charge accumulator for launch push-offs, transitioning into rotational kinetic strike power during flight.