# Plan: One-Button Push-Off & Honest Momentum (Core Feel Prototype)

> **Creator's feedback (verbatim):**
> *"the controls for pushing on the rocks needs to be re thinked. current one i dont know how to."*
> *"the the space ship shouldent move the center of mass and stay there after a regular spin. that sodent happen in frictionless space"*
>
> **Meaning:** (1) Pushing on rocks must be learnable in one sentence, with no mouse aiming or lock-on step. (2) Spinning the mace is an *internal* force. In frictionless space it can shift the body around the centre of mass (CoM), but it can never move the CoM's path. Only touching something outside the system (a rock) can change where you are going. This is GDD Rule 1 being enforced for real. It **supersedes** the original concept line *"by spinning and stopping the spin in a particular direction, one can stir their direction"*. Steering now comes from pushing off and striking rocks.

**Target:** `Tools/CoreFeelPrototype/` (`app.js`, `index.html`, `style.css`). `dist/` is regenerated at the end. The UE C++ port (`UMomentumTransferComponent` release kick and friends) is a **follow-up, out of scope here**. Log it as a TODO in `PROGRESS.md`.

---

## 0. Ground rules

1. Work only in the files listed above, plus the docs in §6. Do not touch `Source/` or `Tools/Build/`, which hold uncommitted C++ work.
2. **Commit after each phase:** `feat(prototype-physics): phase N — <name>`. Stage only `Tools/CoreFeelPrototype/` sources and the docs you edit.
3. After each phase, run `node -c Tools/CoreFeelPrototype/app.js`, then `npm start` and play in a browser.
4. Keep the minimal-HUD rules from `Docs/PLAN_MinimalGameHUD.md`: no trajectory prediction lines, and no new permanent HUD clutter.
5. Tick the tracker in §7 with a one-line note per phase.

---

## 1. Diagnosis (what's wrong today)

| Problem | Where | Why it's wrong |
| :--- | :--- | :--- |
| Releasing A/D after a spin adds sideways velocity | `executeMomentumKick()` (`app.js` ~L876) adds to `State.LateralVelocity`, which `physicsTick` integrates into `CoMPosition` (~L1283) | An internal force is moving the CoM. That is impossible in frictionless space. |
| That velocity then fades away | Lateral damping in `physicsTick` (~L1271, `LateralDampingPerSec`) | Fake friction. In a vacuum, a velocity change you earn should stay until something else changes it. |
| Kinetic push and overpower also write `LateralVelocity` | `releaseAndDeployKineticPush()` (~L2756, ~L2778) | The same fake friction applies, plus a made-up `* 0.65` on x and an `α` fudge. |
| Colossal crash speed ignores the player's own velocity | `updateColossalAsteroidBelt()` (~L2553) uses `ast.velocity.length()` | It should use the relative velocity. |
| Push controls take 4 steps (mouse aim → click/F lock → hold SPACE → release) and act at up to 300 m range | Colossal mode | They can't be discovered, and pushing a rock 50 m away makes no physical sense. |

What is already correct and must stay: the body is offset from the CoM by `-(m_mace/M)·r(θ)` and the mace sits at `+(m_astro/M)·r(θ)` (`physicsTick` step 4, `evaluateContinuousCollisions`), along with the counter-rotation. So after a spin stops, the **body stays a little off-centre** (up to ~36 cm, because the mace is held out at that angle) while the **CoM path is unchanged**. That is the honest "dodge" and "physics debt".

---

## 2. Honest momentum (Phase 1)

1. **Delete the release kick.** In `evaluateActiveSpin()`, letting go of A/D just stops the spin: `AngularVelocityRadPerSec = 0`, and the mace stays at its current angle. Replace `executeMomentumKick()` with a small `stopSpin()` that plays a soft mechanical "clunk" (reuse `playKickSound` at lower gain) and adds **no** velocity, flash or FOV punch.
2. **Remove `State.LateralVelocity` entirely.** Every place that does `CoMVelocity.y + LateralVelocity.y` (about 15 sites: grep `LateralVelocity`) becomes just `CoMVelocity.y`/`.z`. Drop the damping block in `physicsTick`.
3. **Remove the kick-model machinery:** `Tuning.KickModel`, `MinAngularSpeedForKickDegPerSec`, `LateralDampingPerSec`, `MaxLateralSpeedCmPerSec`, `toggleKickModel()`, the dev `M` hotkey, the dev "KICK MODEL" card, the `slider-damping` slider, and the `kickArrow` gizmo. Keep `ImpulseEfficiencyAlpha` only if something still uses it after Phase 2; otherwise remove it and its slider too. Update the dev-drawer RESET handler.
4. All external velocity changes write **`CoMVelocity`** and persist (no decay). That covers strikes (already do), pushes (Phase 2) and bumps (Phase 3).
5. **Fix Colossal crash speed:** `relSpeed = |ast.velocity − CoMVelocity|`.
6. Engine hum, dutch-roll camera bank and the speed readout now read `CoMVelocity` only.

