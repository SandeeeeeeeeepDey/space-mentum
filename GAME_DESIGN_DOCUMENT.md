# SpaceMentum — Game Design & Technical Architecture Document
**Document Version:** 0.1.0-alpha  
**Status:** Concept & Technical Feasibility Assessment  
**Owner:** System Architecture & Game Design  

---

## 0. Prime Directive: Creator Emotional Anchor & Philosophy Rule

> ### 📜 Mandatory Project Law: Preserving Creator Emotions, Likings, Reasonings & Quotes
> **RULE:** In all project documentations (`.md` files), architectural specifications, code comments, class docstrings, and tuning notes, the team **must explicitly document, embed, and honor the Creator's emotions, subjective likings, intuitive reasonings, and verbatim quotes**. 
>
> **Purpose:** We never build sterile, generic game systems. Mechanics, physics solvers, cameras, and shaders exist solely to translate the Creator’s visceral "feel" and emotional excitement into interactive reality. Every teammate reading any file must immediately understand the underlying emotion and vision that inspired that specific subsystem.

---

### 0.1 Creator Vision, Emotional Resonance & Verbatim Quotes Log

| Aspect | Creator's Vision & Direct Quote | Underlying Emotional Resonance & Design Meaning |
| :--- | :--- | :--- |
| **The Core Sensation** | *"the concept is a space person zooming in the forward direction in a uniform velocity and the person has a mace in his hand. by spinning and stopping the spin in a pertiicular direction, one can stir their direction to move away from obsticles. ill include abilities and obsticles through time with flavour. it has physics."* | **Thrilling vulnerability mixed with heavy kinetic power.** The feeling of relentless forward speed where your only salvation is mastering the physical weight and centrifugal kick of a spinning weapon to whip yourself out of harm's way. |
| **Quality Standard** | *"lets go for quality and hence unreal engine make most sense to me"* | **Uncompromising visual and physical excellence.** Rejection of cheap or lightweight shortcuts; desire for a rich, breathtaking, cinematic AAA space odyssey where space feels vast and the mace feels genuinely heavy. |
| **Documentation & Code Philosophy** | *"it is a rule to add my emotions, likings, reasonings, quotes too in all documentations .mds, docstrings, comments, everywhere so we never deviate from my feels."* | **Soul-driven engineering.** Code and documentation must never lose touch with human passion and aesthetic intent. Comments must explain the *emotional goal* of a variable or function, not just the raw arithmetic. |
| **The Void / Soft-Lock** | *"If the engine calculates the player's trajectory will not intersect... trigger a dramatic failure state... in the next 13 to 40 seconds."* | **Existential Dread.** The terror of space isn't just crashing; it's drifting helplessly. The game must mercilessly cut off doomed runs to keep the adrenaline high and pacing fast. |
| **Environmental Density** | *"abandon realistic space spacing. The levels need to be incredibly dense navigational hazards... asteroid belt also can be a way."* | **Claustrophobia & High APM.** The player should never feel relaxed. Space is cluttered, dangerous, and requires constant kinetic rhythm to survive. |
| **Player Skill Ceiling** | *"The 'Blind Math' Frustration is not a problem it is to learn the intuition."* | **Mastery through Suffering.** No UI hand-holding, no ghost trajectory lines. The player must physically *feel* the weight and momentum through trial, error, and eventual mastery. |

---

### 0.2 Implementation Standard for Code Comments & Docstrings
When writing C++, Blueprints, shaders, or configs in this project, every file header and critical function must follow this standard:

```cpp
/**
 * @file UMomentumTransferComponent.h
 * @brief Translates mace centrifugal deceleration into astronaut steering vector.
 *
 * @creator_feel
 * "by spinning and stopping the spin in a pertiicular direction, one can stir their direction..."
 *
 * @emotional_intent
 * The player should feel a sudden, satisfying "whiplash" kick. It must feel like grabbing
 * onto a physical pivot in zero-g, transferring raw rotational violence into clean evasion.
 * Avoid mushy linear interpolation; prioritize a crisp, punchy kinetic impulse.
 */
```

