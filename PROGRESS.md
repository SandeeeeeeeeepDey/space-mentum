# SpaceMentum — Implementation Progress Log

> **Purpose:** Living hand-off log. If a session ends mid-task, any teammate (or agent) can resume
> from the **▶ Resume Here** section below. Update this file after every meaningful chunk of work.
>
> **Creator directive:** *"kepp writing progress in a progress.md file, so that even if your quota ends, that we can continue from where you left"*

---

## ▶ Resume Here

**Current step:** Step 1 (UE 5.8 C++ project scaffold) is ✅ **COMPLETE** and committed.
**Status:** ⏸ Waiting for the Creator: install UE 5.8, compile once, and answer the open questions below.
**Next action (Step 2: core feel prototype, C++):**
1. `UMaceOrbitComponent`: enable tick, integrate ω/θ from held spin direction, expose mace head world position.
2. `UMomentumTransferComponent`: on release, compute `J` with `FTrajectoryPredictor` + `EMomentumKickModel`, apply to lateral velocity, damp it.
3. `UForwardDriftComponent`: compose velocity = forward X + lateral YZ; set CharacterMovement to Flying.
4. `ASpacePlayerController`: create `IA_SpinCounterClockwise`/`IA_SpinClockwise` + IMC in C++, bind A/D (hold/release).
5. `ASpaceAstronautCharacter`: push `EquippedMace` tuning into components; draw debug mace + tether.
6. Decide the "both A and D held" rule (proposal: the most recently pressed wins).

---

## Locked Decisions (Step 1 kickoff — 2026-10-11)

| Topic | Decision | Creator quote |
| :--- | :--- | :--- |
| Step 1 scope | Code-only UE5 C++ scaffold: `.uproject`, 3 modules, Config, Content layout, Docs | — |
| Engine | **Unreal Engine 5.8** (`EngineAssociation: "5.8"`) | *"Unreal Engine 5.8, released in June 2026. is what i want to choose"* |
| Orbit plane | **Full 3D 360° cylindrical**: mace orbits in local YZ plane; release angle steers in Y and Z | — |
| Input | **Two buttons only**: A = spin counter-clockwise, D = spin clockwise. Letting go = stop = release = kick | *"for pc A and D looks good. only two buttons for now to spin the mace, moment of stopping is the moment of release. everything can be controles by this i think."* |
| Platform | **Undecided** (PC vs mobile). Input is designed as 2 abstract actions so both are trivial to map | *"im still not sure which would make more sense for me, a mobile game or a pc game"* |

### Open questions for the Creator
- [ ] **Kick direction:** GDD §3.1.2 says the kick goes *opposite* the mace's travel (`J = -α·m·v_t`). A literal "stop the mace" yanks you *along* its travel. Exposed as `EMomentumKickModel` (default = GDD). Decide after feeling both in Step 2.
- [ ] **Platform:** PC first (recommended for prototyping / Lumen quality) vs mobile.
- [ ] Move `GAME_DESIGN_DOCUMENT.md` into `Docs/` per GDD §4? (not done yet — file left at root)

---

## Architecture Notes (for whoever resumes)

- **Module dependency graph (acyclic, low → high):** `SpaceMentumPhysics` → `SpaceMentumAbilities` → `SpaceMentumCore`.
  Physics depends on nothing in-game; Abilities configures Physics; Core composes everything into the astronaut.
  `SpaceMentumCore` is the **primary game module** (`IMPLEMENT_PRIMARY_GAME_MODULE`).
