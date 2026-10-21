# Plan: Minimal Game HUD & Playable Game Flow (Core Feel Prototype)

> **Creator's request (verbatim):** *"make the game screen minimalistic with only the information needed. the interface doesnt look vary playagle and game like right now."*
>
> **Meaning:** The screen currently reads like an engineering dashboard (12 metric cards, two glass panels, a footer of key pills, a slider drawer, GDD quotes inside death screens). The Creator wants it to feel like a *game*: the space, the astronaut and the mace own the screen, and the HUD shows only what a player needs to act in the next second.

**Target:** `Tools/CoreFeelPrototype/` (the playable web build: `index.html`, `style.css`, `app.js`). `dist/` is regenerated, never hand-edited. The UE C++ code is **out of scope** for this plan.

---

## 0. Ground rules for the implementer

1. **Do not discard uncommitted work.** `app.js`, `index.html`, `style.css` (and their `dist/` copies) have large uncommitted changes. Never run `git checkout`/`git restore`/`git stash` on them. Edit in place.
2. **Do not change physics or gameplay math.** `physicsTick`, collision, doom lookahead, kick, push and scoring formulas stay exactly as they are. This is a presentation + flow change only.
3. **GDD Rule 4 must still hold.** The four mandatory instruments stay, just simplified: (1) directional arrow (the existing 3D arrow), (2) radar/mini-map, (3) speedometer, (4) push power meter. **No** predictive trajectory lines anywhere ("Blind Math").
4. **Nothing gets deleted that a developer needs.** All the current telemetry and tuning sliders move into a hidden **Dev Overlay** (§4). Keep their element IDs unchanged so existing `updateHUD()` / `setupTuningDrawer()` code keeps working.
5. After every phase: `node -c Tools/CoreFeelPrototype/app.js`, then run `npm start` in `Tools/CoreFeelPrototype` and play it in a browser at 1920×1080 **and** 1280×720. At the end, run `npm run package:web` to sync `dist/`, and bump the `?v=8.0` cache-bust in `index.html` to `?v=9.0`.
6. Comment style: match the file (section banners like `// --- ... ---`). Per the GDD rule, note the Creator's quote above in the header comment of the new HUD section in `app.js` and `style.css`.

---

## 1. What the player sees (target layout)

### 1.1 In flight — Corridor mode

```
┌────────────────────────────────────────────────────────────────────┐
│               ▬▬▬▬▬▬▬▬▬●━━━━━━━━|━━━━|━━━━◎   412 / 1200 m      12,450│
│                (progress bar: gate ticks, milestones, portal)   x3  │
│                                                                    │
│                                                                    │
│                         +250 CLEAN  (toast, floats up & fades)     │
│                                                                    │
│                          [ astronaut + mace ]                      │
│                                                                    │
│   ╭─────╮                                                          │
│   │radar│                  ╭── 112 ──╮                            │
│   ╰─────╯                  ╰─ KM/H ──╯  (spin/power arc around it) │
└────────────────────────────────────────────────────────────────────┘
```

| Zone | Shows | Notes |
| :--- | :--- | :--- |
| Top-center | **Progress bar** to the jump portal: filled to `CoMPosition.x / corridorLength`, small ticks for slalom gates (green when cleared, red when missed), diamonds for milestones, portal icon at the end. Small text `412 / 1200 m`. | Replaces "GREYBOX GAUNTLET" card and gate counter. ~40% of screen width, 4px tall. |
| Top-right | **Score** (large, tabular numbers) and a **streak chip** `x3` that only appears when `CleanEvasionStreak > 1`. | Replaces score card. Animate a short scale-pop when score increases. No projected grade in-run. |
| Bottom-left | **Radar** (existing `drawTacticalRadar`), circular, ~140px, no title/legend/contact counter, no panel box — just a faint ring on a translucent dark disc. | Instrument #2. |
| Bottom-center | **Speed dial**: big km/h number + small `KM/H` label. A thin circular **arc** around it is the **power meter**: launch charge during `LaunchPushOff`, spin energy (the existing `Ek / 4500` ratio) in flight, push charge in Colossal mode. Arc turns gold when > 85%. | Instruments #3 and #4 merged into one cluster. Drop cm/s, vx/v_lat breakdown, Joules, rev/s text from the main HUD (they live in the Dev Overlay). |
| 3D scene | Existing directional vector arrow. | Instrument #1. Keep it visible in flight (verify it is). |
| Center (contextual only) | Toasts, proximity warning, doom warning (§1.4). | Nothing permanent in the center except the small existing crosshair dot (drop the ring). |