---

## 1. Executive Summary & Core Concept

### 1.1 High Concept
A physics-driven celestial navigation game where a stranded astronaut/spacecraft hurtles through deep space at constant base forward velocity. The player wields an orbital momentum anchor—a **heavy space mace**. By manipulating the centrifugal spin and timing sudden mace strikes against environmental obstacles, the player transfers momentum, steering their trajectory to weave through perilous cosmic debris fields.

```
       [Forward Drift Velocity (Constant)]
                       ▲
                       │
                       │
                 (Astronaut)
                     O ───(Tether/Chain)─── [Heavy Mace]
                    / \                         ↷ (Rotational Angular Velocity)
                       
       [Release / Lock Arc] ───> Internal Body Counter-Displacement (The Dodge)
       [Obstacle Strike]    ───> External Momentum Transfer (Permanent Trajectory Change)
```

### 1.2 Core Gameplay Pillars
1. **Newtonian Duality (Internal vs. External Forces):** Swinging in vacuum displaces the body relative to the Center of Mass (The Dodge with "physics debt"), while environmental strikes permanently deflect trajectory.
2. **Dense Navigational Gauntlets:** Claustrophobic orbital fields, derelict graveyards, and dense asteroid belts demand continuous spatial rhythm.
3. **Existential Dread & Void Termination:** Ruthless predictive doom detection terminates hopeless trajectories 13–40 seconds before aimless drift sets in.
4. **Pure Intuition ("Blind Math"):** No crutches, no trajectory prediction lines—players must intuitively internalize the mass ratio displacement.

### 1.3 Updated High Concept & Environmental Context

* **The Premise:** The player controls a stranded spacecraft (or astronaut) whose main engines have died, leaving them drifting at a constant base velocity. Their only tool for survival is a heavy, tethered mechanical appendage (the mace).
* **The Environment:** Space is not empty here. The player is drifting through incredibly dense navigational hazards—shattered orbital space stations, dense asteroid belts, and spaceship graveyards. The player is always only seconds away from their next obstacle or anchor point.

### 1.4 The Launch Sequence (Starting Asteroid Push-Off)

> **@creator_feel & Direct Mandate:**
> *"the gane starts with me pushing on an asteroid towards the tragactory i want. al arrow to the direction, a map and speed meter is also needed and the push power too"*

* **Origin Anchor:** The run does not begin floating in vacuum at arbitrary velocity. Instead, the astronaut begins anchored firmly against a celestial starting asteroid at the sector threshold ($X = 0$).
* **Kinetic Fulcrum:** The astronaut plants the heavy mechanical appendage (kinetic ram) directly against the asteroid's surface, using its immense mass as a mechanical reaction base.
* **Trajectory Aiming (Direction Arrow):** The player uses directional controls (A/D or aim controls) to align the mechanical arm along their desired departure trajectory, visualized by a clear, high-contrast holographic **Direction Arrow**.
* **Push Power Accumulation:** The player charges the **Push Power Meter** ($0\% \to 100\%$). Holding the charge compresses the hydraulic ram of the mechanical appendage, building up explosive spring/pneumatic energy.
* **Explosive Departure:** Upon release, the appendage fires violently against the starting asteroid ($m_{\text{asteroid}} \gg m_{\text{player}}$), transferring a massive initial impulse $\vec{J}_{\text{launch}}$. This propels the astronaut into the dense hazard gauntlet with chosen forward speed ($v_x$) and lateral vector ($v_y, v_z$), seamlessly initiating the flight phase!

### 1.5 Creator Vision & Emotional Resonance (Master Log)