✔ **Check:** start a corridor run and spin A/D in open space for a few seconds, then release. `CoMVelocity.y/z` (dev overlay) stay exactly unchanged and the heading arrow doesn't move. The astronaut visibly sits off-centre while the mace is held out. Spin the mace back around and the body returns.

---

## 3. One-button push-off (Phase 2 — corridor)

### 3.1 The rule the player learns

> **Hold SPACE to wind up. When a rock is close enough it lights up. Let go to shove off it, and you fly away from the rock you pushed.**

It uses the same grammar as the launch (hold SPACE, release), so the first thing the player does in every run teaches the core move. A/D keep spinning the mace while SPACE is held. The two inputs are independent.

### 3.2 Mechanics

New tuning (in `Tuning`, mirrored as dev sliders):

| Knob | Start value | Meaning |
| :--- | :--- | :--- |
| `PushReachCm` | 650 | Max distance from the astronaut body to a rock's **surface** for a push to connect. |
| `PushChargeRatePerSec` | 0.9 | ~1.1 s to full charge. |
| `MinPushCharge` | 0.15 | A tap still gives a small shove. |
| `MaxPushDeltaVCmPerSec` | 1400 | Player Δv at full charge against an immovable rock. |
| `PushReleaseBufferMs` | 150 | If you release up to this long *before* a rock enters reach, the push fires when it does. |

- **Anchor selection:** `findPushAnchor()` returns the obstacle whose surface is nearest the body within `PushReachCm` (corridor: `denseObstacles`; Colossal: `colossalIncomingAsteroids`), or `null`.
- **Charging:** while SPACE is held in `Running`, `State.PushCharge` rises (reuse/rename `KineticPushCharge`). The speed-dial arc shows it. When not charging, the arc shows spin energy, dimmed.
- **Release:**
  - If an anchor is in reach, fire `executePushOff(anchor, charge)`.
  - If not, arm the buffer for `PushReleaseBufferMs`. If an anchor enters reach in that window, fire; otherwise **whiff**: the charge is lost and a short airy sound plays, with no toast.