**Removed from the main screen:** both top glass panels, the footer key-pill bar, the feel-tip line, the "120Hz CHAOS SUBSTEPPING" status, the "PHYSICS MIRROR"/"RULE 4 INSTRUMENTATION" badges, the CORRIDOR/COLOSSAL tabs and sector button (move to menus, §2), the ⚙ FEEL KNOBS drawer tab (move to Dev Overlay).

### 1.2 Launch phase (`State.RunPhase === 'LaunchPushOff'`)

Replace the big `#launch-guidance-banner` card with a light prompt in the lower-middle of the screen:

```
            ◀ A        +4.5°        D ▶
                 HOLD  SPACE  TO PUSH
```

- The heading number updates live (from `State.LaunchAimAngleDeg`); A/D glyphs light up while held.
- While SPACE is held, the speed-dial arc fills (launch charge) and the prompt text changes to `RELEASE TO LAUNCH`.
- On launch: prompt fades out over 300 ms. Keep `#btn-charge-push` working for mouse/touch but restyle it as the "HOLD SPACE" text itself (pointer-events on, no big button chrome).

### 1.3 Colossal Belt mode

- Remove `#colossal-mode-banner` entirely from the play screen; its rules text moves to the How to Play page (§2.1). Remove the LOCK / CHARGE buttons on that banner (keyboard/mouse already do it).
- `#aim-crosshair-3d`: keep only the dot ring and brackets, drop the `AIM RETICLE [MOVE MOUSE]` label.
- `#kinetic-target-reticle`: keep the four corners + rotating ring. Replace the target card with one compact line under the box: `48m · 2.1s` (range · time-to-impact). Drop designation, closing speed, diameter. The proximity dilemma becomes the box turning rose + a short `TOO CLOSE` tag.
- Push charge uses the same speed-dial arc.
- The two kinetic metric cards (`card-kinetic-target`, `card-push-force`) go to the Dev Overlay.

### 1.4 Alerts and event feedback (replace the banners)

All event banners (`#strike-banner`, `#slalom-banner`, `#milestone-banner`, `#push-deflection-banner`, `#asteroid-overpower-banner`) become **toasts**: a single `#toast-layer` container above center; each event spawns a short text element that pops in, floats up ~40px and fades over ~900 ms, then is removed. Max 3 visible.

| Event (existing function) | Toast text | Color |
| :--- | :--- | :--- |
| Clean gate (`showSlalomBanner` success) | `CLEAN +{pts}` | cyan |
| Missed gate (`showSlalomBanner` fail) | `MISSED GATE` | rose |
| Milestone (`showMilestoneBanner`) | `{name} +{bonus}` | gold |
| Mace strike (`triggerStrikeFlash`) | `STRIKE` | gold |
| Kinetic push (`triggerPushDeflectionFlash`) | `PUSH {kN} kN` | gold |
| Overpowered (`triggerAsteroidOverpowerFlash`) | `OVERPOWERED!` | rose |

Implement one helper `showToast(text, variant)` and change those functions to call it instead of toggling their banners. Keep the existing screen flashes, sounds and camera shake — they are the "physical cues" the GDD wants. Delete the old banner markup and CSS once nothing references it.

**Proximity warning** (replaces `#proximity-warning-widget`): a rose **screen-edge vignette** whose opacity scales with the existing threat ratio in `updateProximityRadarAlarm`, plus a pulsing `!` chevron on the screen edge pointing toward the nearest threat (project its world position to screen; clamp to the edge). No text unless very close (< 40% of alarm distance): then `IMPACT` under the crosshair.

**Doom warning:** when `State.DoomSnapshot.bIsDoomImminent` is true, desaturate/darken the scene edges (CSS overlay) and show a centered pulsing `NO ANCHORS IN REACH` line. Nothing is shown when safe. (This is a failure alert, not trajectory prediction, so Rule 4 is respected.)