| Aspect | Creator's Vision & Direct Quote | Underlying Emotional Resonance & Design Meaning |
| --- | --- | --- |
| **Launch Push-Off** | *"the gane starts with me pushing on an asteroid towards the tragactory i want. al arrow to the direction, a map and speed meter is also needed and the push power too"* | **Agency & Physical Grounding.** Every run begins with muscular intention. The player establishes their initial line by pushing against a tangible rock, giving immediate tactile meaning to Newtonian reaction force. |
| **The Void / Soft-Lock** | *"If the engine calculates the player's trajectory will not intersect... trigger a dramatic failure state... in the next 13 to 40 seconds."* | **Existential Dread.** The terror of space isn't just crashing; it's drifting helplessly. The game must mercilessly cut off doomed runs to keep the adrenaline high and pacing fast. |
| **Environmental Density** | *"abandon realistic space spacing. The levels need to be incredibly dense navigational hazards... asteroid belt also can be a way."* | **Claustrophobia & High APM.** The player should never feel relaxed. Space is cluttered, dangerous, and requires constant kinetic rhythm to survive. |
| **Player Skill Ceiling** | *"The 'Blind Math' Frustration is not a problem it is to learn the intuition."* | **Mastery through Suffering.** No predictive curved simulation lines. The player must physically *feel* the weight and momentum through trial, error, and eventual mastery. |

---

## 2. Core Physics & Gameplay Rules (The Engine Mandates)

The game engine and physics solvers must strictly adhere to these physics rules to create the gameplay loop:

### Rule 1: The Physics of Movement (Internal vs. External)

The game engine must calculate two entirely different types of movement based on Newton's Laws:

* **Internal Forces (The Dodge):** Swinging the mace in a vacuum cannot change the system's global Center of Mass (CoM). If the player swings the heavy appendage to the left, the ship's body is violently forced to the right ($m_{\text{ship}} \Delta \vec{r}_{\text{ship}} + m_{\text{mace}} \Delta \vec{r}_{\text{mace}} = 0$). The player uses this to dodge obstacles, but if they pull the appendage back to neutral, their body snaps back to the CoM line. This creates a "physics debt."
* **Rotational Forces (The Spin):** Conservation of angular momentum is strictly enforced ($I_{\text{ship}} \omega_{\text{ship}} + I_{\text{mace}} \omega_{\text{mace}} = 0$). If the player spins the appendage clockwise, the main body of the ship must instantly counter-rotate counter-clockwise.
* **External Forces (The Strike & The Push):** The **ONLY** way to permanently change the trajectory of the ship's Center of Mass is to physically strike or push the appendage against an external obstacle (e.g., the starting asteroid or oncoming navigational hazards). The collision/push transfers momentum, granting the player a new directional velocity vector $\Delta \vec{v}_{\text{CoM}} = \frac{\vec{J}_{\text{ext}}}{M_{\text{system}}}$.

### Rule 2: Win & Progression Conditions

* **Propagation:** The ultimate goal is to navigate the dense asteroid field/debris and reach a designated spatial coordinate (the endpoint of the level or sector) using only the momentum mechanics described above.
* **Survival:** If the player has a valid physical path to alter their trajectory (e.g., an asteroid is within reach to strike), the game remains active. It acts as a real-time puzzle to identify the correct sequence of spins and strikes.

### Rule 3: Failure States (Game Over Conditions)

The game features two distinct and absolute failure states:

1. **Kinetic Impact (Death by Crashing):** If the player's main body hitbox intersects with an obstacle at a relative speed exceeding the survivable threshold, the ship is destroyed. Game over.
2. **Doom Detection (Death by The Void):** The engine must run a continuous background predictive trajectory check.
   * **Condition:** If the player's velocity drops to near zero relative to the environment, OR they are drifting into empty space.
   * **Check:** The engine looks ahead 13 to 40 seconds.
   * **Execution:** If there are zero objects within the mace's maximum reach within that time window (meaning the player has absolutely no way to alter their speed or trajectory), the game must instantly trigger a dramatic "Lost to the Void" failure state. Do not let the player drift aimlessly; kill the run immediately to preserve pacing.

### Rule 4: UI & Flight Instrumentation ("Blind Math" vs. Telemetry)

To balance hard-core Newtonian intuition with situational awareness, the interface strictly delineates **Telemetry Instruments** from **Predictive Assistance**:

* **Strict "Blind Math" Boundary (No Predictive Simulation Arcs):**
  * The UI must **NOT** draw predictive physics arcs, forward ghost trajectories, or automated aim lines showing where the ship will travel 5 seconds in the future.
  * The player must calculate the mass displacement and collision outcomes in their mind.
* **Mandatory Flight Instruments (Astronaut Cockpit Telemetry):**
  1. **Directional Vector Arrow:** Displays the launch aim vector at departure, and the active instantaneous flight heading during movement.
  2. **Tactical Navigational Radar / Map (Mini-Map):** Real-time spatial radar displaying the astronaut's position, corridor boundary bounds, proximate hazards (asteroids, station hulks), and slalom target waypoints.
  3. **Cybernetic Speedometer (Speed Meter):** High-precision readout in cm/s and km/h, breaking down forward velocity ($v_x$) and lateral drift ($v_y, v_z$) with visual kick acceleration pulses.
  4. **Push Power Meter (Kinetic Potential Gauge):** Visualizes the launch push charge accumulator ($0\% \to 100\%$) during push-off phase, and the live kinetic energy stored in the rotating appendage ($E_k = \frac{1}{2} m_m v_t^2$) during flight.

---

### 2.1 Functional Requirements (Updated Specification)

#### F-1: Kinematics & Dual-Mode Physics Solver
* **Center of Mass (CoM) Drift:** Continuous uniform base forward drift vector for system CoM ($v_{\text{CoM}, z} = \text{const}$).
* **Two-Body Internal Displacement Solver (The Dodge):**
  * Evaluates positions of Ship ($m_s$) and Mace ($m_m$) about system CoM with default mass ratio $m_m : m_s = 1 : 4$.
  * Lateral displacement: $\vec{r}_s = - \frac{m_m}{m_s} \vec{r}_m$.
  * Total angular momentum conservation: counter-rotates ship orientation against mace angular velocity.
* **External Strike Momentum Transfer Solver:**
  * Detects mace head collisions with environmental obstacles (asteroids, station debris, cosmic hulks).
  * Calculates collision restitution, normal force, and transferred impulse $\vec{J}_{\text{ext}}$, updating system CoM velocity vector $\vec{v}_{\text{CoM}}$.
* **Continuous Collision Detection (CCD):** Active on ship body and mace head to eliminate high-speed tunneling through obstacles.

#### F-2: Control & Input System
* **Dual-Button Spin Accelerator:** Simple, high-precision two-button interface (A = Counter-Clockwise, D = Clockwise) with Last-Input-Priority SOCD resolution.
* **Immediate Tangential Arrest / Strike Release:** Neutral input state (letting go) halts rotational drive, locking mace at current angle to strike or clear obstacles.

#### F-3: Dense Navigational Hazard Spawner
* **Environmental Archetypes:** Shattered orbital stations, high-density asteroid belts, derelict spaceship graveyards.
* **Tight Spatial Cadence:** Density tuned such that reachable obstacles appear every 2–5 seconds, ensuring constant kinetic options and preventing accidental soft-locks during optimal play.

#### F-4: Predictive Doom Detection Subsystem ("The Void")
* **Background Lookahead Horizon:** Continuously evaluates future CoM trajectory between 13.0s and 40.0s forward window.
* **Mace Reach Sweep:** Verifies if any valid strike target intersects the cylinder defined by $\vec{r}_{\text{CoM}}(t) \pm R_{\text{mace}}$.
* **Terminal Trigger:** If count of reachable anchors in the lookahead window == 0, immediately trigger cinematic "Lost to the Void" sequence.

#### F-5: Visual Presentation & "Blind Math" UI
* **No Predictive Lines:** Zero HUD ghost trajectories or forward aim vectors.
* **Physical Cues Only:** Thruster sparks, tether cable tension vibrations, plasma ribbon trails, and dynamic camera FOV/shake provide all necessary intuitive feedback.

