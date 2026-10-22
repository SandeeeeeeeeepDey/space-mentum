# Plan: "Could I Have Made It?" Crash Analysis & Ghost Replay (Core Feel Prototype)

> **Creator's request (verbatim):** *"when i lose, by getting hit, it should show me what menuvering i could have done if any to have avoidd the asteroid … or obstecle"*
>
> **Meaning:** A crash should teach, not just punish. After any crash into an asteroid, debris or other obstacle, the game rewinds the last few seconds, tries the moves you *could* have made, and tells you in one plain sentence whether there was a way out, what it was, and when. Then it lets you watch it. If there was truly no way out, it says so honestly and points at the earlier decision that sealed it.

**Depends on:** `PLAN_PushOffAndHonestMomentum.md` (done) and `PLAN_ThreeLevels.md`. Implement this **after** the levels plan, since it uses level rocks, the portal and the result screens from that plan. It must also work for Colossal Belt rocks (dev-only mode), which move.

**Target:** `Tools/CoreFeelPrototype/` only. `dist/` is regenerated at the end.

---

## 0. Ground rules

1. **Never commit anything under `Source/` or `Tools/Build/`** (the user's staged C++ work). Check `git diff --cached --name-only` before each commit, and commit with explicit paths: `git commit -m "..." -- <paths>`.
2. **Commit after each phase:** `feat(prototype-crash-hint): phase N — <name>`. Tick the §8 tracker with a one-line note.
3. **Never change gameplay physics.** The analysis must run the *same* physics code as the real game, so its answer is trustworthy. Do not write a second, simplified physics model.
4. The analysis only appears **after the run has ended** (on the crash results screen). Nothing is shown during play, so GDD Rule 4 ("Blind Math", no in-play prediction) still holds. Add that note to the GDD (§7).
5. After each phase, run `node -c` on all JS and play: crash on purpose in each level.

---

## 1. What the player sees

On the crash card (`#crash-modal`), under the existing stats, add a block:

```
 IMPACT
 Hit a rock at 104 km/h
 Distance 182 m · Score 1,240 · Best 2,100

 ── COULD YOU HAVE MADE IT? ──────────────
 Yes. Push off the rock you hit when it lit up,
 0.6 s before impact, with 45% power or more.
                         [ ▶ WATCH ]   (W)
 RETRY [R]     MENU
```

- While the analysis runs (it starts at the moment of the crash, during the existing 600 ms reveal delay), the block reads `Checking…`. It must finish within ~1 s of the card appearing. If it doesn't, show `Couldn't check this one` and move on. Never block Retry.
- **One sentence answer**, using one of these templates (pick the first that applies, §4.4):

| Case | Text |
| :--- | :--- |
| Push off the obstacle you hit | `Yes. Push off the {thing} you hit when it lit up, {t} s before impact, with {cMin}% power or more.` (or `between {cMin}% and {cMax}%`) |
| Push off a different obstacle | `Yes. Push off the {thing} on your {side}, {t} s before impact, at about {c}% power.` |
| Your own push caused the crash | `Your push ({cActual}%) sent you into it. About {c}% would have cleared it.` (or `Not pushing would have cleared it.`) |
| Launch was the problem (crash within the analysis window of launch) | `Launch at {aim}° instead of {aimActual}° to miss it.` (add `with {c}% power` if the charge also has to change) |
| Nothing worked | `No escape in the last {N} s. Your line was set before that — try a different route earlier.` |

  `{thing}` = `rock` / `debris` / the obstacle's `name`, lower-cased. `{side}` = left/right/above/below **as seen on screen** (check the y/z → screen mapping against the chase camera; don't guess). Times are rounded to 0.1 s and power to 5%.
- **▶ WATCH (or `W`)** plays the ghost replay (§5), then returns to the same card. Hidden when the answer is "nothing worked".
- **Settings:** add a toggle, `Crash tips: ON / OFF` (default ON, persisted with the other settings). When OFF, the block and the analysis are skipped entirely.

Do the same for Colossal-mode crashes and "bump-then-crash" cases. Void / missed-portal / too-slow results are **out of scope** for this plan.

---

## 2. Make the simulation replayable (Phase 1)

The analysis works by rewinding to a saved moment and re-running the real `physicsTick` with a different input. That needs four things:

1. **Sim clock.** Every `performance.now()` used by gameplay logic (e.g. `LastStrikeTime`, `PushBuffer.until`, crash grace, push cue timing) switches to `State.SimTimeMs`, advanced inside `physicsTick` by `dt * 1000`. UI-only timers (toasts, flashes, DOM fades) keep using `performance.now()`.
2. **Seeded randomness.** Any `Math.random()` that affects physics or spawns (Colossal spawner, anything in bump/strike paths) uses a seeded RNG whose seed lives in `State` (e.g. `State.RngSeed`, a mulberry32-style generator). Cosmetic randomness (sparks, shake) can stay `Math.random()`.
3. **Silent mode.** `const Sim = { silent: false }`. When `Sim.silent` is true:
   - `getAudioContext()` returns `null` (all sound functions already early-return on that; verify each one).
   - `showToast`, the screen-flash functions, `spawnKineticSparks`, the mace-snap visuals and all DOM/mesh-highlight writes return early.
   - `triggerKineticCrash`, `triggerVoidDoom`, missed-portal and level-complete **record an outcome** into `Sim.outcome = { type, simTimeMs, obstacle }` and return, without changing screens or modals.
   - Grep `document.` and `scene.` inside every function reachable from `physicsTick`, and guard each one.
4. **Snapshots + history.**
   - `takeSnapshot()` deep-copies everything `physicsTick` reads or writes: the relevant `State` fields (CoM position/velocity, orbit angle, angular velocity, held spin, push charge/buffer/anchor *reference by index*, internal displacement, counter-rotation, sim clock, RNG seed, run phase, launch aim/charge, strike timing), plus the dynamic obstacle state (Colossal asteroid positions/velocities/destroyed flags; the spawn list). Static level rocks don't need copying.
   - `restoreSnapshot(snap)` writes it back. Obstacles are referenced by **stable index/id**, never by object identity held across restores.
   - **History recorder:** every 1/30 s of sim time during `Running` (and once at launch), push `{ snap, bodyPos, inputs }` into a ring buffer covering the last **6 s**. `inputs` = held spin direction, whether SPACE is held, and the current charge. Also log discrete events with sim time: `pushStart`, `pushRelease{charge, anchorId}`, `launch{aim, charge}`.
5. **Determinism self-test (dev overlay button `REPLAY CHECK`):** take the snapshot from ~3 s ago, re-run it silently with the recorded inputs at the same `dt` (fixed substep `1/120 s`), and confirm the crash happens at the same sim time (±1 tick) and position (±1 cm). Print PASS/FAIL to the console. **This must pass before Phase 2.**

Silent re-runs always use a fixed `dt = 1/120 s`. The real game also feeds `physicsTick` with 4 substeps per frame; record the actual substep `dt`s in history so the self-test can replay them exactly, or switch the real loop to a fixed-step accumulator at 120 Hz (preferred, and it doesn't change the feel). Note which you chose in the tracker.

✔ **Check:** `REPLAY CHECK` passes on 5 different crashes, including one in Colossal mode.

---

## 3. Candidate moves (Phase 2)

Search only moves the player could physically have made with the controls they have. Each candidate = **"play exactly what you did up to time t, then do this one thing, then do nothing"** (spin stops, no more pushes).

| Family | Generated from | Parameters tried |
| :--- | :--- | :--- |
| **Push off X at t** | Every history sample in the window where `findPushAnchor()` (evaluated on that snapshot) returns an obstacle; also try the *second*-nearest in reach if there is one | charge ∈ {0.15, 0.3, 0.45, 0.6, 0.75, 0.9, 1.0} |
| **Change your push** | Each real `pushRelease` event in the window | same anchor and moment with the other charges, plus "don't push" |
| **Change the launch** | Only if the launch is inside the window | aim −35…+35° in 2.5° steps × charge {0.4, 0.7, 1.0} (A/D limits apply) |

- **Window:** the last 6 s before impact (the whole history).
- **Charge feasibility:** charge `c` needs SPACE held for `c / PushChargeRatePerSec` seconds. That's always possible by starting earlier, so don't filter on it. Do mention it in the sentence when `c ≥ 0.6`: append `(start holding SPACE about {s} s earlier)`.
- **Injecting the move:** restore the snapshot at t, then call the real `executePushOff(anchorById, c)` (or set the launch aim/charge and call the real launch function) **inside silent mode**, then step `physicsTick` with no input.

## 4. Evaluating and choosing (Phase 2, continued)

### 4.1 Success test
Simulate each candidate forward, silently, until **2 s of sim time past the original impact time** (or until it crashes or finishes the level).
- **Escape** = no crash in that span.
- **Good escape** = escape *and*, at the end, the existing doom lookahead says safe (still on course for the portal or another anchor). Prefer good escapes.

### 4.2 Budget
Run the search **asynchronously in slices of ≤ 8 ms per frame** (`requestAnimationFrame` or `setTimeout(0)` loop). Before each candidate, restore the snapshot; after the search, restore the *crash-time* snapshot and set `Sim.silent = false`. Abort after 1.5 s of wall time and use the best result found so far. With the three teaching levels (< 10 rocks) the full search should take well under a second; log the time in the dev console.

### 4.3 Grouping the results
Group successful candidates by `(family, anchorId)`. For each group, compute:
- the **time window** (earliest to latest successful t, as "s before impact"),
- the **charge range** that worked at the *latest* successful t,
- the **margin**: closest approach to any obstacle surface during the re-sim (larger is more forgiving).

### 4.4 Picking the one sentence (first match wins)
1. A real push in the window led to the crash and a "change your push" candidate succeeded → *your push caused it* template (prefer the charge nearest to what they used, or "not pushing").
2. A "push off the obstacle you hit" group succeeded → *push off the one you hit*. Use the latest successful t, since that's the most recognisable moment: "when it lit up".
3. Another anchor's group succeeded → *push off a different obstacle*. Choose the group with the largest margin.
4. A launch candidate succeeded → *launch* template. Choose the aim nearest to what they used.
5. Otherwise → *nothing worked*.

Within each rule, prefer good escapes over plain escapes. Keep the chosen candidate's parameters for the replay.

✔ **Check:** for 5 deliberate crashes per level, (a) the sentence reads naturally, and (b) **actually doing what it says in a real retry avoids the obstacle**. This is the test that matters. Record any failure cases in the tracker.

---

## 5. Ghost replay — ▶ WATCH (Phase 3)

1. **Record both paths** at 30 Hz: the *actual* path from history, and the *suggested* path by re-running the chosen candidate silently (keep the body positions it produces). Also record the obstacle positions for moving rocks.
2. **Playback** (results card hidden, game input disabled except `Esc`/`Space` to skip):
   - Camera: the normal chase camera following the **suggested** ghost, starting ~1.5 s before the action moment; play at **0.5× speed** up to 0.5 s after the action, then 1×, ending 1 s after the original impact time.
   - **Red ghost** (translucent sphere and fading trail) = what you did; it ends in an impact flash at the crash point.
   - **Cyan ghost** (translucent astronaut clone or glowing sphere and trail) = what would have worked.
   - At the action moment: pulse the anchor's rim (as in real play) and show a short label anchored to it, e.g. `PUSH 60%` or `LAUNCH +12°`.
   - Moving rocks replay from recorded positions. Static rocks just render.
3. On end or skip: remove ghosts and trails, restore the crash-time scene and camera, and show the crash card again with focus on **RETRY**.

These lines appear only in a post-run replay, never during play, so Rule 4 is respected (§0.4).

✔ **Check:** the replay matches the sentence, ghosts don't stay behind after Retry, and Retry/Menu still work after watching.

---

## 6. UI wiring (Phase 4)

- `index.html`: add the block to `#crash-modal` (`#crash-tip-text`, `#btn-crash-watch` as a `.menu-item[data-action=watch]`, placed after Retry in menu order so Enter still = Retry).
- `activateMenuAction('watch')` → start the replay. The `W` key does the same while results are shown.
- `#screen-settings`: the `Crash tips` toggle; persist in `SPACEMENTUM_SETTINGS_V1`.
- Styles: the block uses the existing results-card typography; the answer line is ~16px, white 90%; `Yes.`/`No escape` coloured cyan/rose respectively. Reduced motion: replay at 1×, no slow-mo.

## 7. Docs & ship (Phase 5)

- `GAME_DESIGN_DOCUMENT.md`: add a "Crash Debrief" subsection under the failure states with the Creator's quote above. State that the analysis runs the real physics, only after the run ends, and that Rule 4 still applies during play.
- `PROGRESS.md`: a session row plus a checklist.
- `README.md`: one line under controls: `W` on the crash screen = watch what would have worked.
- Run `npm run package:web` and bump `?v=`.

---

## 8. Phases & tracker

One Sonnet session might fit. Phase 1 is the riskiest, so if context gets tight, stop after a committed phase.

- [x] Phase 1: Replayable sim (sim clock, seeded RNG, silent mode, snapshots, history, REPLAY CHECK) (§2). Notes: chose the fixed 120 Hz accumulator (`simStep` = `physicsTick` + Colossal belt update, so the belt now steps at 120 Hz too). Input goes through `applyInputEvent` and is logged with sim time; REPLAY CHECK (dev overlay button, or `__sm.runReplayCheck()` with `?dev=1`) passed on 7 crashes (3 corridor sectors 2-3, 4 Colossal) at 0 ms / 0 cm drift. Levels plan not implemented yet, so this runs on the current sectors.
- [x] Phase 2: Candidate search, evaluation and sentence (§3, §4). Notes: search runs async in 8 ms slices (dev: `__sm.analyzeCrashSync()`), 100-900 ms CPU for 60-700 candidates; crash card shows the sentence in a basic `#crash-tip` block (watch button/toggle come in phase 4). "Do what it says" test (input-level retry of the suggestion, 3 corridor sectors x ~26 crashes, 50 Colossal): corridor 26/26, Colossal 44/46 (the 2 failures are `ownPush` answers at a knife-edge charge). Deviations: (1) only the nearest rock in reach is tried as a push anchor, since the one-button push always picks the nearest; (2) a charge is skipped when holding SPACE that long would start before the run began or run through another real release; (3) after a candidate the void verdict is ignored so the rock outcome is still judged (`good` = lookahead says safe at the end); (4) for push-off answers the time is the earliest moment the advice also works over the next 2 samples (rock just lit up), not the latest, since the latest is ~0.0 s. Launch/other-rock/none templates were exercised with constructed crashes (corridor wall, Colossal killer rock + side rock); "5 crashes per level" was done per existing sector since the levels plan isn't implemented.
- [x] Phase 3: Ghost replay (§5). Notes: `W` (or `__sm.startCrashReplay()`) plays it; red = recorded path, cyan = history up to the action then a silent re-run of the chosen move, both at 30 Hz on the sim clock, moving Colossal rocks replay from their recorded tracks and are put back afterwards. Chase camera follows cyan (camera rig reads `Replay.camPos` while active), 0.5x until 0.5 s after the action, then 1x, ends 1 s after impact; Esc/Space/Enter skips; reduced-motion = 1x. Sim is frozen while it plays. Checked in Edge (headless) on a corridor and a Colossal crash: label + rim pulse + legend show, ghosts/trails/flash removed on end and on skip, card returns with RETRY selected, Retry works. The sentence's power range and the replay's PUSH label use the middle of the working range.
- [x] Phase 4: UI wiring and settings toggle (§6). Notes: crash card block (heading, answer line with cyan `Yes.` / rose `No escape`), `#btn-crash-watch` (`data-action=watch`) after RETRY so Enter stays RETRY, hidden unless a move exists or tips are off; `W` and Esc/Space handled in `handleMenuKey`. Settings > CRASH TIPS ON/OFF (click, Enter or left/right), saved as `crashTips` in `SPACEMENTUM_SETTINGS_V1`. OFF = block, watch button and analysis all skipped (history is still recorded: it is cheap and the dev REPLAY CHECK needs it). Verified in Edge: toggle + persistence, card with tip/watch, OFF card, W/Esc/Retry paths, no console errors.
- [x] Phase 5: Docs and dist (§7). Notes: GDD "Crash Debrief" subsection (with the Creator's quote and the Rule 4 note), PROGRESS.md row + checklist, README `W` line, `?v=9.2`, package 0.9.1, `npm run package:web` -> `dist/` identical to source.

## 9. Acceptance

- [x] Every crash into any obstacle shows a "Could you have made it?" answer within ~1 s, or a graceful `Couldn't check this one`. (60-700 candidates take 100-900 ms CPU, sliced at 8 ms/frame; ~0.8 s wall in a slow software-GL headless browser; over the 1.5 s budget with nothing found it says `Couldn't check this one`.)
- [x] Following the suggested move in a real retry avoids that obstacle in at least 9 out of 10 tested crashes. (Scripted input-level retries: corridor 26/26, Colossal 44/46; not tested with a human at the keyboard.)
- [x] "No escape" is only shown when the search really found nothing. (Charges that would need SPACE held from before the run began, or through another real release, are not counted as escapes.)
- [x] The replay shows red (actual) and cyan (suggested) ghosts with the action label, and cleans up fully.
- [x] Gameplay is unchanged when tips are OFF, and normal play never shows hints or prediction lines. (Only change to the feel: the sim now steps at a fixed 120 Hz instead of 4 sub-steps per frame, and the Colossal belt steps at 120 Hz too.)
- [x] `REPLAY CHECK` passes; no console errors; nothing under `Source/` or `Tools/Build/` committed; `dist/` rebuilt.