### 1.5 Key hints

No permanent footer. Show a small, low-contrast hint row bottom-right only (a) during the launch phase and (b) for the first 8 s of flight of the player's first 3 runs (count in `localStorage`, wrapped in try/catch): `A / D spin · release to kick · ESC pause`. In Colossal mode: `CLICK/F lock · hold SPACE push · ESC pause`.

---

## 2. Game flow (make it feel like a game)

New run-flow state, separate from `State.RunPhase` physics phases: `UI.screen ∈ 'title' | 'playing' | 'paused' | 'results'`.

### 2.1 Title screen (`#screen-title`) — shown on load

- Big `SPACEMENTUM` logotype over the live 3D starfield/scene (the game keeps rendering behind, slow idle camera drift if easy; otherwise static scene is fine).
- Tagline (from README): *"You can't brake and you can't turn."*
- Menu (keyboard ↑/↓/Enter and mouse): **PLAY**, **SECTOR** (opens selector), **MODE: CORRIDOR / COLOSSAL BELT** (toggle), **HOW TO PLAY**, **SETTINGS**.
- Show personal best for the selected sector under PLAY (from the existing leaderboard profile).
- PLAY → `restartRun()` with the chosen sector/mode → `UI.screen = 'playing'`.
- Reuse the existing sector cards (`#sector-select-modal` content) restyled as the sector selector; reuse `loadSector()` and `setScreenMode()`.
- **How to Play**: 3–4 short illustrated rows (key glyph + one sentence) for Corridor, plus the Colossal rules moved from the old banner, rewritten in plain words (no GDD quotes).
- **Settings**: Master volume, Engine hum, Proximity alarm volume (the three existing audio sliders — move them here, same IDs or re-bind to the same handlers). Persist to `localStorage` with try/catch.

### 2.2 Pause (`#screen-pause`) — `Esc` (and `P`)

- Only from `playing`. Freeze the simulation: in `animate()` skip `physicsTick`, colossal belt updates, dust/spark updates and doom lookahead while paused, but keep rendering. Mute/duck the continuous audio (engine hum, spin synth, proximity alarm) while paused and restore on resume.
- Clear `Input` held flags on pause so keys released during pause don't fire kicks/pushes.
- Menu: **RESUME**, **RESTART (R)**, **SETTINGS**, **QUIT TO TITLE**.
- Auto-pause on `window` `blur` / `visibilitychange` hidden.

### 2.3 Results screens (rework the three existing modals)

Keep the same element IDs the code writes to; strip everything else.

- **Crash** (`#crash-modal`): headline `IMPACT`, one line `Hit {obstacle} at {km/h}` (convert the existing cm/s), then `Distance 412 m · Score 12,450 · Best 14,000`. Buttons: **RETRY [R]**, **MENU**. Remove the GDD quote, "HULL INTEGRITY" row and glitch badge.
- **Void** (`#void-doom-modal`): headline `LOST TO THE VOID`, one line `Nothing left in reach.`, same stats row and buttons.
- **Victory** (`#sector-completed-modal`): huge grade letter, grade title, then 3 stats only — **Time** (vs par), **Score**, **Gates** — plus `NEW BEST` tag when applicable. Leaderboard trimmed to top 5 with columns `# · SCORE · GRADE · TIME`. Buttons: **NEXT SECTOR**, **RETRY [R]**, **MENU**. Drop the quote, avg/peak speed rows (move to Dev Overlay or drop), pilot/speed/date columns.
- Results appear after a short beat (~600 ms) so the crash/flash is seen first; animate in (fade + slight scale). `R` and `Enter` (= primary button) work on all results screens.

### 2.4 Restart feel

- `R` restarts instantly from any in-game state (already works) — make sure it also closes pause/results and sets `UI.screen = 'playing'`.
- Remove the `M` kick-model hotkey from the player keymap (Dev Overlay only, §4).

---

## 3. Visual style