- **`executePushOff`** (Newton's third law, no fudge factors):
  - Direction `n̂` = unit vector from the anchor centre to the astronaut body, so you go away from the rock. This includes its x component: pushing off a rock ahead of you **slows** you. That's honest and intended, and Doom detection will catch a stall.
  - Impulse `J = charge × MaxPushDeltaVCmPerSec × m_player`.
  - Player: `CoMVelocity += n̂ × J / m_player`.
  - Rock: if it is movable (Colossal asteroids have `massKg`), `velocity −= n̂ × J / m_rock`. Corridor obstacles are static, i.e. infinite mass.
  - Feedback: the existing push sound and sparks at the contact point, camera trauma ∝ charge, toast `PUSH` (gold). Count it in `StrikeCount` for scoring as today.
  - Visual: snap the mace's `OrbitAngleRad` toward the rock's lateral direction over ~100 ms, so the arm visibly reaches out and touches it. This is an internal move, so it's fine physically.
- **In-reach highlight:** the current anchor gets an emissive rim (bump its material's `emissiveIntensity`; restore when it leaves reach). Draw a very faint ring around the astronaut at `PushReachCm` in the lateral (Y-Z) plane, visible only while SPACE is held. These are range cues, not trajectory prediction, so Rule 4 still holds.
- **Launch stays as is** (A/D aims, hold/release SPACE). It is the same verb, with the starting rock always in reach.
- **Mace strikes stay:** a spinning mace that hits a rock still deflects you (`executeExternalStrike`). That's a physical, external force. Push-off is the deliberate, controlled version.
- **Doom lookahead:** in `evaluateBackgroundDoomLookahead`, reach becomes `PushReachCm` (an obstacle is a usable anchor if it will pass within push reach). Remove the `ReachToleranceCm`-based reach formula.

✔ **Check:** fly sector 1 using only SPACE pushes (no spinning) and clear at least 3 gates. Taps give small nudges and full charges give big ones. Releasing early with nothing near does nothing; releasing ~0.1 s early still connects.

---

## 4. Colossal Belt uses the same push (Phase 3)

1. **Remove** mouse aiming and lock-on: `lockKineticTarget`, `unlockKineticTarget`, `updateKineticTargetingAndAim`, `onMouseMoveAim`, `CandidateTarget`/`KineticTarget`/`KineticLockStatus`, `F` and left-click handlers, `#aim-crosshair-3d`, `#kinetic-target-reticle`, `kineticFluxLine`, `releaseAndDeployKineticPush`. Remove their CSS and markup too.
2. Colossal uses `findPushAnchor` / `executePushOff` from Phase 2 unchanged. The in-reach rim highlight makes it obvious which giant rock you'll push. Because the rocks are huge (`massKg` in tonnes), they barely move and **you** get flung. That's correct physics.
3. **Overpower becomes emergent:** delete the special "< 70% charge inside 18 m" rule and `OverpowerRatio`. Instead, when a rock touches the body, compute the closing speed `v_c = −(v_rock − v_player)·n̂`:
   - `v_c > ColossalCrashSpeedCmPerSec` (start: 900) → crash, as today.
   - Otherwise → **bump**: set the player's velocity component along `n̂` to match the rock's, then separate them. Toast `OVERPOWERED!` (rose), with the existing overpower sound and flash.
   - So a weak or late push that doesn't kill enough closing speed means *the rock pushes you*. That keeps the Creator's "it will push you" fantasy without a hidden rule.
4. Keep the `TOO CLOSE` cue as a property of the highlighted anchor: rose rim instead of cyan when the rock's closing speed would exceed what your current charge can cancel. It is a readable warning, not a prediction line.
5. Remove `CriticalDangerDistanceCm`/`OverpowerRatio` sliders. Add `ColossalCrashSpeedCmPerSec`.

✔ **Check:** in Colossal mode, push off an incoming rock with full charge and fly clear. Push weakly at the last moment and get bumped (`OVERPOWERED!`). Do nothing and crash.

---

## 5. Teaching it (Phase 4)

- **Launch prompt** (existing): keep it as is.
- **First-flight hint** (reuse the `#hud-hints` / first-3-runs logic): `HOLD SPACE · LET GO NEAR A ROCK TO PUSH OFF` and `A / D swing the mace`. In Colossal mode, the same text.
- **Contextual cue:** the first time an anchor enters reach in a run, show a small `SPACE` key glyph next to the rock (screen-projected, fades after 1 s). On the player's first 3 runs only.
- **How to Play** screen: rewrite to 3 rows:
  1. *Hold SPACE, let go:* push off rocks (and off the launch rock) to change course.
  2. *A / D:* swing the mace. It shifts your body and smacks rocks, but on its own it can't change where you're going.
  3. *You can't brake:* run out of rocks within reach and you're lost to the void.

  Remove all mouse/F/lock text.

---

## 6. Docs

- `GAME_DESIGN_DOCUMENT.md`: in Rule 1 add a short "Creator update" with both quotes above and state plainly: *releasing a spin never changes the CoM path; steering = push-off (SPACE) and mace strikes.* Mark the original "spinning and stopping the spin… stir their direction" line as superseded (keep it as a quote, since the GDD requires preserving the Creator's words).
- `README.md` controls table: A/D = swing mace (shifts body, smacks rocks); SPACE hold/release = push off rocks; Esc = pause; R = restart.
- `PROGRESS.md`: session row plus a checklist, and a TODO: *port honest momentum + push-off to UE C++ (`UMomentumTransferComponent`, `KineticPushSolver`)*.
- Run `npm run package:web` and bump `?v=` in `index.html`.

---

## 7. Phases & tracker

One Sonnet session should fit (this is much smaller than the HUD plan). If context gets tight, stop after a committed phase.

- [x] Phase 1 — Honest momentum (§2) — notes: LateralVelocity, kick model, damping, kickArrow, M hotkey removed; spin release is now stopSpin() (no Δv); Colossal crash uses relative speed. `slider-alpha`/`ImpulseEfficiencyAlpha` kept until Phase 3 (old Colossal push still uses it). Not play-tested in browser (node -c only).
- [x] Phase 2 — Push-off in corridor (§3) — notes: findPushAnchor/executePushOff/release buffer/whiff/rim highlight/reach ring/mace snap + 5 dev sliders; doom reach = PushReachCm; ReachToleranceCm and KineticChargeRatePerSec removed. Corridor SPACE-in-Running pushes; Colossal still old lock-on path (Phase 3). Not play-tested in browser (node -c only).
- [x] Phase 3 — Colossal uses push-off (§4) — notes: lock-on/mouse-aim/F/click/flux line/reticle/crosshair + CSS/markup removed; Colossal reuses findPushAnchor/executePushOff (rock recoils by massKg); overpower is emergent (closing speed vs ColossalCrashSpeedCmPerSec: crash or bump); rose rim = TOO CLOSE; alpha/push-force/crit-dist/overpower sliders removed, crash-speed slider added. Not play-tested in browser (node -c only).
- [x] Phase 4 — Teaching + docs + dist (§5, §6) — notes: two-line first-flight hint, first-3-runs SPACE glyph cue, How to Play = 3 rows, GDD/README/PROGRESS updated (UE port logged as TODO), `?v=9.1`, package 0.9.0, dist/ rebuilt. Not play-tested in browser (node -c only).

## 8. Acceptance

- [ ] Spinning and releasing in open space never changes `CoMVelocity` (verify in the dev overlay).
- [ ] No velocity anywhere decays on its own.
- [ ] Whole push control = hold SPACE, release near a glowing rock. No mouse, no lock key.
- [ ] Corridor sector 1 is completable using push-offs.
- [ ] Colossal: push to escape, weak push → bump, no push → crash.
- [ ] How to Play, hints, README and GDD all describe the same controls.
- [ ] `node -c app.js` passes, no console errors, `dist/` rebuilt.
