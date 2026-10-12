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
A physics-driven celestial navigation game where an astronaut hurtles forward through deep space at constant base forward velocity. The player wields an orbital momentum anchor—a **heavy space mace**. By manipulating the centrifugal spin and timing sudden arrest/release of the mace's rotation, the player transfers angular momentum into linear lateral impulse, steering their trajectory to weave through perilous cosmic debris and hostile obstacles.

```
       [Forward Drift Velocity (Constant)]
                       ▲
                       │
                       │
                 (Astronaut)
                     O ───(Tether/Chain)─── [Heavy Mace]
                    / \                         ↷ (Rotational Angular Velocity)
                       
       [Release / Lock Arc] ───> Impulse Vector Transfer (Steering Impulse)
```

### 1.2 Core Gameplay Pillars
1. **Kinetic Inertia & Momentum Transfer:** Steering is not a simple joystick thrust; it is calculated through the conservation of angular momentum and tangential release physics.
2. **High-Flow Precision & Rhythm:** Spin up, time the apex, clamp the rotation to alter trajectory, and counter-spin to restabilize.
3. **Modular Subjectivity & Progression:** Extensible hooks for abilities (e.g., tether extension, gravity pulse, magnetic pull, elemental charges) and evolving obstacle archetypes.

---

## 2. Requirements Specification

### 2.1 Functional Requirements

#### F-1: Kinematics & Physics Engine
* **Constant Forward Vector:** Player maintains a continuous, uniform base forward velocity ($v_z = \text{const}$).
* **Mace Pendulum / Orbit Simulation:**
  * Orbit center pinned to player entity with configurable radius $r$ (tether length).
  * Variable angular velocity $\omega(t)$ driven by player input.
  * Mace mass $m_m$ vs player mass $m_p$ ratio affecting center of mass and steering impulse efficiency.
* **Momentum Transfer Mechanic (The Steering Impulse):**
  * When the mace is stopped, locked, or released at angle $\theta$, the tangential momentum $p_{\text{tangential}} = m_m \cdot (\omega \times r)$ imparts an opposing reaction force / vector shift onto the player's lateral trajectory ($v_x, v_y$).
  * Dynamic decay / drag coefficient on lateral velocity so player can stabilize or chain turns.
* **Collision Detection:** Continuous collision detection (CCD) for player and mace against stationary, orbiting, and kinetic obstacles.

#### F-2: Control & Input System
* **Spin Control:** Analog stick or mouse sweep / rotational gesture or dual-button spin accelerator.
* **Brake / Lock Mechanism:** Dedicated action button (e.g., Space / RT / Left Click) to arrest rotation or lock the mace in a designated quadrant.
* **Responsiveness & Low Input Latency:** Predictable, high-frequency input polling ensuring sub-16ms deterministic response.

#### F-3: Obstacle & Hazard Generation
* **Procedural / Segmented Track Spawner:** Dynamic spatial lane generation with configurable difficulty curves.
* **Hazard Types:**
  * *Static Obstacles:* Asteroids, space debris, dormant satellites.
  * *Reactive / Kinetic Obstacles:* Drifting comets, laser grids, gravitational anomalies.
  * *Mace-Destructible Obstacles:* Brittle crystalline barriers breakable only via high-velocity mace impact.

#### F-5: Camera & Visual Presentation (Third-Person Perspective / TPP)
* **Chase Camera Rig (SpringArm Dynamics):**
  * Placed behind and slightly elevated above the astronaut, tilted down toward the forward horizon.
  * Elastic rotational lag: Smooth interpolation on lateral impulse shifts so player feels the kinetic "yank" without motion sickness.
  * Dynamic Field of View (FOV): Widens subtly during high-spin states or speed boosts to heighten the sensation of hyper-velocity.
  * Visual Clearance: Camera framed to keep the rotating mace trajectory fully visible in the lower/mid peripheral frame while maintaining clear sightlines down-tunnel.

---

### 2.2 Non-Functional & Engineering Requirements

| Dimension | Specification |
| :--- | :--- |
| **Frame Rate & Latency** | Target stable 60–120 FPS; frame-independent fixed physics ticks (60Hz or 120Hz). |
| **Modularity & Decoupling** | Pure Separation of Concerns: Physics Simulation, Rendering, Input, and Game State. |
| **Deterministic Simulation** | Reproducible physics calculations to enable replays, ghost runs, and leaderboard validation. |
| **Configuration-Driven Design** | Balance variables (mass, tether length, restitution, damping) defined in external configs (JSON/YAML/Data Tables). |
| **Tooling & Profiling** | In-engine telemetry, trajectory prediction gizmos/visualizers, and debug overlays. |

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

2. **Physics & Continuous Collision Detection (CCD):**
   * **Chaos Physics Substepping:** Enabled in `Project Settings -> Physics -> Framerate -> Substepping`. Crucial so the high-speed rotating mace head does not tunnel through thin obstacles between frame renders.
   * **Momentum Transfer Implementation:** 
     * Mace head modeled as an unconstrained or cable/spring-jointed kinematic/rigid body with custom constraint solver.
     * When the `IA_ArrestSpin` action is triggered, calculate the instantaneous tangential velocity vector $\vec{v}_t = \vec{\omega} \times \vec{r}$.
     * Apply reactionary linear impulse $\vec{J} = - \alpha \cdot m_m \cdot \vec{v}_t$ directly to character's `UCharacterMovementComponent` / Root Rigid Body via `AddImpulse()`.

3. **Enhanced Input System:**
   * Context-sensitive `UInputMappingContext` (`IMC_DefaultSpaceDiving`).
   * Supports both 360-degree analog stick deflection and mouse delta sweeps for mace rotation.

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
│   │   │   └── Types/                   # Core structs, enums, GameplayTags
│   │   └── Private/
│   │
│   ├── SpaceMentumPhysics/              # Dedicated Kinematics & Momentum Solvers
│   │   ├── Public/
│   │   │   ├── Components/
│   │   │   │   ├── UMaceOrbitComponent.h         # Angular acceleration & orbit math
│   │   │   │   ├── UMomentumTransferComponent.h  # Tangential brake impulse generator
│   │   │   │   └── UForwardDriftComponent.h      # Uniform velocity maintainer
│   │   │   └── Solvers/
│   │   │       └── FTrajectoryPredictor.h        # Mathematical trajectory raycast/parabola
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
- [ ] **Kick Direction:** Reaction recoil (GDD §3.1.2, `J = -α·m·v_t`) vs carry-along yank. Decide by feel in the Step 2 prototype (`EMomentumKickModel`).
- [ ] **First Prototype Milestone:** Greybox level in UE5 focusing solely on the C++ momentum transfer feel and Cinemachine-like SpringArm responsiveness.
  *Step 1 (C++ project scaffold) is done. Progress is tracked in [PROGRESS.md](PROGRESS.md).*