- **No glass panels** on the play screen. HUD elements float directly on the scene with a soft text-shadow (`0 0 8px rgba(0,0,0,.8)`) for legibility.
- **Palette** (reuse existing tokens in `:root`): cyan = info/you, gold = reward/charge, rose = danger. White at 90% for numbers, `--text-muted` for the few labels. Drop violet/emerald from the play HUD (emerald may stay for "cleared" gate ticks).
- **Type:** numbers in `JetBrains Mono` with `font-variant-numeric: tabular-nums`; labels in Inter 600, 10–11px, letter-spacing .14em, uppercase. Only three sizes on the play HUD: large (speed, score ~40–48px), medium (toasts ~22px), small (labels).
- **Scaling:** size HUD with `clamp()` based on `vmin` so it works from 1280×720 to 4K. 24px safe-area margin.
- **Motion:** 150–250 ms ease-out for show/hide; score pop 120 ms; toasts as above. Respect `prefers-reduced-motion` (no float, just fade).
- **Menus/results:** centered single column, max-width ~520px, dark translucent scrim over the scene (`rgba(5,7,13,.72)` + blur 6px), big buttons with clear focus/hover states, the selected menu item marked with a cyan bar. Add a tiny Web Audio blip on menu move/confirm via the existing `getAudioContext()`.
- Update `<title>` to `SpaceMentum`.

---

## 4. Dev Overlay (keep the engineering tools, hide them)