- **Coordinates:** +X forward drift (GDD's "v_z"), +Y right, +Z up. Orbit angle 0 = +Y, CCW positive seen from behind.
- **File naming:** UE convention, no `U`/`A` prefix in filenames (`MaceOrbitComponent.h` holds `UMaceOrbitComponent`).
- **UFUNCTION getters return by value** (not const-ref) for UHT safety.
- **Abilities module privately depends on `Niagara`** because data assets hold `TSoftObjectPtr<UNiagaraSystem>`.
- Every file header carries `@creator_feel` + `@emotional_intent` (GDD §0 Prime Directive).
- Component ticks are `bCanEverTick = false` until Step 2 implements the logic.
- ⚠️ UE is **not installed** on this machine yet → nothing has been compiled. First compile will happen when 5.8 is installed.

---

## Step 1 Checklist — Project Scaffold

### Root & Config
- [x] `SpaceMentum.uproject` (UE 5.8, 3 runtime modules, EnhancedInput + Niagara)
- [x] `.gitignore` (UE standard)
- [x] `.gitattributes` (Git LFS for `.uasset`/`.umap`/art/audio — git-lfs 3.7.1 present)
- [x] `Config/DefaultEngine.ini` (zero gravity, 120 Hz substepping, enhanced determinism, Lumen, default GameMode)
- [x] `Config/DefaultInput.ini` (Enhanced Input classes, creator A/D quote)
- [x] `Config/DefaultGame.ini` (project identity)

### Source — targets
- [x] `Source/SpaceMentum.Target.cs`
- [x] `Source/SpaceMentumEditor.Target.cs`

### Source — `SpaceMentumPhysics` ✅
- [x] `SpaceMentumPhysics.Build.cs`
- [x] `SpaceMentumPhysicsModule.h/.cpp` (log category `LogSpaceMentumPhysics`)
- [x] `Types/SpaceMentumPhysicsTypes.h` (`EMaceSpinDirection`, `EMomentumKickModel`, `FMaceOrbitTuning`, `FMomentumTransferTuning`, `FForwardDriftTuning`, `FMaceOrbitState`)
- [x] `Components/MaceOrbitComponent.h/.cpp` (shell)
- [x] `Components/MomentumTransferComponent.h/.cpp` (shell)
- [x] `Components/ForwardDriftComponent.h/.cpp` (shell)
- [x] `Solvers/TrajectoryPredictor.h/.cpp` (`ComputeTangentDirection` implemented)

### Source — `SpaceMentumAbilities` ✅
- [x] `SpaceMentumAbilities.Build.cs`
- [x] `SpaceMentumAbilitiesModule.h/.cpp`
- [x] `Data/MaceDefinitionDataAsset.h`
- [x] `Data/AbilityDataAsset.h`
- [x] `Data/ObstacleArchetypeDataAsset.h` (+ `EObstacleBehaviour`)
- [x] `Components/AbilitySystemBridgeComponent.h/.cpp`

### Source — `SpaceMentumCore` (primary module) ✅
- [x] `SpaceMentumCore.Build.cs` (deps: EnhancedInput, InputCore, Physics, Abilities)
- [x] `SpaceMentumCoreModule.h/.cpp` (`IMPLEMENT_PRIMARY_GAME_MODULE`, log `LogSpaceMentum`)
- [x] `Types/SpaceMentumCoreTypes.h` (`ESpaceRunPhase`)
- [x] `GameModes/SpaceGameModeBase.h/.cpp` (wires pawn/controller/state)
- [x] `GameModes/SpaceGameState.h/.cpp`
- [x] `Player/SpacePlayerController.h/.cpp` (input contract documented)
- [x] `Player/SpaceAstronautCharacter.h/.cpp` (composes Drift/Orbit/Transfer/AbilityBridge + SpringArm/Camera rig)

### Content & Docs ✅
- [x] `Content/**` folder layout from GDD §4 with `.gitkeep`
- [x] `Docs/TECHNICAL_SPECIFICATIONS.md` (dependency graph, coordinates, input spec, coding standard)
- [x] `README.md` (setup: install UE 5.8 → right-click `.uproject` → Generate VS files → build)
- [x] Update GDD §5 checklist with locked decisions + new creator quotes
- [x] Git commit: `feat: Step 1 — UE 5.8 modular C++ project scaffold`

---

## Roadmap (proposed, after Step 1)

| Step | Goal |
| :--- | :--- |
| 2 | Core feel prototype in C++: orbit integration, release → kick, forward drift, A/D input created in code, chase camera |
| 3 | Greybox map `L_Prototype_Greybox` + debug overlay + trajectory gizmo (first editor work) |
| 4 | Obstacles & spawner, collision/CCD, fail state |
| 5 | VFX (plasma trail, space dust), audio (spin-pitch Metasound), FOV shifter |

---

## Session Log

| Date | Session | Work done |
| :--- | :--- | :--- |
| 2026-10-11 | Step 1 kickoff | Decisions locked; root, Config, targets, Physics + Abilities + Core modules, Content layout, Docs, README written; committed. Not yet compiled (UE not installed). |