#### F-6: Flight Instrumentation & Navigational Radar Specification
* **Directional Vector Arrow:** 3D holographic arrow mesh originating from the astronaut's Center of Mass, aligning with launch aim during push-off, and indicating current velocity vector during flight.
* **Tactical Navigational Radar / Mini-Map:** Real-time HUD radar overlay showing relative coordinates $(x, y, z)$ of obstacles within a 120m lookahead sphere, color-coded by hazard type (asteroids: amber, debris: orange, gates: cyan).
* **Cybernetic Speedometer:** Real-time dual-unit display (cm/s and km/h), featuring dynamic gauge fill, forward velocity meter, and lateral drift rate.
* **Push Power Meter:** Dynamic charge bar displaying $0\% \to 100\%$ force accumulator during launch, transitioning into a rotational kinetic energy gauge during continuous flight.

#### F-7: Launch Push-Off Phase Mechanics
* **Phase Initialization (`ESpaceRunPhase::LaunchPushOff`):** Player spawns stationary, tethered against the starting asteroid.
* **Trajectory Angle Control:** A/D keys smoothly steer the direction arrow around the starting asteroid tangent plane.
* **Charge Accumulator:** Holding the push action builds launch impulse up to maximum allowable rating ($F_{\text{max}} = 12000\,\text{N}$, $v_{\text{launch}} = 3000\,\text{cm/s}$).
* **Transition to Flight:** Releasing the charge fires the mechanical ram, applies $\vec{J}_{\text{launch}}$, and transitions game state to `ESpaceRunPhase::Running`.

---

## 3. Selected Engine: Unreal Engine 5 (UE 5.4+) Technical Strategy

The project is committed to **Unreal Engine 5** to achieve high-end visual fidelity, physical presence, and cinematic cosmic atmosphere. Below is the technical execution plan tailored specifically to UE5 architecture.

### 3.1 Unreal Engine 5 Core Subsystem Mapping

```
                                  +-----------------------------------------------+
                                  |            Enhanced Input Component           |
                                  |    (IA_SpinMace, IA_ArrestSpin, IA_Ability)   |
                                  +-----------------------+-----------------------+
                                                          |
                                                          v
                                  +-----------------------------------------------+
                                  |            ASpaceAstronautCharacter           |
                                  |    - Base Forward Thrust Component            |
                                  |    - Trajectory State Machine                 |
                                  +-----------+-----------------------+-----------+
                                              |                       |
                     +------------------------+                       +------------------------+
                     v                                                                         v
+------------------------------------------+                             +------------------------------------------+
|            USpringArmComponent           |                             |             UMaceOrbitComponent          |
|  - Camera Lag Enabled                    |                             |  - Chaos Physics Rotational Constraint   |
|  - Dynamic TargetArmLength Curves        |                             |  - Angular Momentum Accumulator          |
|  - FOV Reactive Shifter                  |                             |  - Impulse Transfer on Arrest            |
+--------------------+---------------------+                             +--------------------+---------------------+
                     |                                                                         |
                     v                                                                         v
+------------------------------------------+                             +------------------------------------------+
|             UCameraComponent             |                             |           UNiagaraComponent              |
|        (Cinematic TPP Viewport)          |                             |   (Tangential Plasma Speed Ribbon Trails)|
+------------------------------------------+                             +------------------------------------------+
```

1. **Camera Rig (TPP Chase Cam):**
   * `USpringArmComponent` attached to character root with `bEnableCameraLag = true` and `bEnableCameraRotationLag = true`.
   * Custom camera lag speed curves to allow physical inertia feeling without disorienting the player.
   * `CameraFovModifier`: Dynamically interpolates Camera FOV ($80^\circ \to 105^\circ$) proportional to current forward speed + mace angular velocity $\omega$.