- Wrap the existing `.telemetry-panel` (all metric cards), the cockpit extras (cm/s, vx, v_lat breakdown), and the whole `#tuning-drawer` into `#dev-overlay`, hidden by default.
- Toggle with the **backquote** key (`` ` ``) or `F3`; also open by default when the URL has `?dev=1`.
- `M` (kick model), BLIND MATH and GIZMOS buttons only work while the Dev Overlay is open.
- In `updateHUD()`, split into `updateGameHUD()` (always) and `updateDevHUD()` (only when the overlay is visible) — the existing metric-card code moves into `updateDevHUD()` unchanged.

---

## 5. Code changes (where)

| File | Change |
| :--- | :--- |
| `index.html` | Restructure `#hud-overlay` into: `#game-hud` (progress bar, score, radar, speed dial, launch prompt, hints, crosshair/reticles, `#toast-layer`, `#vignette-proximity`, `#doom-overlay`, threat chevron), `#screen-title`, `#screen-pause`, the three reworked result modals, `#dev-overlay` (old telemetry panel + tuning drawer). Delete old banners/footer/cockpit panel markup once JS no longer references them. Keep `#radar-canvas`. |
| `style.css` | New section `/* --- Minimal Game HUD --- */` and `/* --- Menus & Results --- */`. Remove styles for deleted elements. Keep `.hidden`. |
| `app.js` | (a) `UI` state object + `showScreen(name)`. (b) Cache all HUD element refs once at init instead of `getElementById` every frame; only write `textContent`/`style` when the value changed. (c) `updateGameHUD()` / `updateDevHUD()` split. (d) `showToast()`; rewire the 6 banner functions. (e) Pause logic in `animate()` + key handlers (`Escape`, `p`, `` ` ``, `F3`, menu arrows/Enter). (f) Proximity vignette + edge chevron in `updateProximityRadarAlarm`. (g) Radar draw: remove legend/title dependencies, draw circular. (h) Update the `mousedown` guard (currently checks `.hud-panel`, `.screen-switcher-tabs`, `.colossal-mode-actions`) to ignore clicks on menus/buttons/dev overlay instead. (i) Results modals: new stats row, `MENU` button handlers, delayed reveal. (j) First-runs hint counter. |
| `package.json` | Bump version to `0.8.0`, update description to drop "Cockpit Flight Telemetry". |
| `PROGRESS.md` | Add a session-log row and a short checklist for this step, quoting the Creator's request above. |

---

## 6. Implementation order (each phase ends playable)

1. **Dev Overlay first** — move telemetry panel + tuning drawer into `#dev-overlay`, add the toggle, split `updateHUD`. Screen is now almost empty; game still works. ✔ check: `` ` `` shows all old data.
2. **Core HUD** — progress bar, score + streak, circular radar, speed dial with power arc, crosshair cleanup. Delete the cockpit panel and footer. ✔ check: all four Rule 4 instruments visible and live.
3. **Launch prompt** — replace launch banner. ✔ check: aim, charge, launch all readable without the old card.
4. **Toasts + alerts** — `showToast`, rewire banners, proximity vignette/chevron, doom overlay. ✔ check: fly through a sector and see every event type at least once.
5. **Colossal mode cleanup** — remove banner, compact reticle. ✔ check: lock, charge, push, overpower all still work.
6. **Title, pause, settings, how-to-play.** ✔ check: load → title → play → Esc pause (physics frozen, audio ducked) → resume → quit to title.
7. **Results screens** — crash/void/victory rework with delayed reveal and MENU. ✔ check: all three outcomes reachable; R and Enter work.
8. **Polish + ship** — style pass, `prefers-reduced-motion`, 720p/4K check, remove dead CSS/markup, `node -c app.js`, `npm run package:web`, bump `?v=`, update `PROGRESS.md`.

---

## 6.1 Sessions, commits & progress tracking

The source files are large (`app.js` ~4.3k lines, `style.css` ~2k, `index.html` ~650), so this plan is split across **three sessions** to avoid context compaction losing the ground rules.

| Session | Phases | Notes |
| :--- | :--- | :--- |
| A | 1–4 | Biggest visual change. |
| B | 5–7 | Can absorb phase 8 if context is still healthy. |
| C | 8 | Polish + `dist/` + `PROGRESS.md`. |

**Rules for every session:**
- Start by reading §0 (ground rules), §5 (code map) and this section, then only the sections for your phases.
- **Commit after each phase** (stage only `Tools/CoreFeelPrototype/` source files and this plan; message `feat(prototype-ui): phase N — <short name>`). The first commit also checkpoints the pre-existing uncommitted prototype work — that is intended; do not revert or split it out.
- After finishing a phase, tick it below and add a one-line note (anything the next session must know: renamed functions, leftover TODOs, known bugs).
- If context gets tight mid-session, finish/commit the current phase, update the tracker, and stop.

**Progress tracker:**

- [x] Phase 1 — Dev Overlay — notes: `Input` object was never declared in app.js (pre-existing ReferenceError) — now defined next to new `UI` state; `updateHUD()` = `updateGameHUD()` + `updateDevHUD()` (only if `UI.devOpen`); `setDevOverlay(bool)`; M hotkey dev-only; radar canvas/proximity widget currently live inside the hidden overlay until phase 2 moves them.
- [x] Phase 2 — Core HUD — notes: new `#game-hud` + `HUD` cache object (`cacheHudRefs`, `setHudText`, `rebuildProgressMarkers`); footer + radar widget removed (radar canvas now in `.hud-radar`, circular clip, labels/counter dropped); dev cockpit panel kept as "COCKPIT EXTRAS". CORRIDOR/COLOSSAL tabs + sector button still live in the Dev Overlay header until phase 6 menus. Added missing `playGrazeWarning()` (was an undefined-function crash on missed gates). Old launch/colossal banners still in-flow (centered) until phases 3/5. Dead CSS (cockpit/radar-widget/key-pill) left for phase 8.
- [ ] Phase 3 — Launch prompt — notes:
- [ ] Phase 4 — Toasts + alerts — notes:
- [ ] Phase 5 — Colossal cleanup — notes:
- [ ] Phase 6 — Title / pause / settings / how-to-play — notes:
- [ ] Phase 7 — Results screens — notes:
- [ ] Phase 8 — Polish + ship — notes:

---

## 7. Acceptance checklist

- [ ] During flight, the only always-on HUD elements are: progress bar, score (+streak chip when > 1), radar, speed dial with power arc, crosshair dot. Everything else is contextual.
- [ ] No HUD text is smaller than 10px at 1280×720; nothing overlaps at 1280×720.
- [ ] Rule 4 instruments present (arrow, radar, speedometer, power meter); no trajectory prediction drawn.
- [ ] Game opens on a title screen; Esc pauses and truly freezes physics; R restarts from anywhere; results screens have Retry/Menu (+Next on victory).
- [ ] All old telemetry and tuning sliders are reachable via `` ` `` / `?dev=1` and still work.
- [ ] No console errors; `node -c app.js` passes; `dist/` matches source after `npm run package:web`.
- [ ] No physics/scoring behavior changed (same score for the same run inputs).