2. **Dual-Mode Physics & Continuous Collision Detection (CCD):**
   * **Chaos Physics Substepping:** Enabled in `Project Settings -> Physics -> Framerate -> Substepping`. Prevents thin obstacle tunneling during rapid mace rotations.
   * **Two-Body Internal Displacement Solver (The Dodge):**
     * Solves $m_s \Delta \vec{r}_s + m_m \Delta \vec{r}_m = 0$.
     * As the appendage swings into an arc, the ship displacement is calculated as $\vec{r}_s = - \frac{m_m}{m_s} \vec{r}_m$ (default 1:4 ratio).
     * Rotational counter-torque rotates ship body inversely to mace angular velocity ($I_s \vec{\omega}_s + I_m \vec{\omega}_m = 0$).
     * Global Center of Mass (CoM) trajectory remains strictly conserved in vacuum; pulling mace to neutral incurs "physics debt" and snaps body back to original CoM axis.
   * **External Strike Momentum Transfer (The Strike):**
     * The ONLY mechanism for modifying total system CoM velocity.
     * When the rotating mace impacts an external obstacle (asteroid, station bulkhead), impulse $\vec{J}_{\text{ext}} = \int \vec{F}_{\text{collision}} dt$ is transferred to the system Center of Mass, creating a new permanent directional velocity vector.
   * **Predictive Doom Detection ("The Void"):**
     * Executed by `UDoomDetectionSubsystem` / `FDoomTrajectoryEvaluator`.
     * Sweeps forward 13s–40s along current CoM velocity vector $\vec{v}_{\text{CoM}}$.
     * If relative speed $\approx 0$ or zero reachable obstacles exist within tether radius $R_{\text{reach}}$ along the forward window, triggers immediate "Lost to the Void" termination.

3. **Enhanced Input & Blind Math UI Pipeline:**
   * **Two-Button Control:** High-frequency polling on A (CCW) and D (CW) with SOCD Last-Input-Priority.
   * **Strict "Blind Math" Policy:** Zero trajectory arcs, predictive ghost lines, or aim reticles rendered in viewport. Player learns spatial timing and 1:4 displacement entirely through tactile rhythm and visual cues.

4. **Visual & VFX Pipeline:**
   * **Niagara Systems:**
     * `NS_MacePlasmaTrail`: Particle ribbon spawned along the mace's orbital path; color shifts from cyan to ultraviolet as rotational speed $\omega$ increases.
     * `NS_HyperspaceDust`: Directional space dust particles aligned with forward vector $v_z$ providing instant speed reference for the human eye.
     * `NS_KineticImpactBurst`: Burst particles on obstacle destruction.
   * **Lumen & Materials:** Real-time emissive lighting on cosmic hazards and energy chains; deep space skybox with volumetric cosmic dust.

---

## 4. DeepMind-Grade Modular Folder & Architecture Specification

To ensure a multidisciplinary team can work frictionlessly without merge conflicts, code and content are separated into modular, self-contained sub-modules and data-driven configuration assets:

```
SpaceMentum/
├── Config/
│   ├── DefaultEngine.ini                # Chaos substepping, collision channels, rendering
│   ├── DefaultInput.ini                 # Enhanced Input bindings
│   └── DefaultGame.ini
│
├── Source/
│   ├── SpaceMentum.Target.cs
│   ├── SpaceMentumEditor.Target.cs
│   │
│   ├── SpaceMentumCore/                 # Engine & Framework Foundations
│   │   ├── Public/
│   │   │   ├── GameModes/               # ASpaceGameModeBase, ASpaceGameState
│   │   │   ├── Player/                  # ASpaceAstronautCharacter, ASpacePlayerController
│   │   │   ├── Subsystems/              # UDoomDetectionSubsystem (13s-40s Void lookahead)
│   │   │   └── Types/                   # Core structs, enums, GameplayTags
│   │   └── Private/
│   │
│   ├── SpaceMentumPhysics/              # Dedicated Kinematics & Momentum Solvers
│   │   ├── Public/
│   │   │   ├── Components/
│   │   │   │   ├── UMaceOrbitComponent.h         # Angular acceleration & two-body orbit math
│   │   │   │   ├── UMomentumTransferComponent.h  # External strike collision momentum transfer
│   │   │   │   └── UForwardDriftComponent.h      # Uniform CoM base velocity maintainer
│   │   │   └── Solvers/
│   │   │       ├── FCenterOfMassKinematicsSolver.h # Internal dodge 1:4 displacement solver
│   │   │       └── FDoomTrajectoryEvaluator.h    # Lookahead collision & void evaluator
│   │   └── Private/
│   │
│   └── SpaceMentumAbilities/            # Modular Data-Driven Extensibility
│       ├── Public/
│       │   ├── Data/
│       │   │   ├── UMaceDefinitionDataAsset.h    # Configurable mace mass, radius, trail type
│       │   │   ├── UAbilityDataAsset.h           # Ability definitions
│       │   │   └── UObstacleArchetypeDataAsset.h # Hazard spawn weights, health, physics responses
│       │   └── Components/
│       │       └── UAbilitySystemBridgeComponent.h
│       └── Private/
│
├── Content/                             # Binary Asset Workspace (Strict Separation)
│   ├── Core/
│   │   ├── Blueprints/                  # BP_SpaceAstronaut (derives from C++), BP_GameMode
│   │   ├── Input/                       # IMC_Default, IA_Spin, IA_Arrest, IA_Ability
│   │   └── Data/                        # DA_Mace_Default, DA_PhysicsTuning, DA_ObstacleTable
│   ├── Environment/
│   │   ├── Meshes/                      # Asteroids, space wreckage, cosmic gates
│   │   └── Materials/                   # PBR master materials & material instances
│   ├── FX/
│   │   ├── Niagara/                     # NS_MaceTrail, NS_SpaceDust, NS_Shockwave
│   │   └── Shaders/                     # Volumetric nebula shaders
│   ├── Audio/
│   │   ├── Metasounds/                  # Dynamic audio: pitch scales with spin velocity
│   │   └── SoundCues/
│   └── Maps/
│       ├── L_Prototype_Greybox.umap     # Physics calibration and tuning arena
│       └── L_SpaceCorridor_Vertical.umap# Production vertical slice map
│
└── Docs/
    ├── GAME_DESIGN_DOCUMENT.md
    └── TECHNICAL_SPECIFICATIONS.md
```

---

## 5. Next Decisions & Feedback Checklist

- [x] **Perspective Choice:** Third-Person Perspective (TPP / Chase Cam) with dynamic FOV and spring arm damping.
- [x] **Engine Selection:** **Unreal Engine 5.8** with Chaos Physics Substepping, Enhanced Input, and Niagara.
  *Creator: "Unreal Engine 5.8, released in June 2026. is what i want to choose"*
- [x] **Input Paradigm:** **Two buttons only.** Hold A = spin counter-clockwise, hold D = spin clockwise. Releasing = the mace stops = momentum kick.
  *Creator: "for pc A and D looks good. only two buttons for now to spin the mace, moment of stopping is the moment of release. everything can be controles by this i think."*
- [x] **Mace Orbit Plane:** **Full 3D 360° cylindrical orbit.** The mace orbits in the astronaut's local YZ plane, so the release angle steers in both X and Y.
- [ ] **Target Platform:** PC vs Mobile is still open. Two abstract inputs keep both viable (A/D on PC, left/right thumb zones on mobile).
  *Creator: "im still not sure which would make more sense for me, a mobile game or a pc game"*
- [x] **Kick Direction:** Reaction recoil (GDD §3.1.2, `J = -α·m·v_t`, default Newtonian recoil) and carry-along yank models both fully supported and switchable in runtime tuning (`EMomentumKickModel`).
- [x] **First Prototype Milestone:** Greybox level in UE5 focusing solely on the C++ momentum transfer feel and Cinemachine-like SpringArm responsiveness (Step 3).
- [x] **Target Platform & Build Pipeline:** Win64 Shipping Package automation (`PackageSpaceMentumWin64.ps1` / `PackageSpaceMentumWin64.bat`) with standalone offline asset validation, alongside interactive 3D Web Companion standalone build pipeline (`package-dist.js`).
- [x] **Step 1–7 Milestones Complete:** Full project architecture, core feel, greybox arena, hazard spawner & CCD collisions, dynamic camera/VFX/audio, 3-sector progression with leaderboards, and high-fidelity asset & standalone packaging pipeline are verified. Progress tracked in [PROGRESS.md](PROGRESS.md).


