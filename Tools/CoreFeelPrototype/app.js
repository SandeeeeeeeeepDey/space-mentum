/**
 * SpaceMentum — Interactive Core Feel Prototype & Step 5 Cockpit Flight Mirror
 * Direct JavaScript mirror of Unreal Engine 5.8 C++ architecture:
 *   - FCenterOfMassKinematicsSolver (Solvers/CenterOfMassKinematicsSolver.cpp)
 *   - FDoomTrajectoryEvaluator (Solvers/DoomTrajectoryEvaluator.cpp)
 *   - USpaceFlightInstrumentationComponent (UI/SpaceFlightInstrumentationComponent.cpp)
 *   - ASpaceStartingAsteroidActor (Environment/SpaceStartingAsteroidActor.cpp)
 *   - UMaceOrbitComponent (Components/MaceOrbitComponent.cpp)
 *   - UMomentumTransferComponent (Components/MomentumTransferComponent.cpp)
 *   - ASpaceObstacleSpawner (Environment/SpaceObstacleSpawner.cpp)
 *   - UDoomDetectionSubsystem (Subsystems/DoomDetectionSubsystem.cpp)
 *
 * @creator_feel
 * "the concept is a space person zooming in the forward direction in a uniform velocity
 *  and the person has a mace in his hand. by spinning and stopping the spin in a
 *  pertiicular direction, one can stir their direction to move away from obsticles."
 * "the gane starts with me pushing on an asteroid towards the tragactory i want.
 *  al arrow to the direction, a map and speed meter is also needed and the push power too"
 * "abandon realistic space spacing. The levels need to be incredibly dense navigational hazards..."
 * "If the engine calculates the player's trajectory will not intersect... trigger a dramatic
 *  failure state... in the next 13 to 40 seconds."
 * "The 'Blind Math' Frustration is not a problem it is to learn the intuition."
 */

(function () {
  'use strict';

  // --- Physics Tuning Knobs (Mirror of C++ Defaults) ---
  const Tuning = {
    // FLaunchPushOffParameters (§1.4 & Rule 1.1)
    MaxLaunchForceNewtons: 12000.0,
    LaunchSpeedCmPerSec: 3000.0, // 108 km/h departure speed at 100% charge
    ChargeRatePerSec: 0.85,      // ~1.18 seconds to full hydraulic ram compression
    AimAngularSpeedDegPerSec: 65.0,
    MaxAimAngleDeg: 35.0,        // ±35° departure cone off axial forward normal

    // FMaceOrbitTuning
    TetherLengthCm: 180.0,
    MaceMassKg: 40.0,
    SpinAccelerationDegPerSec2: 1440.0,
    MaxAngularSpeedDegPerSec: 1080.0,

    // FMomentumTransferTuning
    AstronautMassKg: 160.0, // Default 1:4 mass ratio against 40 kg mace

    // FForwardDriftTuning
    ForwardSpeedCmPerSec: 3000.0, // 108 km/h sacred forward drift (30 m/s)

    // Doom Detection & Lookahead Bounds (Creator Mandate)
    MinLookaheadSeconds: 13.0,
    MaxLookaheadSeconds: 40.0,
    StallSpeedThresholdCmPerSec: 100.0,

    // Step 5: Camera, VFX & Dynamic Audio Tuning
    MaxSpeedFovDilationDeg: 25.0,
    MaxDutchRollDeg: 4.5,
    TraumaDecayRate: 3.5,
    AudioMasterVolume: 1.0,

    // Step 7: Asset Pipeline, Sound & Visual Polish Tuning
    EngineHumVolume: 0.45,
    ProximityAlarmVolume: 0.75,
    ProximityDistanceThresholdCm: 2500.0,
    PlasmaHeatMultiplier: 1.0,

    // Step 8: Colossal Asteroids, Wide Belt & Kinetic Push Manipulation (§Creator Mandate)
    ActiveScreenMode: 'Gauntlet', // 'Gauntlet' (Slalom Corridor) or 'ColossalBelt' (Monumental Asteroids & Wide Belt)
    KineticMode: 'Push', // 'Push' (Repulsion) or 'Pull' (Attraction - Next Phase)
    ColossalAsteroidScale: 1.0,
    OuterBeltRadiusCm: 35000.0, // 350 meters wide outside view
    IncomingAsteroidSpeedCmPerSec: 1600.0,
    ColossalCrashSpeedCmPerSec: 900.0, // closing speed above this = crash, below = bump (OVERPOWERED)

    // One-Button Push-Off (hold SPACE, release near a rock)
    PushReachCm: 650.0,            // body to rock SURFACE
    PushChargeRatePerSec: 0.9,     // ~1.1 s to full charge
    MinPushCharge: 0.15,           // a tap still shoves
    MaxPushDeltaVCmPerSec: 1400.0, // player dv at full charge vs an immovable rock
    PushReleaseBufferMs: 150.0,    // early release still fires if a rock arrives in this window
  };

  // --- Multi-Sector Celestial Catalog (Step 6) ---
  const SectorCatalog = {
    1: {
      index: 1,
      name: 'Sector 1: Perseus Shallows',
      shortName: 'PERSEUS SHALLOWS',
      briefing: 'Calibrated testing grounds with spaced asteroid debris and 5 slalom gates.',
      corridorLengthCm: 120000, // 1.2 km
      corridorRadiusCm: 800,
      parTimeSeconds: 38.0,
      hazardDensity: 1.0,
      baseForwardSpeedCmPerSec: 3000.0,
      portalColor: 0x00f0ff,
      portalThemeColorStr: '#00f0ff',
      milestones: [
        { index: 1, distCm: 30000, title: 'Alpha Horizon (300m)', bonus: 500, color: 0x00f0ff },
        { index: 2, distCm: 60000, title: 'Debris Midpoint (600m)', bonus: 1000, color: 0xffb703 },
        { index: 3, distCm: 90000, title: 'Portal Approach (900m)', bonus: 1500, color: 0xf72585 }
      ],
      gates: [
        { forwardX: 6000, targetY: -220, targetZ: 140, radius: 190, color: 0x00f0ff, name: 'Gate 1 [Alpha Left]' },
        { forwardX: 12000, targetY: 260, targetZ: -120, radius: 190, color: 0x00f0ff, name: 'Gate 2 [Beta Right]' },
        { forwardX: 18000, targetY: -160, targetZ: -240, radius: 180, color: 0x00f0ff, name: 'Gate 3 [Gamma Low]' },
        { forwardX: 24000, targetY: 280, targetZ: 180, radius: 170, color: 0x00f0ff, name: 'Gate 4 [Delta High-Right]' },
        { forwardX: 30000, targetY: 0, targetZ: 0, radius: 160, color: 0x38bdf8, name: 'Gate 5 [Sector Threshold]' },
      ]
    },
    2: {
      index: 2,
      name: 'Sector 2: Kepler Graveyard',
      shortName: 'KEPLER GRAVEYARD',
      briefing: 'Dense orbital wreckage and shattered solar arrays requiring precise deflection strikes.',
      corridorLengthCm: 150000, // 1.5 km
      corridorRadiusCm: 750,
      parTimeSeconds: 48.0,
      hazardDensity: 1.35,
      baseForwardSpeedCmPerSec: 3200.0,
      portalColor: 0xf59e0b,
      portalThemeColorStr: '#f59e0b',
      milestones: [
        { index: 1, distCm: 37500, title: 'Solar Array Relic (375m)', bonus: 750, color: 0xf59e0b },
        { index: 2, distCm: 75000, title: 'Kepler Core Hulk (750m)', bonus: 1500, color: 0x00f0ff },
        { index: 3, distCm: 112500, title: 'Vector Event Horizon (1125m)', bonus: 2250, color: 0xd946ef }
      ],
      gates: [
        { forwardX: 7000, targetY: 240, targetZ: 100, radius: 180, color: 0xf59e0b, name: 'Gate 1 [Solar Array Beta]' },
        { forwardX: 15000, targetY: -220, targetZ: -180, radius: 180, color: 0xf59e0b, name: 'Gate 2 [Array Gap]' },
        { forwardX: 25000, targetY: 280, targetZ: -140, radius: 170, color: 0xf59e0b, name: 'Gate 3 [Kepler Mast]' },
        { forwardX: 38000, targetY: -260, targetZ: 200, radius: 170, color: 0xf59e0b, name: 'Gate 4 [Hull Flange]' },
        { forwardX: 52000, targetY: 0, targetZ: -250, radius: 160, color: 0xf59e0b, name: 'Gate 5 [Radiator Shunt]' },
        { forwardX: 68000, targetY: -250, targetZ: 0, radius: 160, color: 0xf59e0b, name: 'Gate 6 [Core Chimney]' },
        { forwardX: 85000, targetY: 200, targetZ: 180, radius: 160, color: 0xf59e0b, name: 'Gate 7 [Escape Channel]' },
      ]
    },
    3: {
      index: 3,
      name: 'Sector 3: Tartarus Belt',
      shortName: 'TARTARUS BELT',
      briefing: 'Claustrophobic iron-nickel asteroid gauntlet. Zero error tolerance.',
      corridorLengthCm: 180000, // 1.8 km
      corridorRadiusCm: 700,
      parTimeSeconds: 56.0,
      hazardDensity: 1.75,
      baseForwardSpeedCmPerSec: 3400.0,
      portalColor: 0xd946ef,
      portalThemeColorStr: '#d946ef',
      milestones: [
        { index: 1, distCm: 45000, title: 'Tartarus Gate (450m)', bonus: 1000, color: 0xd946ef },
        { index: 2, distCm: 90000, title: 'Iron Trench (900m)', bonus: 2000, color: 0x00f0ff },
        { index: 3, distCm: 135000, title: 'Singularity Threshold (1350m)', bonus: 3000, color: 0xffb703 }
      ],
      gates: [
        { forwardX: 6500, targetY: -280, targetZ: 120, radius: 170, color: 0xd946ef, name: 'Gate 1 [Nickel Spine]' },
        { forwardX: 14000, targetY: 260, targetZ: -160, radius: 165, color: 0xd946ef, name: 'Gate 2 [Hematite Arch]' },
        { forwardX: 23000, targetY: -200, targetZ: -240, radius: 160, color: 0xd946ef, name: 'Gate 3 [Shatter Gap]' },
        { forwardX: 34000, targetY: 240, targetZ: 200, radius: 160, color: 0xd946ef, name: 'Gate 4 [Vortex Rib]' },
        { forwardX: 47000, targetY: -260, targetZ: -100, radius: 155, color: 0xd946ef, name: 'Gate 5 [Basalt Needle]' },
        { forwardX: 62000, targetY: 220, targetZ: -200, radius: 155, color: 0xd946ef, name: 'Gate 6 [Gravity Well]' },
        { forwardX: 79000, targetY: -180, targetZ: 240, radius: 150, color: 0xd946ef, name: 'Gate 7 [Magma Crust]' },
        { forwardX: 98000, targetY: 250, targetZ: 0, radius: 150, color: 0xd946ef, name: 'Gate 8 [Trench Throat]' },
        { forwardX: 118000, targetY: 0, targetZ: -220, radius: 150, color: 0xd946ef, name: 'Gate 9 [Singularity Rim]' },
      ]
    }
  };

  // --- Native LocalStorage SaveGame Persistence Mirror ---
  const SaveGame = {
    STORAGE_KEY: 'SPACEMENTUM_PILOT_PROFILE_V1',
    getProfile() {
      try {
        const raw = localStorage.getItem(this.STORAGE_KEY);
        if (raw) return JSON.parse(raw);
      } catch (e) {
        // Fallback for private browsing
      }
      return {
        callsign: 'Astronaut-77',
        highestUnlockedSector: 1,
        personalBests: {},
        leaderboards: { 1: [], 2: [], 3: [] },
      };
    },
    saveProfile(profile) {
      try {
        localStorage.setItem(this.STORAGE_KEY, JSON.stringify(profile));
      } catch (e) {}
    },
    recordRun(scoreData) {
      const profile = this.getProfile();
      let isNewPB = false;
      const sectorPB = profile.personalBests[scoreData.sectorIndex];
      if (!sectorPB || scoreData.compositeScore > sectorPB.compositeScore) {
        profile.personalBests[scoreData.sectorIndex] = scoreData;
        isNewPB = true;
      }
      if (!profile.leaderboards[scoreData.sectorIndex]) {
        profile.leaderboards[scoreData.sectorIndex] = [];
      }
      const entry = {
        callsign: profile.callsign,
        score: scoreData.compositeScore,
        grade: scoreData.grade,
        time: scoreData.timeSeconds.toFixed(1) + 's',
        speed: (scoreData.topSpeedCms * 0.036).toFixed(1) + ' km/h',
        date: new Date().toLocaleDateString()
      };
      profile.leaderboards[scoreData.sectorIndex].push(entry);
      profile.leaderboards[scoreData.sectorIndex].sort((a, b) => b.score - a.score);
      if (profile.leaderboards[scoreData.sectorIndex].length > 10) {
        profile.leaderboards[scoreData.sectorIndex].length = 10;
      }
      if (scoreData.bCompleted && scoreData.sectorIndex >= profile.highestUnlockedSector) {
        profile.highestUnlockedSector = Math.min(3, scoreData.sectorIndex + 1);
      }
      this.saveProfile(profile);
      return { isNewPB, profile };
    }
  };

  // --- Live Physics & Run State ---
  const State = {
    // Run Lifecycle & Fail States (Rule 3 & F-7)
    RunPhase: 'LaunchPushOff', // 'LaunchPushOff', 'Running', 'Completed', 'LostToTheVoid', 'CrashedKineticImpact'

    // Step 6: Multi-Sector Progression & Flight Scoring
    CurrentSectorIndex: 1,
    ActiveFlightTimeSeconds: 0.0,
    CleanGatesCleared: 0,
    CleanEvasionStreak: 0,
    GrazePenalties: 0,
    PeakSpeedCmPerSec: 3000.0,
    SpeedSampleSum: 0.0,
    SpeedSampleCount: 0,
    CurrentCompositeScore: 0,
    MilestonesPassed: {},

    // Launch Push-Off (§1.4)
    LaunchAimAngleDeg: 0.0,
    LaunchPushCharge: 0.0,
    bIsChargingPush: false,

    // FMaceOrbitState
    OrbitAngleRad: 0.0,
    AngularVelocityRadPerSec: 0.0,
    HeldSpinDirection: 'None', // 'CounterClockwise', 'Clockwise', 'None'

    // System Center of Mass World Position & Velocity
    CoMPosition: { x: 0.0, y: 0.0, z: 0.0 },
    CoMVelocity: { x: 0.0, y: 0.0, z: 0.0 }, // 0 at launch anchor; base drift after push-off

    // Internal 1:4 Two-Body Counter-Displacement (The Dodge)
    InternalDisplacement: { y: 0.0, z: 0.0 },
    CounterRotationRad: 0.0,

    // Stats
    StrikeCount: 0,
    LastStrikeTime: 0,

    // Replayable sim: gameplay clock (ms, advanced only by physicsTick) + seeded RNG for physics/spawns
    SimTimeMs: 0,
    RngSeed: 1,

    // Doom Lookahead Diagnosis
    DoomSnapshot: {
      bIsDoomImminent: false,
      reason: 'None',
      lookaheadHorizon: 40.0,
      reachableCount: 0,
      nearestTime: -1.0,
    },

    // Camera FOV, Dutch Roll & Screen Trauma (Step 5)
    CurrentFOV: 80.0,
    TargetFOV: 80.0,
    CameraRollDeg: 0.0,
    CameraTrauma: 0.0,
    CameraDistance: 650.0,

    // Step 7: Proximity Radar Alert State
    ProximityAlarm: {
      bIsActive: false,
      distanceCm: 10000.0,
      threatRatio: 0.0,
    },

    // Rule 4 "Blind Math" Policy: Defaults to TRUE (no predictive trajectory arcs)
    bBlindMath: true,
    bShowGizmos: false,

    // Step 8: Push State
    KineticPushCharge: 0.0,
    bIsChargingKineticPush: false,
    PushAnchor: null,
    PushBuffer: null,    // { until, charge } armed by an early release
    MaceSnap: null,      // { targetRad, remaining } visual arm reach on push-off
    PushCue: null,       // { obs, until } one-shot SPACE glyph near the first rock in reach
    PushCueShown: false,
  };

  // --- Held Input Flags ---
  const Input = {
    isAPressed: false,
    isDPressed: false,
    isSpacePressed: false,
    lastAPressTime: 0, // SOCD ordering: monotonically increasing press sequence numbers
    lastDPressTime: 0,
    seq: 0,
  };

  // Player-facing options (persisted with the other settings)
  const Settings = { crashTips: true };

  // --- Replayable Sim: silent re-runs of the real physics (crash analysis, §2 of PLAN_CrashEscapeHint) ---
  // While Sim.silent is true nothing may touch audio, DOM or scene objects; end-of-run triggers
  // only record Sim.outcome.
  const Sim = { silent: false, outcome: null, saved: null, override: null, ignoreVoid: false, voidSeen: false };

  const SIM_DT = 1 / 120;          // fixed physics step (real game and silent re-runs alike)
  const HIST_SAMPLE_STEPS = 4;     // one history snapshot per 1/30 s
  const HIST_WINDOW_MS = 6000;

  // mulberry32 over State.RngSeed: used for everything that changes physics or spawns
  function simRandom() {
    let t = (State.RngSeed = (State.RngSeed + 0x6D2B79F5) >>> 0);
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  }

  // --- Game Flow / UI State (separate from State.RunPhase physics phases) ---
  const UI = {
    screen: 'playing', // 'title' | 'playing' | 'paused' | 'results'
    sub: null, // open sub-screen over title/pause: 'howto' | 'settings' | 'sector'
    devOpen: false,
    audioDucked: false,
    menuIndex: 0,
    title: { sector: 1, mode: 'Gauntlet' }, // selections made on the title screen
  };

  // --- Dynamic Slalom Gates (Populated by loadSector) ---
  let SlalomTargets = [];

  // --- Web Audio API Procedural Synthesizer ---
  let audioCtx = null;
  let continuousSpinOsc = null;
  let continuousSpinGain = null;
  let continuousReactorHumOsc = null;
  let continuousReactorHumGain = null;
  let continuousReactorFilter = null;
  let lastProximityPingTime = 0;

  function getAudioContext() {
    if (Sim.silent) return null;
    if (!audioCtx) {
      const AudioContextClass = window.AudioContext || window.webkitAudioContext;
      if (AudioContextClass) {
        audioCtx = new AudioContextClass();
        setupContinuousSpinSynth();
        applyAudioDuck();
      }
    }
    if (audioCtx && audioCtx.state === 'suspended') {
      audioCtx.resume();
    }
    return audioCtx;
  }

  // Pause / menus: silence the continuous loops (the sim that retargets them is frozen meanwhile)
  function applyAudioDuck() {
    if (!audioCtx || !continuousSpinGain || !continuousReactorHumGain) return;
    const t = audioCtx.currentTime;
    if (UI.audioDucked) {
      continuousSpinGain.gain.setTargetAtTime(0.0001, t, 0.05);
      continuousReactorHumGain.gain.setTargetAtTime(0.0001, t, 0.05);
    } else {
      continuousReactorHumGain.gain.setTargetAtTime(0.035 * Tuning.EngineHumVolume * Tuning.AudioMasterVolume, t, 0.1);
    }
  }

  function setupContinuousSpinSynth() {
    if (!audioCtx) return;
    try {
      // 1. Continuous Mace Spin Synth
      continuousSpinOsc = audioCtx.createOscillator();
      continuousSpinGain = audioCtx.createGain();

      continuousSpinOsc.type = 'sine';
      continuousSpinOsc.frequency.setValueAtTime(60, audioCtx.currentTime);
      continuousSpinGain.gain.setValueAtTime(0.0001, audioCtx.currentTime);

      continuousSpinOsc.connect(continuousSpinGain);
      continuousSpinGain.connect(audioCtx.destination);
      continuousSpinOsc.start();

      // 2. Step 7: Continuous Propulsion Reactor & Suit Life-Support Hum
      continuousReactorHumOsc = audioCtx.createOscillator();
      continuousReactorFilter = audioCtx.createBiquadFilter();
      continuousReactorHumGain = audioCtx.createGain();

      continuousReactorHumOsc.type = 'sawtooth';
      continuousReactorHumOsc.frequency.setValueAtTime(55, audioCtx.currentTime);
      continuousReactorFilter.type = 'lowpass';
      continuousReactorFilter.frequency.setValueAtTime(110, audioCtx.currentTime);

      continuousReactorHumGain.gain.setValueAtTime(0.035 * Tuning.EngineHumVolume * Tuning.AudioMasterVolume, audioCtx.currentTime);

      continuousReactorHumOsc.connect(continuousReactorFilter);
      continuousReactorFilter.connect(continuousReactorHumGain);
      continuousReactorHumGain.connect(audioCtx.destination);
      continuousReactorHumOsc.start();
    } catch (e) {
      // Audio autoplay policy fallback
    }
  }

  function updateContinuousSpinAudio(angularSpeedRadPerSec) {
    if (Sim.silent || !audioCtx || !continuousSpinOsc || !continuousSpinGain) return;
    const revPerSec = Math.abs(angularSpeedRadPerSec) / (2 * Math.PI);
    if (revPerSec > 0.1 && State.RunPhase === 'Running') {
      const targetFreq = Math.min(650, 75 + revPerSec * 160);
      const targetVol = Math.min(0.25, revPerSec * 0.08) * Tuning.AudioMasterVolume;
      continuousSpinOsc.frequency.setTargetAtTime(targetFreq, audioCtx.currentTime, 0.05);
      continuousSpinGain.gain.setTargetAtTime(targetVol, audioCtx.currentTime, 0.05);
    } else {
      continuousSpinGain.gain.setTargetAtTime(0.0001, audioCtx.currentTime, 0.08);
    }
  }

  function updateContinuousEngineHumAudio(forwardSpeedCmPerSec, lateralSpeedCmPerSec) {
    if (Sim.silent || !audioCtx || !continuousReactorHumOsc || !continuousReactorHumGain) return;
    if (State.RunPhase === 'Running') {
      const speedRatio = Math.min(1.5, Math.hypot(forwardSpeedCmPerSec, lateralSpeedCmPerSec) / 3000.0);
      const targetFreq = 50 + speedRatio * 25.0;
      const targetVol = (0.035 + speedRatio * 0.04) * Tuning.EngineHumVolume * Tuning.AudioMasterVolume;
      continuousReactorHumOsc.frequency.setTargetAtTime(targetFreq, audioCtx.currentTime, 0.1);
      continuousReactorHumGain.gain.setTargetAtTime(targetVol, audioCtx.currentTime, 0.1);
    } else {
      continuousReactorHumGain.gain.setTargetAtTime(0.015 * Tuning.EngineHumVolume * Tuning.AudioMasterVolume, audioCtx.currentTime, 0.2);
    }
  }

  function playProximityRadarPing(threatRatio) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const pingOsc = ctx.createOscillator();
    const pingGain = ctx.createGain();

    pingOsc.type = 'sine';
    const baseFreq = 640 + threatRatio * 860;
    pingOsc.frequency.setValueAtTime(baseFreq, now);
    pingOsc.frequency.exponentialRampToValueAtTime(baseFreq + 120, now + 0.05);

    const volume = (0.08 + threatRatio * 0.35) * Tuning.ProximityAlarmVolume * Tuning.AudioMasterVolume;
    pingGain.gain.setValueAtTime(volume, now);
    pingGain.gain.exponentialRampToValueAtTime(0.001, now + 0.055);

    pingOsc.connect(pingGain);
    pingGain.connect(ctx.destination);
    pingOsc.start(now);
    pingOsc.stop(now + 0.055);
  }

  function updateProximityRadarAlarm(minDistCm, nowSeconds, nearestPos) {
    const thresholdCm = Tuning.ProximityDistanceThresholdCm;

    if (minDistCm <= thresholdCm && State.RunPhase === 'Running') {
      const threatRatio = Math.max(0.0, Math.min(1.0, 1.0 - (minDistCm / thresholdCm)));
      State.ProximityAlarm.bIsActive = true;
      State.ProximityAlarm.threatRatio = threatRatio;
      State.ProximityAlarm.distanceCm = minDistCm;
      State.ProximityAlarm.nearestPos = nearestPos || null;

      const pingInterval = 1.0 - (threatRatio * 0.92);
      if (!Sim.silent && nowSeconds - lastProximityPingTime >= pingInterval) {
        lastProximityPingTime = nowSeconds;
        playProximityRadarPing(threatRatio);
      }
    } else {
      State.ProximityAlarm.bIsActive = false;
      State.ProximityAlarm.threatRatio = 0.0;
      State.ProximityAlarm.distanceCm = minDistCm;
      State.ProximityAlarm.nearestPos = null;
    }
  }

  function playPortalWarpVacuumCollapse() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const sweepOsc = ctx.createOscillator();
    const sweepGain = ctx.createGain();
    sweepOsc.type = 'sawtooth';
    sweepOsc.frequency.setValueAtTime(540, now);
    sweepOsc.frequency.exponentialRampToValueAtTime(32, now + 0.9);
    sweepGain.gain.setValueAtTime(0.7 * Tuning.AudioMasterVolume, now);
    sweepGain.gain.exponentialRampToValueAtTime(0.001, now + 1.0);
    sweepOsc.connect(sweepGain);
    sweepGain.connect(ctx.destination);
    sweepOsc.start(now);
    sweepOsc.stop(now + 1.0);

    const noiseBuffer = ctx.createBuffer(1, ctx.sampleRate * 0.8, ctx.sampleRate);
    const data = noiseBuffer.getChannelData(0);
    for (let i = 0; i < noiseBuffer.length; i++) {
      data[i] = (Math.random() * 2 - 1) * Math.exp(-i / (ctx.sampleRate * 0.25));
    }
    const noiseSource = ctx.createBufferSource();
    noiseSource.buffer = noiseBuffer;
    const noiseFilter = ctx.createBiquadFilter();
    noiseFilter.type = 'lowpass';
    noiseFilter.frequency.setValueAtTime(1400, now);
    noiseFilter.frequency.exponentialRampToValueAtTime(180, now + 0.8);
    const noiseGain = ctx.createGain();
    noiseGain.gain.setValueAtTime(0.5 * Tuning.AudioMasterVolume, now);
    noiseGain.gain.exponentialRampToValueAtTime(0.001, now + 0.8);
    noiseSource.connect(noiseFilter);
    noiseFilter.connect(noiseGain);
    noiseGain.connect(ctx.destination);
    noiseSource.start(now);
  }

  function playLaunchChargeSound(ratio) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'sawtooth';

    const startFreq = 120 + ratio * 320;
    osc.frequency.setValueAtTime(startFreq, now);
    osc.frequency.exponentialRampToValueAtTime(startFreq + 60, now + 0.08);

    gain.gain.setValueAtTime(0.08 + ratio * 0.15, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.08);

    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.08);
  }

  // --- Step 8: Kinetic Manipulator Web Audio Synthesis ---
  function playKineticChargeSound(ratio) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'triangle';

    const freq = 180 + ratio * 720;
    osc.frequency.setValueAtTime(freq, now);
    osc.frequency.exponentialRampToValueAtTime(freq + 40, now + 0.06);

    gain.gain.setValueAtTime((0.05 + ratio * 0.18) * Tuning.AudioMasterVolume, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.06);

    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.06);
  }

  function playKineticPushBlastSound(forceKN) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    // 1. Deep sub-bass repulsion thump (38Hz -> 18Hz)
    const subOsc = ctx.createOscillator();
    const subGain = ctx.createGain();
    subOsc.type = 'sine';
    subOsc.frequency.setValueAtTime(80, now);
    subOsc.frequency.exponentialRampToValueAtTime(22, now + 0.45);
    subGain.gain.setValueAtTime(0.95 * Tuning.AudioMasterVolume, now);
    subGain.gain.exponentialRampToValueAtTime(0.001, now + 0.5);
    subOsc.connect(subGain);
    subGain.connect(ctx.destination);
    subOsc.start(now);
    subOsc.stop(now + 0.5);

    // 2. High-energy plasma crackle burst
    const noiseBuffer = ctx.createBuffer(1, ctx.sampleRate * 0.3, ctx.sampleRate);
    const data = noiseBuffer.getChannelData(0);
    for (let i = 0; i < noiseBuffer.length; i++) {
      data[i] = (Math.random() * 2 - 1) * Math.exp(-i / (ctx.sampleRate * 0.08));
    }
    const noise = ctx.createBufferSource();
    noise.buffer = noiseBuffer;
    const filter = ctx.createBiquadFilter();
    filter.type = 'highpass';
    filter.frequency.setValueAtTime(800, now);
    const nGain = ctx.createGain();
    nGain.gain.setValueAtTime(0.65 * Tuning.AudioMasterVolume, now);
    nGain.gain.exponentialRampToValueAtTime(0.001, now + 0.3);
    noise.connect(filter);
    filter.connect(nGain);
    nGain.connect(ctx.destination);
    noise.start(now);
  }

  function playAsteroidOverpoweredSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    // Concussive crushing impact sound
    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(110, now);
    osc.frequency.exponentialRampToValueAtTime(18, now + 0.6);
    gain.gain.setValueAtTime(1.0 * Tuning.AudioMasterVolume, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.65);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.65);
  }

  function playLaunchExplosionThump() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    // 1. Deep 42Hz Sub-bass punch transient
    const subOsc = ctx.createOscillator();
    const subGain = ctx.createGain();
    subOsc.type = 'sine';
    subOsc.frequency.setValueAtTime(140, now);
    subOsc.frequency.exponentialRampToValueAtTime(36, now + 0.45);
    subGain.gain.setValueAtTime(1.0, now);
    subGain.gain.exponentialRampToValueAtTime(0.001, now + 0.5);
    subOsc.connect(subGain);
    subGain.connect(ctx.destination);
    subOsc.start(now);
    subOsc.stop(now + 0.5);

    // 2. High-pressure hydraulic exhaust release hiss
    const noiseBuffer = ctx.createBuffer(1, ctx.sampleRate * 0.35, ctx.sampleRate);
    const output = noiseBuffer.getChannelData(0);
    for (let i = 0; i < noiseBuffer.length; i++) {
      output[i] = Math.random() * 2 - 1;
    }
    const whiteNoise = ctx.createBufferSource();
    whiteNoise.buffer = noiseBuffer;

    const filter = ctx.createBiquadFilter();
    filter.type = 'bandpass';
    filter.frequency.setValueAtTime(1800, now);
    filter.frequency.exponentialRampToValueAtTime(320, now + 0.35);
    filter.Q.setValueAtTime(3.0, now);

    const noiseGain = ctx.createGain();
    noiseGain.gain.setValueAtTime(0.55, now);
    noiseGain.gain.exponentialRampToValueAtTime(0.001, now + 0.35);

    whiteNoise.connect(filter);
    filter.connect(noiseGain);
    noiseGain.connect(ctx.destination);
    whiteNoise.start(now);
    whiteNoise.stop(now + 0.35);
  }

  function playKickSound(gainScale = 1.0) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();

    osc.type = 'triangle';
    osc.frequency.setValueAtTime(190, now);
    osc.frequency.exponentialRampToValueAtTime(32, now + 0.18);

    gain.gain.setValueAtTime(0.65 * gainScale, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.22);

    osc.connect(gain);
    gain.connect(ctx.destination);

    osc.start(now);
    osc.stop(now + 0.22);
  }

  function playStrikeSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    // 1. Resonant metallic ping
    const metalOsc = ctx.createOscillator();
    const metalGain = ctx.createGain();
    metalOsc.type = 'square';
    metalOsc.frequency.setValueAtTime(560, now);
    metalOsc.frequency.exponentialRampToValueAtTime(180, now + 0.28);
    metalGain.gain.setValueAtTime(0.55, now);
    metalGain.gain.exponentialRampToValueAtTime(0.001, now + 0.32);
    metalOsc.connect(metalGain);
    metalGain.connect(ctx.destination);
    metalOsc.start(now);
    metalOsc.stop(now + 0.32);

    // 2. Sub-bass momentum kick
    const subOsc = ctx.createOscillator();
    const subGain = ctx.createGain();
    subOsc.type = 'sine';
    subOsc.frequency.setValueAtTime(95, now);
    subOsc.frequency.exponentialRampToValueAtTime(25, now + 0.35);
    subGain.gain.setValueAtTime(0.85, now);
    subGain.gain.exponentialRampToValueAtTime(0.001, now + 0.35);
    subOsc.connect(subGain);
    subGain.connect(ctx.destination);
    subOsc.start(now);
    subOsc.stop(now + 0.35);
  }

  function playVoidDoomSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(45, now);
    osc.frequency.linearRampToValueAtTime(28, now + 2.5);

    gain.gain.setValueAtTime(0.7, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 2.8);

    osc.connect(gain);
    gain.connect(ctx.destination);

    osc.start(now);
    osc.stop(now + 2.8);
  }

  function playCrashSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(140, now);
    osc.frequency.exponentialRampToValueAtTime(20, now + 0.45);

    gain.gain.setValueAtTime(0.9, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.5);

    osc.connect(gain);
    gain.connect(ctx.destination);

    osc.start(now);
    osc.stop(now + 0.5);
  }

  function playClearChime() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    [880, 1320].forEach((freq, idx) => {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.type = 'sine';
      osc.frequency.setValueAtTime(freq, now + idx * 0.06);
      gain.gain.setValueAtTime(0.3 * Tuning.AudioMasterVolume, now + idx * 0.06);
      gain.gain.exponentialRampToValueAtTime(0.001, now + idx * 0.06 + 0.35);
      osc.connect(gain);
      gain.connect(ctx.destination);
      osc.start(now + idx * 0.06);
      osc.stop(now + idx * 0.06 + 0.35);
    });
  }

  function playGrazeWarning() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'square';
    osc.frequency.setValueAtTime(220, now);
    osc.frequency.exponentialRampToValueAtTime(110, now + 0.22);
    gain.gain.setValueAtTime(0.14 * Tuning.AudioMasterVolume, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.25);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.25);
  }

  function playMilestoneChime() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    [523.25, 659.25, 783.99, 1046.50].forEach((freq, idx) => {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.type = 'triangle';
      osc.frequency.setValueAtTime(freq, now + idx * 0.07);
      gain.gain.setValueAtTime(0.28 * Tuning.AudioMasterVolume, now + idx * 0.07);
      gain.gain.exponentialRampToValueAtTime(0.001, now + idx * 0.07 + 0.45);
      osc.connect(gain);
      gain.connect(ctx.destination);
      osc.start(now + idx * 0.07);
      osc.stop(now + idx * 0.07 + 0.45);
    });
  }

  function playPortalWarpFanfare() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    // Sub-bass hyperspace punch
    const subOsc = ctx.createOscillator();
    const subGain = ctx.createGain();
    subOsc.type = 'sawtooth';
    subOsc.frequency.setValueAtTime(320, now);
    subOsc.frequency.exponentialRampToValueAtTime(45, now + 0.85);
    subGain.gain.setValueAtTime(0.75 * Tuning.AudioMasterVolume, now);
    subGain.gain.exponentialRampToValueAtTime(0.001, now + 1.1);
    subOsc.connect(subGain);
    subGain.connect(ctx.destination);
    subOsc.start(now);
    subOsc.stop(now + 1.1);

    // Triumphant orchestral triad chord
    [440.0, 554.37, 659.25, 880.0].forEach((freq, idx) => {
      const chordOsc = ctx.createOscillator();
      const chordGain = ctx.createGain();
      chordOsc.type = 'sine';
      chordOsc.frequency.setValueAtTime(freq, now + 0.12);
      chordGain.gain.setValueAtTime(0.3 * Tuning.AudioMasterVolume, now + 0.12);
      chordGain.gain.exponentialRampToValueAtTime(0.001, now + 1.6);
      chordOsc.connect(chordGain);
      chordGain.connect(ctx.destination);
      chordOsc.start(now + 0.12);
      chordOsc.stop(now + 1.6);
    });
  }

  // --- Input events: the DOM handlers and the silent replays both go through here ---
  function applyInputEvent(name) {
    if (!Sim.silent && Hist.active) Hist.events.push({ t: State.SimTimeMs, type: 'input', name });
    switch (name) {
      case 'aDown': Input.isAPressed = true; Input.lastAPressTime = ++Input.seq; evaluateActiveSpin(); break;
      case 'dDown': Input.isDPressed = true; Input.lastDPressTime = ++Input.seq; evaluateActiveSpin(); break;
      case 'aUp': Input.isAPressed = false; evaluateActiveSpin(); break;
      case 'dUp': Input.isDPressed = false; evaluateActiveSpin(); break;
      case 'spaceDown':
        Input.isSpacePressed = true;
        if (State.RunPhase === 'LaunchPushOff') startChargingPush();
        else if (State.RunPhase === 'Running') startPushCharge();
        break;
      case 'spaceUp':
        Input.isSpacePressed = false;
        if (State.RunPhase === 'LaunchPushOff') fireLaunchPush(); else releasePush();
        break;
      case 'launchDown': startChargingPush(); break;
      case 'launchUp': fireLaunchPush(); break;
      case 'clear': clearHeldInput(); break;
    }
  }

  // --- SOCD Input Handling ---
  function evaluateActiveSpin() {
    if (State.RunPhase !== 'Running') return;

    if (Input.isAPressed && Input.isDPressed) {
      State.HeldSpinDirection = Input.lastAPressTime > Input.lastDPressTime ? 'CounterClockwise' : 'Clockwise';
    } else if (Input.isAPressed) {
      State.HeldSpinDirection = 'CounterClockwise';
    } else if (Input.isDPressed) {
      State.HeldSpinDirection = 'Clockwise';
    } else {
      if (State.HeldSpinDirection !== 'None') {
        stopSpin();
      }
      State.HeldSpinDirection = 'None';
    }
  }

  // --- Spin Stop (Internal Release: no velocity change, ever) ---
  function stopSpin() {
    // Spinning the mace is an internal force: it can shift the body around the CoM
    // but can never change the CoM path. Letting go just halts the spin.
    const bWasSpinning = Math.abs(State.AngularVelocityRadPerSec) > 0.5;
    State.AngularVelocityRadPerSec = 0.0;
    if (bWasSpinning) playKickSound(0.18);
  }

  // --- External Strike Momentum Transfer ---
  function executeExternalStrike(obstacle, hitWorldPos, normal, u) {
    const strikeEfficiency = 0.90;
    const totalMass = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const cosT = Math.cos(State.OrbitAngleRad);
    const sinT = Math.sin(State.OrbitAngleRad);
    const sign = State.AngularVelocityRadPerSec >= 0 ? 1.0 : -1.0;
    const tanSpeed = Math.max(800.0, Math.abs(State.AngularVelocityRadPerSec) * Tuning.TetherLengthCm);

    const tangentialNormal = new THREE.Vector3(0, -sinT * sign, cosT * sign).normalize();
    const impulseDirection = normal.clone().negate().reflect(normal).normalize();

    const maceFactor = (0.35 + u * 0.65);
    const impulseMag = strikeEfficiency * (1.0 + obstacle.restitution) * Tuning.MaceMassKg * tanSpeed * maceFactor;
    const deltaV = impulseDirection.clone().multiplyScalar(impulseMag / totalMass);

    State.CoMVelocity.y += deltaV.y;
    State.CoMVelocity.z += deltaV.z;

    const maxDeflection = 1400.0;
    State.CoMVelocity.y = THREE.MathUtils.clamp(State.CoMVelocity.y, -maxDeflection, maxDeflection);
    State.CoMVelocity.z = THREE.MathUtils.clamp(State.CoMVelocity.z, -maxDeflection, maxDeflection);

    State.AngularVelocityRadPerSec *= -0.35;

    const pushDist = Math.max(35.0, deltaV.length() * 0.15);
    State.CoMPosition.y += normal.y * pushDist;
    State.CoMPosition.z += normal.z * pushDist;

    State.StrikeCount++;
    State.LastStrikeTime = State.SimTimeMs;

    if (!Sim.silent) {
      playStrikeSound();
      triggerStrikeFlash(obstacle, deltaV, u);
      spawnKineticSparks(hitWorldPos, 35);
    }

    State.TargetFOV = Math.min(106.0, State.CurrentFOV + 14.0);
    State.CameraTrauma = Math.min(1.0, State.CameraTrauma + 0.45);
  }

  // --- Toasts: short floating event text (Creator: minimal HUD, only what the player needs) ---
  const TOAST_MAX_VISIBLE = 3;

  function showToast(text, variant) {
    if (Sim.silent) return;
    const layer = document.getElementById('toast-layer');
    if (!layer) return;
    while (layer.children.length >= TOAST_MAX_VISIBLE) layer.removeChild(layer.firstChild);
    const el = document.createElement('div');
    el.className = `toast ${variant || 'cyan'}`;
    el.textContent = text;
    layer.appendChild(el);
    el.addEventListener('animationend', () => el.remove());
    setTimeout(() => el.remove(), 1100); // fallback if animations are disabled
  }

  function clearToasts() {
    const layer = document.getElementById('toast-layer');
    if (layer) layer.innerHTML = '';
  }

  function triggerStrikeFlash() {
    if (Sim.silent) return;
    showToast('STRIKE', 'gold');

    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = 'brightness(1.6) drop-shadow(0 0 35px rgba(0, 240, 255, 0.9))';
      setTimeout(() => { container.style.filter = 'none'; }, 160);
    }
  }

  // --- Rule 3 Fail State Triggers ---
  function triggerVoidDoom(snapshot) {
    if (State.RunPhase !== 'Running') return;
    if (Sim.silent && Sim.ignoreVoid) {
      // Crash analysis keeps flying a doomed line so it can still tell whether the rock was avoided
      Sim.voidSeen = true;
      return;
    }
    State.RunPhase = 'LostToTheVoid';
    if (Sim.silent) {
      Sim.outcome = { type: 'void', simTimeMs: State.SimTimeMs, obstacle: null };
      return;
    }

    playVoidDoomSound();

    fillRunStats('void');
    showResults('void-doom-modal');
  }

  function triggerKineticCrash(obstacle, impactSpeed) {
    if (State.RunPhase !== 'Running') return;
    State.RunPhase = 'CrashedKineticImpact';
    const crashPos = { x: State.CoMPosition.x, y: State.CoMPosition.y, z: State.CoMPosition.z };
    if (Sim.silent) {
      Sim.outcome = { type: 'crash', simTimeMs: State.SimTimeMs, obstacle: obstacle || null, pos: crashPos };
      return;
    }
    Hist.crash = {
      simTimeMs: State.SimTimeMs,
      pos: crashPos,
      body: {
        x: State.CoMPosition.x,
        y: State.CoMPosition.y + State.InternalDisplacement.y,
        z: State.CoMPosition.z + State.InternalDisplacement.z,
      },
      obstacleUid: (obstacle && obstacle.uid) || null,
      obstacleName: (obstacle && obstacle.name) || null,
      speed: impactSpeed,
    };

    playCrashSound();
    State.CameraTrauma = 1.0;
    startCrashAnalysis();

    const speedEl = document.getElementById('crash-speed-val');
    if (speedEl) speedEl.textContent = `${Math.round(impactSpeed * 0.036)} km/h`;
    const obstacleEl = document.getElementById('crash-obstacle-val');
    if (obstacleEl) obstacleEl.textContent = (obstacle && obstacle.name) || 'an asteroid';
    fillRunStats('crash');
    showResults('crash-modal');
  }

  // --- Launch Push-Off Initiation & Departure (§1.4) ---
  function startChargingPush() {
    if (State.RunPhase !== 'LaunchPushOff') return;
    State.bIsChargingPush = true;
    const btn = Sim.silent ? null : document.getElementById('btn-charge-push');
    if (btn) btn.classList.add('charging');
  }

  function fireLaunchPush() {
    if (State.RunPhase !== 'LaunchPushOff' || !State.bIsChargingPush) return;
    if (!Sim.silent) {
      // Pre-launch state (aim + charge in hand) so the crash analysis can retry other launches
      Hist.prelaunch = takeSnapshot();
      Hist.prelaunch.launch = { aim: State.LaunchAimAngleDeg, charge: Math.max(0.18, State.LaunchPushCharge) };
    }
    State.bIsChargingPush = false;

    const btn = Sim.silent ? null : document.getElementById('btn-charge-push');
    if (btn) btn.classList.remove('charging');

    const finalCharge = Math.max(0.18, State.LaunchPushCharge);
    State.LaunchPushCharge = 0.0;

    // Departure angle calculation
    const rad = State.LaunchAimAngleDeg * (Math.PI / 180.0);
    const departureSpeed = finalCharge * Tuning.LaunchSpeedCmPerSec;

    // Apply launch velocity vector into gauntlet
    State.CoMVelocity.x = departureSpeed * Math.cos(rad);
    State.CoMVelocity.y = departureSpeed * Math.sin(rad);
    State.CoMVelocity.z = 0.0;

    State.RunPhase = 'Running';

    // Punch camera FOV & trauma
    State.TargetFOV = Math.min(106.0, 80.0 + finalCharge * 18.0);
    State.CameraTrauma = finalCharge * 0.45;

    if (Sim.silent) return;

    // Audio & Screen Effects
    playLaunchExplosionThump();
    triggerKickFlash();
    spawnKineticSparks(new THREE.Vector3(0, 0, 0), 45);

    // History starts at launch: the whole flight is the analysis window
    Hist.events.length = 0;
    Hist.samples.length = 0;
    Hist.launch = { simTimeMs: State.SimTimeMs, aim: State.LaunchAimAngleDeg, charge: finalCharge };
    recordHistorySample();

    countHintRun();
    const banner = document.getElementById('launch-guidance-banner');
    if (banner) {
      banner.classList.add('fade-out');
      setTimeout(() => {
        if (State.RunPhase !== 'LaunchPushOff') banner.classList.add('hidden');
      }, 300);
    }
  }

  function restartRun() {
    showScreen('playing');
    State.RngSeed = (Math.random() * 4294967296) >>> 0;
    State.SimTimeMs = 0.0;
    State.LastStrikeTime = 0.0;
    doomCheckAccumulator = 0;
    colossalSpawnTimer = 0;
    simAccumulator = 0;
    Input.seq = 0; Input.lastAPressTime = 0; Input.lastDPressTime = 0;
    stopCrashReplay();
    resetHistory();
    Analysis.token++;
    Analysis.status = 'idle';
    Analysis.result = null;
    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      State.RunPhase = 'Running';
      State.CoMPosition.x = 0.0; State.CoMPosition.y = 0.0; State.CoMPosition.z = 0.0;
      State.CoMVelocity.x = 400.0; State.CoMVelocity.y = 0.0; State.CoMVelocity.z = 0.0;
      State.AngularVelocityRadPerSec = 0.0;
      State.OrbitAngleRad = 0.0;
      State.HeldSpinDirection = 'None';
      State.StrikeCount = 0;
      State.ActiveFlightTimeSeconds = 0.0;
      State.KineticPushCharge = 0.0;
      State.bIsChargingKineticPush = false;
      clearPushState();
      resetPushCue();
      clearToasts();
      State.ProximityAlarm.threatRatio = 0.0;
      State.ProximityAlarm.nearestPos = null;

      const banner = document.getElementById('launch-guidance-banner');
      if (banner) banner.classList.add('hidden');
      countHintRun();

      const voidModal = document.getElementById('void-doom-modal');
      if (voidModal) voidModal.classList.add('hidden');
      const crashModal = document.getElementById('crash-modal');
      if (crashModal) crashModal.classList.add('hidden');
      const victoryModal = document.getElementById('sector-completed-modal');
      if (victoryModal) victoryModal.classList.add('hidden');

      if (colossalBeltGroup) {
        colossalIncomingAsteroids.forEach(ast => {
          colossalBeltGroup.remove(ast.mesh);
          if (ast.mesh.geometry) ast.mesh.geometry.dispose();
          if (ast.mesh.material) ast.mesh.material.dispose();
        });
      }
      colossalIncomingAsteroids = [];
      for (let i = 0; i < 5; i++) {
        spawnIncomingColossalAsteroid(4500 + i * 2800);
      }

      Hist.launch = { simTimeMs: 0, aim: 0, charge: 0 };
      recordHistorySample();
      updateHUD();
      return;
    }

    State.RunPhase = 'LaunchPushOff';
    State.LaunchAimAngleDeg = 0.0;
    State.LaunchPushCharge = 0.0;
    State.bIsChargingPush = false;
    State.bIsChargingKineticPush = false;
    State.KineticPushCharge = 0.0;
    clearPushState();
    resetPushCue();

    State.CoMPosition.x = 0.0;
    State.CoMPosition.y = 0.0;
    State.CoMPosition.z = 0.0;
    State.CoMVelocity.x = 0.0;
    State.CoMVelocity.y = 0.0;
    State.CoMVelocity.z = 0.0;
    State.AngularVelocityRadPerSec = 0.0;
    State.OrbitAngleRad = 0.0;
    State.HeldSpinDirection = 'None';
    State.StrikeCount = 0;

    // Step 6 Telemetry & Scoring Resets
    State.ActiveFlightTimeSeconds = 0.0;
    State.CleanGatesCleared = 0;
    State.CleanEvasionStreak = 0;
    State.GrazePenalties = 0;
    State.PeakSpeedCmPerSec = 3000.0;
    State.SpeedSampleSum = 0.0;
    State.SpeedSampleCount = 0;
    State.CurrentCompositeScore = 0;
    State.MilestonesPassed = {};

    Input.isAPressed = false;
    Input.isDPressed = false;
    Input.isSpacePressed = false;

    // Show launch banner
    const banner = document.getElementById('launch-guidance-banner');
    if (banner) banner.classList.remove('hidden', 'fade-out');

    // Hide modals and banners
    const voidModal = document.getElementById('void-doom-modal');
    if (voidModal) voidModal.classList.add('hidden');
    const crashModal = document.getElementById('crash-modal');
    if (crashModal) crashModal.classList.add('hidden');
    const victoryModal = document.getElementById('sector-completed-modal');
    if (victoryModal) victoryModal.classList.add('hidden');
    const sectorModal = document.getElementById('sector-select-modal');
    if (sectorModal) sectorModal.classList.add('hidden');
    clearToasts();
    State.ProximityAlarm.bIsActive = false;
    State.ProximityAlarm.threatRatio = 0.0;
    State.ProximityAlarm.nearestPos = null;

    // Reset slalom gates
    for (const target of SlalomTargets) {
      target.evaluated = false;
      target.cleared = false;
      if (target.meshGroup && target.meshGroup.children.length >= 2) {
        target.meshGroup.children[0].material.color.setHex(target.color);
        target.meshGroup.children[1].material.color.setHex(target.color);
      }
    }

    if (exitPortalGroup && exitPortalLight) {
      const currentSector = SectorCatalog[State.CurrentSectorIndex];
      exitPortalEventHorizon.material.color.setHex(currentSector.portalColor);
      exitPortalEventHorizon.material.emissive.setHex(currentSector.portalColor);
      exitPortalLight.color.setHex(currentSector.portalColor);
      exitPortalLight.intensity = 12000.0;
    }

    updateHUD();
  }

  // --- Replayable Sim: history ring buffer, snapshots, silent re-runs (PLAN_CrashEscapeHint §2) ---
  const Hist = {
    active: false,
    samples: [],     // { t, snap, body, inputs } every 1/30 s of sim time, last HIST_WINDOW_MS
    events: [],      // input events (replayable) + pushRelease markers, stamped with sim time
    prelaunch: null, // snapshot just before the launch push fired (carries .launch {aim, charge})
    launch: null,    // { simTimeMs, aim, charge }
    crash: null,     // { simTimeMs, pos, obstacleUid, obstacleName, speed } of the run that just ended
    sinceSample: 0,
  };

  function resetHistory() {
    Hist.active = true;
    Hist.samples.length = 0;
    Hist.events.length = 0;
    Hist.prelaunch = null;
    Hist.launch = null;
    Hist.crash = null;
    Hist.sinceSample = 0;
  }

  // State fields physicsTick reads or writes (scalars); vectors and references are handled below
  const SNAP_KEYS = [
    'RunPhase', 'LaunchAimAngleDeg', 'LaunchPushCharge', 'bIsChargingPush',
    'OrbitAngleRad', 'AngularVelocityRadPerSec', 'HeldSpinDirection', 'CounterRotationRad',
    'StrikeCount', 'LastStrikeTime', 'SimTimeMs', 'RngSeed',
    'ActiveFlightTimeSeconds', 'CleanGatesCleared', 'CleanEvasionStreak', 'GrazePenalties',
    'PeakSpeedCmPerSec', 'SpeedSampleSum', 'SpeedSampleCount', 'CurrentCompositeScore',
    'KineticPushCharge', 'bIsChargingKineticPush',
    'CurrentFOV', 'TargetFOV', 'CameraTrauma',
  ];

  function findObstacleByUid(uid) {
    if (!uid) return null;
    for (const o of denseObstacles) if (o.uid === uid) return o;
    for (const o of colossalIncomingAsteroids) if (o.uid === uid) return o;
    return null;
  }

  // Collider-only stand-in for a Colossal rock (no geometry, no scene node)
  function makeSimAsteroid(d) {
    return {
      uid: d.uid,
      mesh: { position: new THREE.Vector3(d.px, d.py, d.pz), rotation: { x: 0, y: 0, z: 0 }, geometry: null, material: null },
      radius: d.radius,
      massKg: d.massKg,
      velocity: new THREE.Vector3(d.vx, d.vy, d.vz),
      rotSpeed: new THREE.Vector3(0, 0, 0),
      name: d.name,
      destroyed: d.destroyed,
      rangeCm: 99999.0,
      closingSpeedCms: 0.0,
      ttiSeconds: 99.9,
      bIsCritical: false,
    };
  }

  // Deep copy of everything the sim steps read or write. Rocks are referenced by uid, never by identity.
  function takeSnapshot() {
    const s = { st: {} };
    for (const k of SNAP_KEYS) s.st[k] = State[k];
    s.com = { x: State.CoMPosition.x, y: State.CoMPosition.y, z: State.CoMPosition.z };
    s.vel = { x: State.CoMVelocity.x, y: State.CoMVelocity.y, z: State.CoMVelocity.z };
    s.disp = { y: State.InternalDisplacement.y, z: State.InternalDisplacement.z };
    s.milestones = Object.assign({}, State.MilestonesPassed);
    s.doom = Object.assign({}, State.DoomSnapshot);
    s.prox = Object.assign({}, State.ProximityAlarm);
    s.anchorUid = State.PushAnchor ? (State.PushAnchor.uid || null) : null;
    s.pushBuffer = State.PushBuffer ? { until: State.PushBuffer.until, charge: State.PushBuffer.charge } : null;
    s.maceSnap = State.MaceSnap ? { targetRad: State.MaceSnap.targetRad, remaining: State.MaceSnap.remaining } : null;
    s.input = {
      a: Input.isAPressed, d: Input.isDPressed, space: Input.isSpacePressed,
      lastA: Input.lastAPressTime, lastD: Input.lastDPressTime, seq: Input.seq,
    };
    s.gates = SlalomTargets.map(t => (t.evaluated ? 1 : 0) | (t.cleared ? 2 : 0));
    s.globals = { doomAcc: doomCheckAccumulator, spawnTimer: colossalSpawnTimer, seq: colossalAsteroidSeq };
    s.ast = colossalIncomingAsteroids.map(a => ({
      uid: a.uid, radius: a.radius, massKg: a.massKg, name: a.name, destroyed: a.destroyed,
      px: a.mesh.position.x, py: a.mesh.position.y, pz: a.mesh.position.z,
      vx: a.velocity.x, vy: a.velocity.y, vz: a.velocity.z,
    }));
    return s;
  }

  // keepAsteroids: leave colossalIncomingAsteroids alone (used when handing the live world back)
  function restoreSnapshot(s, keepAsteroids) {
    for (const k of SNAP_KEYS) State[k] = s.st[k];
    State.CoMPosition.x = s.com.x; State.CoMPosition.y = s.com.y; State.CoMPosition.z = s.com.z;
    State.CoMVelocity.x = s.vel.x; State.CoMVelocity.y = s.vel.y; State.CoMVelocity.z = s.vel.z;
    State.InternalDisplacement.y = s.disp.y; State.InternalDisplacement.z = s.disp.z;
    State.MilestonesPassed = Object.assign({}, s.milestones);
    Object.assign(State.DoomSnapshot, s.doom);
    Object.assign(State.ProximityAlarm, s.prox);
    State.PushBuffer = s.pushBuffer ? { until: s.pushBuffer.until, charge: s.pushBuffer.charge } : null;
    State.MaceSnap = s.maceSnap ? { targetRad: s.maceSnap.targetRad, remaining: s.maceSnap.remaining } : null;
    Input.isAPressed = s.input.a; Input.isDPressed = s.input.d; Input.isSpacePressed = s.input.space;
    Input.lastAPressTime = s.input.lastA; Input.lastDPressTime = s.input.lastD; Input.seq = s.input.seq;
    for (let i = 0; i < SlalomTargets.length && i < s.gates.length; i++) {
      SlalomTargets[i].evaluated = !!(s.gates[i] & 1);
      SlalomTargets[i].cleared = !!(s.gates[i] & 2);
    }
    doomCheckAccumulator = s.globals.doomAcc;
    colossalSpawnTimer = s.globals.spawnTimer;
    colossalAsteroidSeq = s.globals.seq;
    if (!keepAsteroids) colossalIncomingAsteroids = s.ast.map(makeSimAsteroid);
    State.PushAnchor = findObstacleByUid(s.anchorUid);
  }

  function recordHistorySample() {
    Hist.samples.push({
      t: State.SimTimeMs,
      snap: takeSnapshot(),
      body: {
        x: State.CoMPosition.x,
        y: State.CoMPosition.y + State.InternalDisplacement.y,
        z: State.CoMPosition.z + State.InternalDisplacement.z,
      },
      inputs: { spin: State.HeldSpinDirection, space: Input.isSpacePressed, charge: State.KineticPushCharge },
    });
    Hist.sinceSample = 0;
    const cutoff = State.SimTimeMs - HIST_WINDOW_MS;
    let drop = 0;
    while (drop < Hist.samples.length - 1 && Hist.samples[drop].t < cutoff) drop++;
    if (drop > 0) Hist.samples.splice(0, drop);
    const t0 = Hist.samples[0].t;
    let ev = 0;
    while (ev < Hist.events.length && Hist.events[ev].t < t0) ev++;
    if (ev > 0) Hist.events.splice(0, ev);
  }

  function histAfterStep() {
    if (State.RunPhase !== 'Running') return;
    if (++Hist.sinceSample >= HIST_SAMPLE_STEPS) recordHistorySample();
  }

  // Silent mode brackets: the live world (State, rocks, accumulators) is saved on entry and handed back on exit.
  function enterSilent() {
    if (Sim.silent) return;
    Sim.saved = { snap: takeSnapshot(), realAsteroids: colossalIncomingAsteroids };
    Sim.silent = true;
    Sim.outcome = null;
    Sim.override = null;
  }

  function exitSilent() {
    if (!Sim.silent) return;
    Sim.silent = false;
    colossalIncomingAsteroids = Sim.saved.realAsteroids;
    restoreSnapshot(Sim.saved.snap, true);
    Sim.saved = null;
    Sim.outcome = null;
    Sim.override = null;
    Sim.ignoreVoid = false;
    Sim.voidSeen = false;
  }

  function withSilent(fn) {
    enterSilent();
    try {
      return fn();
    } finally {
      exitSilent();
    }
  }

  // "Do nothing from here": let go of everything the player could be holding. Unlike clearHeldInput this keeps the
  // push that just fired (arm reach, anchor) intact, so the consequences of that push play out.
  function releaseHeldControls() {
    Input.isAPressed = false;
    Input.isDPressed = false;
    Input.isSpacePressed = false;
    State.HeldSpinDirection = 'None';
    State.bIsChargingPush = false;
    State.bIsChargingKineticPush = false;
    State.LaunchPushCharge = 0.0;
    State.KineticPushCharge = 0.0;
  }

  // Step the real sim from the current (restored) state to untilMs, replaying recorded input events
  // (sorted by time) as they come due. After a Sim.override fires, recorded inputs stop and everything is released.
  // onStep runs after each step; returning true ends the run.
  function simAdvance(untilMs, events, onStep) {
    let ei = 0;
    let cut = false;
    if (events) while (ei < events.length && events[ei].t < State.SimTimeMs) ei++;
    while (State.SimTimeMs < untilMs && !Sim.outcome) {
      if (events && !cut) {
        while (ei < events.length && events[ei].t <= State.SimTimeMs) {
          const ev = events[ei++];
          if (ev.type === 'input') applyInputEvent(ev.name);
        }
      }
      simStep(SIM_DT);
      if (Sim.override && Sim.override.done && !cut) {
        cut = true;
        releaseHeldControls();
      }
      if (onStep && onStep() === true) break;
    }
  }

  // Dev overlay `REPLAY CHECK`: restore the history sample from ~3 s before the crash, replay the recorded
  // inputs silently, and require the same crash at the same sim time (±1 tick) and place (±1 cm).
  function runReplayCheck() {
    const crash = Hist.crash;
    if (!crash || Hist.samples.length === 0) {
      console.warn('[REPLAY CHECK] nothing to check yet: crash into something first');
      return null;
    }
    let sample = Hist.samples[0];
    for (const s of Hist.samples) if (s.t <= crash.simTimeMs - 3000) sample = s;
    const events = Hist.events.filter(e => e.type === 'input');
    const out = withSilent(() => {
      restoreSnapshot(sample.snap);
      simAdvance(crash.simTimeMs + 500, events);
      return Sim.outcome;
    });
    const tickMs = SIM_DT * 1000.0;
    const dtMs = out ? Math.abs(out.simTimeMs - crash.simTimeMs) : Infinity;
    const distCm = out && out.pos ? Math.hypot(out.pos.x - crash.pos.x, out.pos.y - crash.pos.y, out.pos.z - crash.pos.z) : Infinity;
    const pass = !!out && out.type === 'crash' && dtMs <= tickMs + 1e-6 && distCm <= 1.0;
    const result = {
      pass, fromMs: sample.t, crashMs: crash.simTimeMs, outcome: out ? out.type : 'none',
      dtMs, distCm, events: events.length, mode: Tuning.ActiveScreenMode,
    };
    console.log(`[REPLAY CHECK] ${pass ? 'PASS' : 'FAIL'}`, result);
    Hist.lastCheck = result;
    const el = document.getElementById('replay-check-result');
    if (el) el.textContent = pass ? `PASS (${dtMs.toFixed(2)} ms, ${distCm.toFixed(3)} cm)` : `FAIL (${result.outcome}, ${dtMs.toFixed(2)} ms, ${distCm.toFixed(2)} cm)`;
    return result;
  }

  // --- Crash Debrief: "could you have made it?" (PLAN_CrashEscapeHint §3-4) ---
  // After a crash the history is rewound and the REAL physics is re-run silently with one different move.
  // Shown only on the results card, never during play (Rule 4 "Blind Math" still holds in flight).
  const Analysis = { token: 0, status: 'idle', result: null, job: null };

  const ANALYSIS_SLICE_MS = 8;
  const ANALYSIS_BUDGET_MS = 1500;
  const ANALYSIS_TAIL_MS = 2000;                       // judge each candidate this long past the original impact
  const PUSH_CHARGES = [0.15, 0.3, 0.45, 0.6, 0.75, 0.9, 1.0];
  const LAUNCH_CHARGES = [0.4, 0.7, 1.0];
  const LAUNCH_AIM_STEP_DEG = 2.5;
  const MAX_CHANGED_PUSHES = 3;                        // most recent real pushes tried with other charges

  // Rocks the body could push off right now (surface within reach), nearest first
  function listPushAnchors(maxCount) {
    const body = getPushBodyPos();
    const found = [];
    for (const obs of getPushCandidates()) {
      if (obs.destroyed) continue;
      const c = getPushObstacleCenter(obs);
      if (Math.abs(c.x - body.x) > Tuning.PushReachCm + obs.radius) continue;
      const d = c.distanceTo(body) - obs.radius;
      if (d <= Tuning.PushReachCm) found.push({ obs, d });
    }
    found.sort((a, b) => a.d - b.d);
    return found.slice(0, maxCount).map(f => f.obs);
  }

  // Distance from the body to the nearest rock surface (no allocations: runs every sim step of every candidate)
  function simClearance() {
    const bx = State.CoMPosition.x;
    const by = State.CoMPosition.y + State.InternalDisplacement.y;
    const bz = State.CoMPosition.z + State.InternalDisplacement.z;
    const list = Tuning.ActiveScreenMode === 'ColossalBelt' ? colossalIncomingAsteroids : denseObstacles;
    let m = Infinity;
    for (let i = 0; i < list.length; i++) {
      const o = list[i];
      if (o.destroyed) continue;
      const c = o.position || o.mesh.position;
      const dx = c.x - bx;
      if (dx > 2500 || dx < -2500) continue;
      const dy = c.y - by, dz = c.z - bz;
      const d = Math.sqrt(dx * dx + dy * dy + dz * dz) - o.radius;
      if (d < m) m = d;
    }
    return m;
  }

  function lastSampleAtOrBefore(samples, t) {
    let best = null;
    for (const s of samples) {
      if (s.t <= t) best = s; else break;
    }
    return best;
  }

  function describeObstacle(obs) {
    if (!obs) return 'rock';
    if (obs.massKg) return String(obs.name || 'asteroid').toLowerCase(); // Colossal rocks carry designations
    return obs.isAsteroid === false ? 'debris' : 'rock';
  }

  // Can the player hold SPACE for charge c ending at endT? Needs the time to exist after the run began and not run
  // through another real release (that earlier push would not have happened). ignoreFromMs skips the real hold being replaced.
  function holdFeasible(job, endT, charge, ignoreFromMs) {
    const startT = endT - (charge / Tuning.PushChargeRatePerSec) * 1000 - 1;
    if (startT < job.runStartMs) return false;
    for (const e of job.releaseEvents) {
      if (e.t > startT && e.t < endT && !(ignoreFromMs !== undefined && e.t >= ignoreFromMs)) return false;
    }
    return true;
  }

  // Candidate moves, most useful first. Runs inside silent mode (it restores samples to look for anchors).
  function* candidateStream(job) {
    // 1. "Change your push": same moment, other power (or none) - for the most recent real pushes
    const pushes = job.pushEvents;
    for (let i = pushes.length - 1; i >= Math.max(0, pushes.length - MAX_CHANGED_PUSHES); i--) {
      const pe = pushes[i];
      const sample = lastSampleAtOrBefore(job.samples, pe.t);
      if (!sample) continue;
      const upEv = job.inputEvents.filter(e => e.name === 'spaceUp' && e.t <= pe.t).pop();
      const downEv = upEv ? job.inputEvents.filter(e => e.name === 'spaceDown' && e.t <= upEv.t).pop() : null;
      for (const ch of [null].concat(PUSH_CHARGES)) {
        if (ch !== null && Math.abs(ch - pe.charge) < 0.04) continue;
        if (ch !== null && upEv && !holdFeasible(job, upEv.t, ch, downEv ? downEv.t : upEv.t)) continue;
        yield { family: 'change', sample, eventT: pe.t, actual: pe.charge, anchorUid: pe.anchorUid, charge: ch };
      }
    }
    // 2. "Push off X at t": every history sample with a rock in reach, latest first
    for (let si = job.samples.length - 1; si >= 0; si--) {
      const sample = job.samples[si];
      restoreSnapshot(sample.snap);
      const anchors = listPushAnchors(1); // the one-button push always picks the nearest rock in reach
      for (const a of anchors) {
        for (const ch of PUSH_CHARGES) {
          if (!holdFeasible(job, sample.t, ch)) continue;
          yield { family: 'push', sample, anchorUid: a.uid, charge: ch };
        }
      }
    }
    // 3. "Change the launch": only when the launch is inside the window and nothing above worked
    if (job.prelaunch && job.launch && job.crash.simTimeMs - job.launch.simTimeMs <= HIST_WINDOW_MS &&
        !job.successes.some(s => s.cand.family !== 'launch')) {
      const actual = job.prelaunch.launch;
      for (let aim = -Tuning.MaxAimAngleDeg; aim <= Tuning.MaxAimAngleDeg + 1e-6; aim += LAUNCH_AIM_STEP_DEG) {
        for (const ch of LAUNCH_CHARGES) {
          if (Math.abs(aim - actual.aim) < LAUNCH_AIM_STEP_DEG * 0.5 && Math.abs(ch - actual.charge) < 0.15) continue;
          yield { family: 'launch', aim, charge: ch };
        }
      }
    }
  }

  // Re-run the real sim from the candidate's moment, apply the one different move, then do nothing.
  function evalCandidate(job, c, capture) {
    Sim.outcome = null;
    Sim.override = null;
    let events = null;
    if (c.family === 'launch') {
      restoreSnapshot(job.prelaunch);
      State.LaunchAimAngleDeg = c.aim;
      State.LaunchPushCharge = c.charge;
      State.bIsChargingPush = true;
      fireLaunchPush();
    } else {
      restoreSnapshot(c.sample.snap);
      if (c.family === 'push') {
        clearHeldInput(); // spin stops, nothing is held
        const anchor = findObstacleByUid(c.anchorUid);
        if (!anchor) return null;
        executePushOff(anchor, c.charge);
      } else {
        Sim.override = { atMs: c.eventT, charge: c.charge, done: false };
        events = job.inputEvents;
      }
    }
    let minClear = Infinity;
    let stepNo = 0;
    if (capture) capture.push(captureFrame());
    simAdvance(job.tEnd, events, () => {
      const m = simClearance();
      if (m < minClear) minClear = m;
      if (capture && ++stepNo % HIST_SAMPLE_STEPS === 0) capture.push(captureFrame());
    });
    // "Good" = still on course afterwards: the existing doom lookahead must not call it lost
    Sim.voidSeen = false;
    if (!Sim.outcome && State.RunPhase === 'Running' && Tuning.ActiveScreenMode !== 'ColossalBelt') {
      evaluateBackgroundDoomLookahead();
    }
    const o = Sim.outcome;
    Sim.override = null;
    return {
      escape: !(o && o.type === 'crash'),
      good: !(o && o.type === 'crash') && !Sim.voidSeen,
      margin: minClear,
      outcome: o ? o.type : 'none',
    };
  }

  // Body position (+ every moving rock) at the current sim time, for the ghost replay
  function captureFrame() {
    const f = {
      t: State.SimTimeMs,
      x: State.CoMPosition.x,
      y: State.CoMPosition.y + State.InternalDisplacement.y,
      z: State.CoMPosition.z + State.InternalDisplacement.z,
      rocks: null,
    };
    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      f.rocks = {};
      for (const a of colossalIncomingAsteroids) f.rocks[a.uid] = [a.mesh.position.x, a.mesh.position.y, a.mesh.position.z];
    }
    return f;
  }

  function makeAnalysisJob() {
    const crash = Hist.crash;
    const obs = findObstacleByUid(crash.obstacleUid);
    const events = Hist.events.slice();
    const job = {
      token: Analysis.token,
      crash,
      samples: Hist.samples.slice(),
      inputEvents: events.filter(e => e.type === 'input'),
      releaseEvents: events.filter(e => e.type === 'input' && (e.name === 'spaceUp' || e.name === 'clear')),
      runStartMs: Hist.launch ? Hist.launch.simTimeMs : 0,
      pushEvents: events.filter(e => e.type === 'pushRelease'),
      prelaunch: Hist.prelaunch,
      launch: Hist.launch,
      tEnd: crash.simTimeMs + ANALYSIS_TAIL_MS,
      crashObs: obs,
      crashThing: obs ? describeObstacle(obs) : (crash.obstacleName ? crash.obstacleName.toLowerCase() : 'rock'),
      mode: Tuning.ActiveScreenMode,
      successes: [],
      evaluated: 0,
      finished: false,
      aborted: false,
      t0: performance.now(),
    };
    job.gen = candidateStream(job);
    return job;
  }

  function startCrashAnalysis() {
    Analysis.token++;
    Analysis.result = null;
    Analysis.replayData = null;
    Analysis.job = null;
    if (!Settings.crashTips || !Hist.crash || Hist.samples.length === 0) {
      Analysis.status = 'off';
      return;
    }
    Analysis.status = 'running';
    setCrashTip('Checking…', 'pending');
    const job = makeAnalysisJob();
    Analysis.job = job;
    scheduleAnalysisSlice(job);
  }

  function scheduleAnalysisSlice(job) {
    let fired = false;
    const go = () => {
      if (fired) return;
      fired = true;
      runAnalysisSlice(job);
    };
    requestAnimationFrame(go);
    setTimeout(go, 40); // rAF doesn't fire in hidden tabs
  }

  function runAnalysisSlice(job) {
    if (job.token !== Analysis.token) return; // retried / left the card: drop it
    try {
      runAnalysisFor(job, ANALYSIS_SLICE_MS);
    } catch (err) {
      console.error('[CRASH TIP] analysis failed', err);
      job.failed = true;
    }
    if (job.finished || job.aborted || job.failed) finishAnalysis(job);
    else scheduleAnalysisSlice(job);
  }

  // Evaluate candidates for up to sliceMs of wall time (Infinity = run to completion, used by tests)
  function runAnalysisFor(job, sliceMs) {
    const start = performance.now();
    withSilent(() => {
      Sim.ignoreVoid = true;
      while (performance.now() - start < sliceMs) {
        const nx = job.gen.next();
        if (nx.done) { job.finished = true; return; }
        job.evaluated++;
        const res = evalCandidate(job, nx.value);
        if (res && res.escape) job.successes.push(Object.assign({ cand: nx.value }, res));
        if (performance.now() - job.t0 > ANALYSIS_BUDGET_MS) { job.aborted = true; return; }
      }
    });
  }

  function roundPower(c) { return Math.round(c * 20) * 5; }
  function roundSeconds(ms) { return (Math.round(ms / 100) / 10).toFixed(1); }

  // Which way the rock lies as seen from the chase camera (looks down +X; screen right = +Z, up = +Y)
  function screenSide(from, to) {
    const dy = to.y - from.y, dz = to.z - from.z;
    if (Math.abs(dz) >= Math.abs(dy)) return dz >= 0 ? 'right' : 'left';
    return dy >= 0 ? 'above' : 'below';
  }

  // Earliest moment (most time to react: the rock has just lit up) at which the push also works over the
  // next two history samples, so the advice survives a human's ~0.07 s of timing jitter.
  // entries: successful push candidates against one rock. Returns { t, entries } (entries valid at t).
  function robustPushWindow(job, entries) {
    const sampleTimes = job.samples.map(s => s.t);
    const idx = new Map(sampleTimes.map((t, i) => [t, i]));
    const byT = new Map();
    for (const e of entries) {
      const t = e.cand.sample.t;
      if (!byT.has(t)) byT.set(t, []);
      byT.get(t).push(e);
    }
    const times = Array.from(byT.keys()).sort((a, b) => a - b);
    for (const t of times) {
      const i = idx.get(t);
      let ok = new Set(byT.get(t).map(e => e.cand.charge));
      for (let k = 1; k <= 2; k++) {
        const nt = sampleTimes[i + k];
        if (nt === undefined) break;
        const here = new Set((byT.get(nt) || []).map(e => e.cand.charge));
        ok = new Set(Array.from(ok).filter(c => here.has(c)));
      }
      if (ok.size) return { t, entries: byT.get(t).filter(e => ok.has(e.cand.charge)) };
    }
    const latest = times[times.length - 1]; // only isolated hits: fall back to the last one
    return { t: latest, entries: byT.get(latest) };
  }

  function preferGood(list) {
    const good = list.filter(s => s.good);
    return good.length ? good : list;
  }

  // First matching rule wins (§4.4). Returns { kind, text, chosen }.
  function chooseAnswer(job) {
    const crashT = job.crash.simTimeMs;
    const S = job.successes;
    const holdNote = (c) => c >= 0.6 ? ` (start holding SPACE about ${(c / Tuning.PushChargeRatePerSec).toFixed(1)} s earlier)` : '';

    // 1. Your own push caused it
    const lastPush = job.pushEvents[job.pushEvents.length - 1];
    if (lastPush) {
      const pool = preferGood(S.filter(s => s.cand.family === 'change' && s.cand.eventT === lastPush.t));
      if (pool.length) {
        const dist = (s) => Math.abs((s.cand.charge === null ? 0 : s.cand.charge) - lastPush.charge);
        pool.sort((a, b) => dist(a) - dist(b) || b.margin - a.margin);
        const best = pool[0];
        const actual = roundPower(lastPush.charge);
        const text = best.cand.charge === null
          ? `Your push (${actual}%) sent you into it. Not pushing would have cleared it.`
          : `Your push (${actual}%) sent you into it. About ${roundPower(best.cand.charge)}% would have cleared it${holdNote(best.cand.charge)}.`;
        return { kind: 'ownPush', text, chosen: best };
      }
    }

    // 2. Push off the obstacle that was hit
    const pushS = S.filter(s => s.cand.family === 'push');
    const hitPool = preferGood(pushS.filter(s => s.cand.anchorUid === job.crash.obstacleUid));
    if (hitPool.length) {
      const win = robustPushWindow(job, hitPool);
      const latest = win.t;
      const atLatest = win.entries;
      const charges = atLatest.map(s => s.cand.charge);
      const cMin = Math.min(...charges), cMax = Math.max(...charges);
      let power;
      if (cMin === cMax) power = `at about ${roundPower(cMin)}% power`;
      else if (cMax >= 1.0) power = `with ${roundPower(cMin)}% power or more`;
      else power = `with between ${roundPower(cMin)}% and ${roundPower(cMax)}% power`;
      // replay shows a charge from the middle of the working range, not an edge of it
      const byCharge = atLatest.slice().sort((a, b) => a.cand.charge - b.cand.charge);
      const best = byCharge[Math.floor((byCharge.length - 1) / 2)];
      const text = `Yes. Push off the ${job.crashThing} you hit when it lit up, ${roundSeconds(crashT - latest)} s before impact, ${power}${holdNote(cMin)}.`;
      return { kind: 'hitRock', text, chosen: best };
    }

    // 3. Push off a different obstacle (largest margin)
    const otherPool = preferGood(pushS.filter(s => s.cand.anchorUid !== job.crash.obstacleUid));
    if (otherPool.length) {
      const top = otherPool.slice().sort((a, b) => b.margin - a.margin)[0];
      const win = robustPushWindow(job, otherPool.filter(s => s.cand.anchorUid === top.cand.anchorUid));
      const best = win.entries.slice().sort((a, b) => b.margin - a.margin)[0];
      const sample = best.cand.sample;
      const obs = (restoreSnapshot(sample.snap), findObstacleByUid(best.cand.anchorUid));
      const side = obs ? screenSide(sample.body, getPushObstacleCenter(obs)) : 'side';
      const text = `Yes. Push off the ${describeObstacle(obs)} on your ${side}, ${roundSeconds(crashT - sample.t)} s before impact, at about ${roundPower(best.cand.charge)}% power${holdNote(best.cand.charge)}.`;
      return { kind: 'otherRock', text, chosen: best };
    }

    // 4. Launch differently
    const launchPool = preferGood(S.filter(s => s.cand.family === 'launch'));
    if (launchPool.length && job.prelaunch) {
      const actual = job.prelaunch.launch;
      const cost = (s) => Math.abs(s.cand.aim - actual.aim) + (Math.abs(s.cand.charge - actual.charge) > 0.15 ? 100 : 0);
      launchPool.sort((a, b) => cost(a) - cost(b) || b.margin - a.margin);
      const best = launchPool[0];
      const aimTxt = (a) => `${a > 0 ? '+' : ''}${a.toFixed(1)}`;
      let text = `Launch at ${aimTxt(best.cand.aim)}° instead of ${aimTxt(actual.aim)}° to miss it.`;
      if (Math.abs(best.cand.charge - actual.charge) > 0.15) {
        text = `Launch at ${aimTxt(best.cand.aim)}° with ${roundPower(best.cand.charge)}% power instead of ${aimTxt(actual.aim)}° at ${roundPower(actual.charge)}% to miss it.`;
      }
      return { kind: 'launch', text, chosen: best };
    }

    // 5. Nothing worked
    const covered = Math.max(1, Math.round((crashT - job.samples[0].t) / 1000));
    return {
      kind: 'none',
      text: `No escape in the last ${covered} s. Your line was set before that — try a different route earlier.`,
      chosen: null,
    };
  }

  function finishAnalysis(job) {
    job.ms = performance.now() - job.t0;
    if (job.failed || (job.aborted && job.successes.length === 0)) {
      Analysis.status = 'failed';
      Analysis.result = { kind: 'failed', text: "Couldn't check this one", chosen: null };
      setCrashTip(Analysis.result.text, 'failed');
    } else {
      const ans = withSilent(() => chooseAnswer(job));
      Analysis.status = 'done';
      Analysis.result = ans;
      Analysis.replayData = null;
      if (ans.chosen) {
        try {
          Analysis.replayData = buildReplayData(job, ans);
        } catch (err) {
          console.error('[CRASH TIP] replay data failed', err);
        }
      }
      setCrashTip(ans.text, ans.kind === 'none' ? 'none' : 'yes');
    }
    console.log(`[CRASH TIP] ${Analysis.result.kind}: ${Analysis.result.text} (${job.evaluated} candidates, ${job.successes.length} escapes, ${job.ms.toFixed(0)} ms${job.aborted ? ', aborted' : ''})`);
  }

  // Run the whole analysis synchronously (tests / dev console)
  function analyzeCrashSync() {
    if (!Hist.crash) return null;
    Analysis.token++;
    const job = makeAnalysisJob();
    Analysis.job = job;
    runAnalysisFor(job, Infinity);
    job.ms = performance.now() - job.t0;
    const ans = withSilent(() => chooseAnswer(job));
    Analysis.status = 'done';
    Analysis.result = ans;
    Analysis.replayData = null;
    if (ans.chosen) Analysis.replayData = buildReplayData(job, ans);
    return { ans, job };
  }

  // --- Ghost replay: what you did (red) vs what would have worked (cyan) (§5) ---
  const Replay = { active: false, camPos: { x: 0, y: 0, z: 0 }, camVel: { x: 0, y: 0, z: 0 } };
  const REPLAY_PRE_MS = 1500;
  const REPLAY_SLOW_AFTER_ACTION_MS = 500;
  const REPLAY_AFTER_IMPACT_MS = 1000;
  const REPLAY_SLOW_FACTOR = 0.5;
  const REPLAY_TRAIL_POINTS = 70;

  // Both paths at 30 Hz + the moving rocks, all on the sim clock. The cyan path is the real history up to the
  // action, then a silent re-run of the chosen move (the same physics the answer came from).
  function buildReplayData(job, ans) {
    const cand = ans.chosen.cand;
    const crash = job.crash;
    const frames = [];
    withSilent(() => {
      Sim.ignoreVoid = true;
      evalCandidate(job, cand, frames);
    });
    const actual = job.samples.map(s => ({ t: s.t, x: s.body.x, y: s.body.y, z: s.body.z }));
    actual.push({ t: crash.simTimeMs, x: crash.body.x, y: crash.body.y, z: crash.body.z });

    let actionT, label, anchorUid = null;
    if (cand.family === 'launch') {
      actionT = job.launch.simTimeMs;
      const act = job.prelaunch.launch;
      label = `LAUNCH ${cand.aim >= 0 ? '+' : ''}${cand.aim.toFixed(0)}°`;
      if (Math.abs(cand.charge - act.charge) > 0.15) label += ` ${roundPower(cand.charge)}%`;
    } else {
      actionT = cand.family === 'change' ? cand.eventT : cand.sample.t;
      anchorUid = cand.anchorUid;
      label = cand.charge === null ? "DON'T PUSH" : `PUSH ${roundPower(cand.charge)}%`;
    }

    const cyan = actual.filter(pt => pt.t <= actionT);
    for (const f of frames) if (f.t > actionT) cyan.push({ t: f.t, x: f.x, y: f.y, z: f.z });
    if (cyan.length < 2) cyan.push({ t: actionT + 1, x: actual[actual.length - 1].x, y: actual[actual.length - 1].y, z: actual[actual.length - 1].z });

    // Moving rocks: history samples up to the action, then the re-run
    const rocks = [];
    for (const s of job.samples) {
      if (s.t > actionT || !s.snap.ast.length) continue;
      const m = {};
      for (const a of s.snap.ast) m[a.uid] = [a.px, a.py, a.pz];
      rocks.push({ t: s.t, m });
    }
    for (const f of frames) if (f.t > actionT && f.rocks) rocks.push({ t: f.t, m: f.rocks });

    return {
      actual, cyan, rocks, actionT, label, anchorUid,
      crashT: crash.simTimeMs,
      charge: cand.family === 'launch' ? cand.charge : cand.charge,
      isLaunch: cand.family === 'launch',
    };
  }

  function pathAt(path, t, out) {
    const n = path.length;
    let a, b, k = 0;
    if (t <= path[0].t) { a = b = path[0]; }
    else if (t >= path[n - 1].t) { a = b = path[n - 1]; }
    else {
      let i = path._i || 0;
      if (path[i].t > t) i = 0;
      while (i < n - 2 && path[i + 1].t <= t) i++;
      path._i = i;
      a = path[i]; b = path[i + 1];
      k = (t - a.t) / Math.max(1e-6, b.t - a.t);
    }
    out.x = a.x + (b.x - a.x) * k;
    out.y = a.y + (b.y - a.y) * k;
    out.z = a.z + (b.z - a.z) * k;
    return out;
  }

  function makeGhost(colorHex) {
    const group = new THREE.Group();
    const core = new THREE.Mesh(
      new THREE.SphereGeometry(42, 20, 14),
      new THREE.MeshBasicMaterial({ color: colorHex, transparent: true, opacity: 0.6, depthWrite: false })
    );
    const shell = new THREE.Mesh(
      new THREE.SphereGeometry(70, 20, 14),
      new THREE.MeshBasicMaterial({ color: colorHex, transparent: true, opacity: 0.18, depthWrite: false, blending: THREE.AdditiveBlending })
    );
    group.add(core);
    group.add(shell);
    const positions = new Float32Array(REPLAY_TRAIL_POINTS * 3);
    const colors = new Float32Array(REPLAY_TRAIL_POINTS * 3);
    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    geo.setAttribute('color', new THREE.BufferAttribute(colors, 3));
    geo.setDrawRange(0, 0);
    const trail = new THREE.Line(geo, new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, blending: THREE.AdditiveBlending, depthWrite: false }));
    trail.frustumCulled = false;
    scene.add(group);
    scene.add(trail);
    const c = new THREE.Color(colorHex);
    return { group, core, shell, trail, positions, colors, base: [c.r, c.g, c.b] };
  }

  function disposeGhost(g) {
    if (!g) return;
    scene.remove(g.group);
    scene.remove(g.trail);
    g.core.geometry.dispose(); g.core.material.dispose();
    g.shell.geometry.dispose(); g.shell.material.dispose();
    g.trail.geometry.dispose(); g.trail.material.dispose();
  }

  const REPLAY_STEP_MS = 1000 / 30;
  const _rp = { x: 0, y: 0, z: 0 };

  function updateGhost(g, path, t, tMax) {
    const tt = Math.min(t, tMax);
    pathAt(path, tt, _rp);
    g.group.position.set(_rp.x, _rp.y, _rp.z);
    let n = 0;
    for (let i = 0; i < REPLAY_TRAIL_POINTS; i++) {
      const ti = tt - i * REPLAY_STEP_MS;
      if (ti < path[0].t) break;
      pathAt(path, ti, _rp);
      const o = i * 3;
      g.positions[o] = _rp.x; g.positions[o + 1] = _rp.y; g.positions[o + 2] = _rp.z;
      const fade = 1 - i / REPLAY_TRAIL_POINTS;
      g.colors[o] = g.base[0] * fade; g.colors[o + 1] = g.base[1] * fade; g.colors[o + 2] = g.base[2] * fade;
      n++;
    }
    g.trail.geometry.attributes.position.needsUpdate = true;
    g.trail.geometry.attributes.color.needsUpdate = true;
    g.trail.geometry.setDrawRange(0, n);
  }

  function canWatchReplay() {
    const modal = document.getElementById('crash-modal');
    return !Replay.active && UI.screen === 'results' && UI.resultsModal === 'crash-modal' &&
      !!modal && !modal.classList.contains('hidden') && Analysis.status === 'done' && !!Analysis.replayData;
  }

  function startCrashReplay() {
    if (!canWatchReplay()) return false;
    const d = Analysis.replayData;
    const R = Replay;
    R.data = d;
    R.reduced = !!(window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches);
    R.t0 = Math.max(d.actual[0].t, d.actionT - REPLAY_PRE_MS);
    R.tEnd = d.crashT + REPLAY_AFTER_IMPACT_MS;
    R.t = R.t0;
    R.first = true;
    R.flash = null;
    R.red = makeGhost(0xff3b5c);
    R.cyan = makeGhost(0x00f0ff);
    R.prevCyan = null;
    R.anchorObs = d.anchorUid ? findObstacleByUid(d.anchorUid) : null;
    R.rockMeshes = [];
    if (d.rocks.length) {
      for (const a of colossalIncomingAsteroids) {
        if (a.uid) R.rockMeshes.push({ ast: a, saved: a.mesh.position.clone() });
      }
    }
    d.actual._i = 0; d.cyan._i = 0;
    const modal = document.getElementById('crash-modal');
    if (modal) modal.classList.add('hidden');
    const hud = document.getElementById('replay-hud');
    if (hud) hud.classList.remove('hidden');
    const label = document.getElementById('replay-label');
    if (label) { label.textContent = d.label; label.classList.add('hidden'); }
    R.active = true;
    clearToasts();
    return true;
  }

  function updateReplay(dt) {
    const R = Replay, d = R.data;
    const slow = !R.reduced && R.t < d.actionT + REPLAY_SLOW_AFTER_ACTION_MS;
    R.t += dt * 1000 * (slow ? REPLAY_SLOW_FACTOR : 1);
    if (R.t >= R.tEnd) { stopCrashReplay(); return; }

    updateGhost(R.red, d.actual, R.t, d.crashT);
    updateGhost(R.cyan, d.cyan, R.t, R.tEnd);

    // chase camera follows the suggested (cyan) ghost
    pathAt(d.cyan, R.t, _rp);
    const prev = R.prevCyan;
    const simDt = prev ? Math.max(1, R.t - prev.t) / 1000 : 0;
    if (prev && simDt > 0) {
      R.camVel.x = (_rp.x - prev.x) / simDt; R.camVel.y = (_rp.y - prev.y) / simDt; R.camVel.z = (_rp.z - prev.z) / simDt;
    } else {
      R.camVel.x = 3000; R.camVel.y = 0; R.camVel.z = 0;
    }
    R.prevCyan = { t: R.t, x: _rp.x, y: _rp.y, z: _rp.z };
    R.camPos.x = _rp.x; R.camPos.y = _rp.y; R.camPos.z = _rp.z;
    if (R.first) {
      R.first = false;
      camera.position.set(_rp.x - State.CameraDistance, _rp.y * 0.45, _rp.z * 0.45 + 120);
    }

    // moving rocks replay from the recorded tracks
    if (R.rockMeshes.length) {
      const tr = d.rocks;
      let i = 0;
      while (i < tr.length - 1 && tr[i + 1].t <= R.t) i++;
      const a = tr[i], b = tr[Math.min(tr.length - 1, i + 1)];
      const k = b.t > a.t ? Math.min(1, Math.max(0, (R.t - a.t) / (b.t - a.t))) : 0;
      for (const rm of R.rockMeshes) {
        const pa = a.m[rm.ast.uid];
        if (!pa) continue;
        const pb = b.m[rm.ast.uid] || pa;
        rm.ast.mesh.position.set(pa[0] + (pb[0] - pa[0]) * k, pa[1] + (pb[1] - pa[1]) * k, pa[2] + (pb[2] - pa[2]) * k);
      }
    }

    // impact flash where the red ghost ends
    if (!R.flash && R.t >= d.crashT) {
      const cp = d.actual[d.actual.length - 1];
      const mesh = new THREE.Mesh(
        new THREE.SphereGeometry(60, 16, 12),
        new THREE.MeshBasicMaterial({ color: 0xff6b4a, transparent: true, opacity: 0.95, blending: THREE.AdditiveBlending, depthWrite: false })
      );
      mesh.position.set(cp.x, cp.y, cp.z);
      scene.add(mesh);
      R.flash = { mesh, age: 0 };
      spawnKineticSparks(new THREE.Vector3(cp.x, cp.y, cp.z), 40);
    }
    if (R.flash) {
      R.flash.age += dt;
      const s = 1 + R.flash.age * 7;
      R.flash.mesh.scale.setScalar(s);
      R.flash.mesh.material.opacity = Math.max(0, 0.95 - R.flash.age * 1.6);
    }
  }

  // Label + rim pulse at the action moment (after the camera has moved)
  function updateReplayOverlay() {
    const R = Replay, d = R.data;
    const label = document.getElementById('replay-label');
    const inWindow = R.t >= d.actionT - 250 && R.t <= d.actionT + 1400;
    if (R.anchorObs) {
      const pulse = R.t >= d.actionT - 400 && R.t <= d.actionT + 250;
      if (pulse) setPushAnchorHighlight(R.anchorObs, true, 0.35 + 0.65 * Math.abs(Math.sin(performance.now() * 0.012)), false);
      else setPushAnchorHighlight(R.anchorObs, false, 0);
    }
    if (!label) return;
    if (!inWindow) { label.classList.add('hidden'); return; }
    camera.updateMatrixWorld();
    const wp = R.anchorObs ? getPushObstacleCenter(R.anchorObs) : new THREE.Vector3(R.camPos.x, R.camPos.y, R.camPos.z);
    const v = new THREE.Vector3(wp.x, wp.y, wp.z).project(camera);
    if (v.z > 1.0) { label.classList.add('hidden'); return; }
    label.style.left = `${(v.x * 0.5 + 0.5) * window.innerWidth}px`;
    label.style.top = `${(-v.y * 0.5 + 0.5) * window.innerHeight}px`;
    label.classList.remove('hidden');
  }

  // Skip or end: ghosts, trail, label and rim pulse go away, rocks go back, the crash card returns with RETRY focused
  function stopCrashReplay() {
    const R = Replay;
    if (!R.active) return;
    R.active = false;
    disposeGhost(R.red); disposeGhost(R.cyan);
    R.red = R.cyan = null;
    if (R.flash) {
      scene.remove(R.flash.mesh);
      R.flash.mesh.geometry.dispose(); R.flash.mesh.material.dispose();
      R.flash = null;
    }
    if (R.anchorObs) setPushAnchorHighlight(R.anchorObs, false, 0);
    for (const rm of R.rockMeshes || []) rm.ast.mesh.position.copy(rm.saved);
    R.rockMeshes = [];
    const hud = document.getElementById('replay-hud');
    if (hud) hud.classList.add('hidden');
    const label = document.getElementById('replay-label');
    if (label) label.classList.add('hidden');
    // camera back on the crash scene
    camera.position.set(State.CoMPosition.x - State.CameraDistance, State.CoMPosition.y * 0.45, State.CoMPosition.z * 0.45 + 120);
    simAccumulator = 0;
    lastTime = performance.now();
    if (UI.screen === 'results' && UI.resultsModal === 'crash-modal') {
      const modal = document.getElementById('crash-modal');
      if (modal) modal.classList.remove('hidden');
      UI.menuIndex = 0;
      selectMenuItem(0);
    }
  }

  function setCrashTip(text, variant) {
    const block = document.getElementById('crash-tip');
    const el = document.getElementById('crash-tip-text');
    if (block) block.classList.toggle('hidden', !Settings.crashTips);
    if (!el) return;
    el.textContent = text;
    el.className = `crash-tip-text ${variant || ''}`;
  }

  // --- Fixed 120Hz Physics Tick ---
  let doomCheckAccumulator = 0;

  function physicsTick(dt) {
    if (dt <= 0) return;
    State.SimTimeMs += dt * 1000.0;

    // 0. Launch Push-Off Phase State Logic (§1.4)
    if (State.RunPhase === 'LaunchPushOff') {
      if (Input.isAPressed) {
        State.LaunchAimAngleDeg = Math.max(-Tuning.MaxAimAngleDeg, State.LaunchAimAngleDeg - Tuning.AimAngularSpeedDegPerSec * dt);
      }
      if (Input.isDPressed) {
        State.LaunchAimAngleDeg = Math.min(Tuning.MaxAimAngleDeg, State.LaunchAimAngleDeg + Tuning.AimAngularSpeedDegPerSec * dt);
      }

      if (State.bIsChargingPush) {
        State.LaunchPushCharge = Math.min(1.0, State.LaunchPushCharge + dt * Tuning.ChargeRatePerSec);
        playLaunchChargeSound(State.LaunchPushCharge);
        State.TargetFOV = 80.0 - State.LaunchPushCharge * 6.0;
      } else {
        State.TargetFOV = 80.0;
      }

      State.CurrentFOV += (State.TargetFOV - State.CurrentFOV) * Math.min(1.0, dt * 10.0);
      return;
    }

    if (State.RunPhase !== 'Running') return;

    // Step 8: Kinetic Push Charging Accumulator
    if (State.bIsChargingKineticPush) {
      State.KineticPushCharge = Math.min(1.0, State.KineticPushCharge + dt * Tuning.PushChargeRatePerSec);
      playKineticChargeSound(State.KineticPushCharge);
    }

    const accelRad = Tuning.SpinAccelerationDegPerSec2 * (Math.PI / 180);
    const maxSpeedRad = Tuning.MaxAngularSpeedDegPerSec * (Math.PI / 180);

    // 1. Angular Acceleration
    if (State.HeldSpinDirection === 'CounterClockwise') {
      if (State.AngularVelocityRadPerSec < 0) {
        State.AngularVelocityRadPerSec += accelRad * 2.0 * dt;
      } else {
        State.AngularVelocityRadPerSec = Math.min(State.AngularVelocityRadPerSec + accelRad * dt, maxSpeedRad);
      }
    } else if (State.HeldSpinDirection === 'Clockwise') {
      if (State.AngularVelocityRadPerSec > 0) {
        State.AngularVelocityRadPerSec -= accelRad * 2.0 * dt;
      } else {
        State.AngularVelocityRadPerSec = Math.max(State.AngularVelocityRadPerSec - accelRad * dt, -maxSpeedRad);
      }
    } else {
      State.AngularVelocityRadPerSec = 0.0;
    }

    // 2. Continuous Audio Pitch Scaling (Metasound mirror)
    updateContinuousSpinAudio(State.AngularVelocityRadPerSec);

    // 2.5 One-button push-off: anchor tracking, release buffer, arm reach
    updatePushOff(dt);

    // 3. Angle Integration
    if (Math.abs(State.AngularVelocityRadPerSec) > 0.0001) {
      State.OrbitAngleRad += State.AngularVelocityRadPerSec * dt;
      State.OrbitAngleRad = State.OrbitAngleRad % (2 * Math.PI);
      if (State.OrbitAngleRad < 0) State.OrbitAngleRad += 2 * Math.PI;
    }

    // 4. Rule 1: Two-Body Internal Displacement Solver (The Dodge)
    const totalMass = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const cosT = Math.cos(State.OrbitAngleRad);
    const sinT = Math.sin(State.OrbitAngleRad);
    const appendageY = cosT * Tuning.TetherLengthCm;
    const appendageZ = sinT * Tuning.TetherLengthCm;

    // Astronaut body displaced opposite to mace head (1:4 default)
    State.InternalDisplacement.y = -(Tuning.MaceMassKg / totalMass) * appendageY;
    State.InternalDisplacement.z = -(Tuning.MaceMassKg / totalMass) * appendageZ;

    // Total Angular Momentum Conservation: Counter-Rotation
    const Im = Tuning.MaceMassKg * Math.pow(Tuning.TetherLengthCm, 2);
    const Is = 0.5 * Tuning.AstronautMassKg * Math.pow(45.0, 2);
    const rotRatio = Math.min(Im / Is, 3.5);
    State.CounterRotationRad = -rotRatio * State.OrbitAngleRad;

    // 6. Update System Center of Mass World Position
    State.CoMPosition.x += State.CoMVelocity.x * dt;
    State.CoMPosition.y += State.CoMVelocity.y * dt;
    State.CoMPosition.z += State.CoMVelocity.z * dt;

    // 7. Collision Evaluations (Appendage Deflections & Body Crashes)
    evaluateContinuousCollisions();

    // 7.5. Step 7 Proximity Radar Alarm & Engine Hum Audio
    let minObsDistCm = 999999.0;
    let nearestObsPos = null;
    for (const obs of denseObstacles) {
      if (obs.destroyed) continue;
      const dx = obs.position.x - State.CoMPosition.x;
      if (dx < -200 || dx > 3500) continue;
      const dist = Math.hypot(
        dx,
        obs.position.y - (State.CoMPosition.y + State.InternalDisplacement.y),
        obs.position.z - (State.CoMPosition.z + State.InternalDisplacement.z)
      ) - obs.radius;
      if (dist < minObsDistCm) {
        minObsDistCm = Math.max(0, dist);
        nearestObsPos = obs.position;
      }
    }
    updateProximityRadarAlarm(minObsDistCm, State.SimTimeMs / 1000.0, nearestObsPos);
    updateContinuousEngineHumAudio(State.CoMVelocity.x, Math.hypot(State.CoMVelocity.y, State.CoMVelocity.z));

    // 8. Slalom Gate Evasions
    evaluateSlalomCrossings();

    // 9. Step 6: Flight Telemetry & Live Composite Scoring
    State.ActiveFlightTimeSeconds += dt;
    const currentTotalSpeed = Math.hypot(
      State.CoMVelocity.x,
      State.CoMVelocity.y,
      State.CoMVelocity.z
    );
    if (currentTotalSpeed > State.PeakSpeedCmPerSec) {
      State.PeakSpeedCmPerSec = currentTotalSpeed;
    }
    State.SpeedSampleSum += currentTotalSpeed;
    State.SpeedSampleCount++;

    const currentSector = SectorCatalog[State.CurrentSectorIndex];
    const distM = State.CoMPosition.x * 0.01;
    const avgSpeed = State.SpeedSampleCount > 0 ? (State.SpeedSampleSum / State.SpeedSampleCount) : 3000.0;
    State.CurrentCompositeScore = Math.max(0, Math.round(
      (distM * 10) +
      (avgSpeed * 0.2) +
      (State.CleanGatesCleared * 500) +
      (State.StrikeCount * 300) -
      (State.ActiveFlightTimeSeconds * 25) -
      (State.GrazePenalties * 200)
    ));

    // 10. Step 6: Celestial Milestone Checkpoints
    evaluateMilestoneCrossings();

    // 11. Step 6: Jump Portal Terminus Breach
    if (currentSector && State.CoMPosition.x >= currentSector.corridorLengthCm) {
      completeSectorRun();
      return;
    }

    // 12. Background 13s–40s Void Doom Detection (Evaluates every 120ms)
    doomCheckAccumulator += dt;
    if (doomCheckAccumulator >= 0.12) {
      doomCheckAccumulator = 0;
      // Colossal Belt has no corridor anchors to look ahead at; the doom verdict is corridor-only
      if (Tuning.ActiveScreenMode !== 'ColossalBelt') evaluateBackgroundDoomLookahead();
    }

    // 13. Dynamic Camera FOV Shifter
    const forwardSpeedRatio = Math.max(0, (State.CoMVelocity.x - 3000) / 1500);
    const spinSpeedRatio = Math.abs(State.AngularVelocityRadPerSec) / maxSpeedRad;
    const restingFOV = 80.0 + forwardSpeedRatio * 6.0 + spinSpeedRatio * 4.0;
    State.TargetFOV += (restingFOV - State.TargetFOV) * Math.min(1.0, dt * 2.5);
    State.CurrentFOV += (State.TargetFOV - State.CurrentFOV) * Math.min(1.0, dt * 10.0);
  }

  // --- Collision Evaluation: Full Mechanical Appendage Push & Deflection ---
  function evaluateContinuousCollisions() {
    const totalMass = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const cosT = Math.cos(State.OrbitAngleRad);
    const sinT = Math.sin(State.OrbitAngleRad);

    const maceRelY = (Tuning.AstronautMassKg / totalMass) * cosT * Tuning.TetherLengthCm;
    const maceRelZ = (Tuning.AstronautMassKg / totalMass) * sinT * Tuning.TetherLengthCm;
    const maceWorldPos = new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + maceRelY,
      State.CoMPosition.z + maceRelZ
    );

    const astronautWorldPos = new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );

    const armVector = new THREE.Vector3().subVectors(maceWorldPos, astronautWorldPos);
    const armLengthSq = armVector.lengthSq();
    const astronautBodyRadius = 40.0;

    for (const obs of denseObstacles) {
      if (obs.destroyed) continue;

      const dx = Math.abs(obs.position.x - State.CoMPosition.x);
      if (dx > 500.0) continue;

      let bDeflectedByAppendage = false;

      // 1. Line-segment capsule sweep for the ENTIRE MECHANICAL APPENDAGE
      if (armLengthSq > 0.001) {
        const toObs = new THREE.Vector3().subVectors(obs.position, astronautWorldPos);
        const u = Math.max(0.0, Math.min(1.0, toObs.dot(armVector) / armLengthSq));
        const closestPointOnArm = astronautWorldPos.clone().add(armVector.clone().multiplyScalar(u));

        const armThickness = THREE.MathUtils.lerp(18.0, 42.0, u);
        const distToObstacle = closestPointOnArm.distanceTo(obs.position);

        if (distToObstacle <= (obs.radius + armThickness)) {
          bDeflectedByAppendage = true;
          const normal = new THREE.Vector3().subVectors(closestPointOnArm, obs.position).normalize();
          executeExternalStrike(obs, closestPointOnArm, normal, u);
        }
      }

      // 2. Fatal Kinetic Crash check against fragile astronaut body
      const timeSinceLastDeflection = (State.SimTimeMs - State.LastStrikeTime) / 1000.0;
      if (!bDeflectedByAppendage && timeSinceLastDeflection > 0.12) {
        const distToBody = astronautWorldPos.distanceTo(obs.position);
        if (distToBody <= (obs.radius + astronautBodyRadius)) {
          const totalSpeed = Math.hypot(
            State.CoMVelocity.x,
            State.CoMVelocity.y,
            State.CoMVelocity.z
          );
          triggerKineticCrash(obs, totalSpeed);
          return;
        }
      }
    }
  }

  // --- Slalom Crossings ---
  function evaluateSlalomCrossings() {
    const astronautWorldPos = new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );

    for (const target of SlalomTargets) {
      if (target.evaluated) continue;

      if (State.CoMPosition.x >= target.forwardX) {
        target.evaluated = true;
        const gateCenter = new THREE.Vector3(target.forwardX, target.targetY, target.targetZ);
        const distFromCenter = Math.hypot(astronautWorldPos.y - gateCenter.y, astronautWorldPos.z - gateCenter.z);

        if (distFromCenter <= target.radius) {
          target.cleared = true;
          State.CleanGatesCleared++;
          State.CleanEvasionStreak++;
          playClearChime();
          showSlalomBanner(true, target, target.radius - distFromCenter);
        } else {
          target.cleared = false;
          State.CleanEvasionStreak = 0;
          State.GrazePenalties++;
          playGrazeWarning();
          showSlalomBanner(false, target, distFromCenter - target.radius);
        }
      }
    }
  }

  const CLEAN_GATE_POINTS = 500; // mirrors the (CleanGatesCleared * 500) term of the score formula

  function showSlalomBanner(isSuccess) {
    if (isSuccess) {
      showToast(`CLEAN +${CLEAN_GATE_POINTS}`, 'cyan');
    } else {
      showToast('MISSED GATE', 'rose');
    }
  }

  // --- Step 6: Celestial Milestone Checkpoints ---
  function evaluateMilestoneCrossings() {
    const currentSector = SectorCatalog[State.CurrentSectorIndex];
    if (!currentSector || !currentSector.milestones) return;

    for (const milestone of currentSector.milestones) {
      if (State.MilestonesPassed[milestone.index]) continue;

      if (State.CoMPosition.x >= milestone.distCm) {
        State.MilestonesPassed[milestone.index] = true;
        playMilestoneChime();
        showMilestoneBanner(milestone.title, milestone.bonus);
      }
    }
  }

  function showMilestoneBanner(title, bonus) {
    showToast(`${title} +${bonus}`, 'gold');
  }

  // --- Step 6: Sector Run Victory & Jump Portal Completion ---
  function completeSectorRun() {
    if (State.RunPhase !== 'Running') return;
    State.RunPhase = 'Completed';
    if (Sim.silent) {
      Sim.outcome = { type: 'complete', simTimeMs: State.SimTimeMs, obstacle: null };
      return;
    }

    const currentSector = SectorCatalog[State.CurrentSectorIndex];
    const totalDistMeters = currentSector.corridorLengthCm * 0.01;
    const avgSpeedCms = State.SpeedSampleCount > 0 ? (State.SpeedSampleSum / State.SpeedSampleCount) : 3000.0;
    const peakSpeedCms = State.PeakSpeedCmPerSec;
    const timeSec = State.ActiveFlightTimeSeconds;

    const finalScore = Math.max(0, Math.round(
      (totalDistMeters * 10) +
      (avgSpeedCms * 0.2) +
      (State.CleanGatesCleared * 500) +
      (State.StrikeCount * 300) +
      2500 -
      (timeSec * 25) -
      (State.GrazePenalties * 200)
    ));

    State.CurrentCompositeScore = finalScore;

    let grade = 'C';
    let gradeTitle = 'NEWTONIAN NOVICE';
    let gradeDesc = 'Surviving with rough trajectory adjustments.';
    let gradeClass = 'grade-c';

    if (finalScore >= 16000) {
      grade = 'S+';
      gradeTitle = 'KINETIC ASCENDANT';
      gradeDesc = 'Flawless spatial flow, maximum velocity, zero boundary grazes.';
      gradeClass = 'grade-s-plus';
    } else if (finalScore >= 13500) {
      grade = 'S';
      gradeTitle = 'ORBITAL MAESTRO';
      gradeDesc = 'Flawless Newtonian conservation & high-speed momentum control.';
      gradeClass = 'grade-s';
    } else if (finalScore >= 10500) {
      grade = 'A';
      gradeTitle = 'CLEAN NAVIGATOR';
      gradeDesc = 'Solid kinetic trajectory, aggressive mace strikes and clean line.';
      gradeClass = 'grade-a';
    } else if (finalScore >= 7500) {
      grade = 'B';
      gradeTitle = 'DEBRIS SURVIVOR';
      gradeDesc = 'Reached the jump portal despite boundary friction.';
      gradeClass = 'grade-b';
    }

    const runResult = {
      sectorIndex: State.CurrentSectorIndex,
      compositeScore: finalScore,
      grade: grade,
      gradeTitle: gradeTitle,
      timeSeconds: timeSec,
      avgSpeedCms: avgSpeedCms,
      topSpeedCms: peakSpeedCms,
      cleanGates: State.CleanGatesCleared,
      totalGates: SlalomTargets.length,
      strikes: State.StrikeCount,
      grazes: State.GrazePenalties,
      bCompleted: true,
    };

    const { isNewPB, profile } = SaveGame.recordRun(runResult);

    // Audio & Visual Fanfare
    playPortalWarpFanfare();
    triggerWarpJumpFlash(currentSector.portalThemeColorStr);
    State.CameraTrauma = 0.8;

    // Populate Victory Modal UI
    const modal = document.getElementById('sector-completed-modal');
    const circle = document.getElementById('victory-grade-circle');
    const letter = document.getElementById('victory-grade-letter');
    const title = document.getElementById('victory-grade-title');

    if (circle && letter && title) {
      circle.className = `grade-circle ${gradeClass}`;
      letter.textContent = grade;
      title.textContent = gradeTitle;
    }

    const timeVal = document.getElementById('victory-time-val');
    if (timeVal) timeVal.textContent = `${timeSec.toFixed(1)}s (Par ${currentSector.parTimeSeconds.toFixed(1)}s)`;

    const gatesVal = document.getElementById('victory-gates-val');
    if (gatesVal) gatesVal.textContent = `${State.CleanGatesCleared} / ${SlalomTargets.length}`;

    const scoreVal = document.getElementById('victory-score-val');
    if (scoreVal) scoreVal.textContent = finalScore.toLocaleString();

    const pbTag = document.getElementById('leaderboard-pb-tag');
    if (pbTag) pbTag.classList.toggle('hidden', !isNewPB);

    // Populate Leaderboard table
    const tbody = document.getElementById('leaderboard-tbody');
    if (tbody) {
      tbody.innerHTML = '';
      const entries = profile.leaderboards[State.CurrentSectorIndex] || [];
      entries.slice(0, 5).forEach((e, i) => {
        const tr = document.createElement('tr');
        if (i === 0) tr.className = 'rank-1';
        tr.innerHTML = `
          <td>${i + 1}</td>
          <td>${e.score.toLocaleString()}</td>
          <td><span class="badge ${e.grade.startsWith('S') ? 'badge-amber' : ''}">${e.grade}</span></td>
          <td>${e.time}</td>
        `;
        tbody.appendChild(tr);
      });
    }

    // Configure Next Sector button
    const nextBtn = document.getElementById('btn-next-sector');
    if (nextBtn) {
      if (State.CurrentSectorIndex < 3) {
        nextBtn.textContent = 'NEXT SECTOR';
      } else {
        nextBtn.textContent = 'BACK TO SECTOR 1';
      }
    }

    // Play victorious jump portal fanfare & vacuum collapse
    playPortalWarpVacuumCollapse();
    playPortalWarpFanfare();
    triggerWarpJumpFlash(currentSector.portalThemeColorStr);

    showResults('sector-completed-modal');
  }

  function triggerWarpJumpFlash(colorStr) {
    if (Sim.silent) return;
    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = `brightness(2.2) drop-shadow(0 0 60px ${colorStr || '#00f0ff'})`;
      setTimeout(() => { container.style.filter = 'none'; }, 350);
    }
  }

  // --- Background 13s–40s Void Doom Trajectory Evaluator ---
  function evaluateBackgroundDoomLookahead() {
    // An obstacle is a usable anchor if it will pass within push reach (rock surface)
    const maxReachCm = Tuning.PushReachCm;

    const totalFwdSpeed = State.CoMVelocity.x;
    if (totalFwdSpeed < Tuning.StallSpeedThresholdCmPerSec) {
      State.DoomSnapshot.bIsDoomImminent = true;
      State.DoomSnapshot.reason = 'ZeroForwardDriftStall';
      State.DoomSnapshot.lookaheadHorizon = 13.0;
      State.DoomSnapshot.reachableCount = 0;
      triggerVoidDoom(State.DoomSnapshot);
      return;
    }

    const startX = State.CoMPosition.x + Tuning.MinLookaheadSeconds * totalFwdSpeed;
    const endX = State.CoMPosition.x + Tuning.MaxLookaheadSeconds * totalFwdSpeed;

    let reachableCount = 0;
    let nearestTime = -1;

    for (const obs of denseObstacles) {
      if (obs.destroyed) continue;
      if (obs.position.x < startX || obs.position.x > endX) continue;

      const t = (obs.position.x - State.CoMPosition.x) / totalFwdSpeed;
      const projY = State.CoMPosition.y + State.CoMVelocity.y * t;
      const projZ = State.CoMPosition.z + State.CoMVelocity.z * t;

      const lateralDist = Math.hypot(obs.position.y - projY, obs.position.z - projZ);
      const effectiveReach = maxReachCm + obs.radius;

      if (lateralDist <= effectiveReach) {
        reachableCount++;
        if (nearestTime < 0 || t < nearestTime) {
          nearestTime = t;
        }
      }
    }

    State.DoomSnapshot.reachableCount = reachableCount;
    State.DoomSnapshot.nearestTime = nearestTime;
    State.DoomSnapshot.lookaheadHorizon = Tuning.MaxLookaheadSeconds;

    if (reachableCount === 0) {
      State.DoomSnapshot.bIsDoomImminent = true;
      State.DoomSnapshot.reason = 'ZeroReachableAnchorsInVoid';
      triggerVoidDoom(State.DoomSnapshot);
    } else {
      State.DoomSnapshot.bIsDoomImminent = false;
      State.DoomSnapshot.reason = 'None';
    }
  }

  // --- Three.js Scene Setup & Visual Entities ---
  let scene, camera, renderer;
  let ambientLight, keyCosmicLight, rimCosmicLight;
  let astronautAvatar, orbitRingMesh, pushReachRing, tangentArrow, trajectoryLine;
  let starsParticles, tunnelRings = [];
  let denseObstacles = [];

  // Volumetric Mechanical Appendage Constructs
  let appendageGroup, appendageSocketMesh, appendageArmMesh, appendageJointBands = [];
  let appendagePistonMeshLeft = null, appendagePistonMeshRight = null;
  let maceHeadGroup, maceCrystalMesh, macePointLight, appendageBeamLine;
  let rcsFlames = [], suitTelemetryMaterial = null;

  // New Step 5 & Launch Constructs
  let startingAsteroidGroup, startingAsteroidMesh, startingAnchorPad;
  let directionalArrowGroup, directionalArrowMesh;
  let plasmaTrailMesh, plasmaTrailGeo, plasmaTrailPositions = [], plasmaTrailColors = [];
  let hyperspaceDustParticles, hyperspaceDustGeo;
  let sparkParticlesGroup, sparkParticles = [];

  // Step 6: Multi-Sector Terminus Exit Jump Portal & Milestone Entities
  let exitPortalGroup = null, exitPortalTitaniumRing = null, exitPortalAccretionDisk = null, exitPortalEventHorizon = null, exitPortalLight = null;
  let milestoneGateGroups = [];

  // Step 8: Colossal Asteroid Belt & Multi-Angle Hazards
  let colossalBeltGroup = null;
  let colossalBackgroundAsteroids = [];
  let colossalIncomingAsteroids = [];
  let kineticShockwaveMesh = null;
  let colossalSpawnTimer = 0;

  function init3D() {
    const container = document.getElementById('canvas-container');

    scene = new THREE.Scene();
    scene.fog = new THREE.FogExp2(0x05070d, 0.00028);

    camera = new THREE.PerspectiveCamera(80, window.innerWidth / window.innerHeight, 10, 80000);
    camera.position.set(-650, 0, 120);

    renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: 'high-performance' });
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.toneMapping = THREE.ACESFilmicToneMapping;
    renderer.toneMappingExposure = 1.35;
    container.appendChild(renderer.domElement);

    // High-Contrast Deep Space Lighting Rig
    ambientLight = new THREE.AmbientLight(0x08101e, 1.6);
    scene.add(ambientLight);

    keyCosmicLight = new THREE.DirectionalLight(0x00f0ff, 3.2);
    keyCosmicLight.position.set(-400, -300, 600);
    scene.add(keyCosmicLight);

    rimCosmicLight = new THREE.DirectionalLight(0xbd00ff, 2.5);
    rimCosmicLight.position.set(700, 500, -300);
    scene.add(rimCosmicLight);

    // 1. Starting Celestial Asteroid Anchor (§1.4)
    createStartingAsteroidAnchor();

    // 2. Astronaut Avatar Group (Step 7 High-Fidelity PBR Modeling)
    astronautAvatar = new THREE.Group();
    scene.add(astronautAvatar);

    // Capsule Torso Body
    const bodyGeo = new THREE.CylinderGeometry(24, 24, 88, 16);
    const bodyMat = new THREE.MeshStandardMaterial({ color: 0xe2e8f0, metalness: 0.85, roughness: 0.2 });
    const bodyMesh = new THREE.Mesh(bodyGeo, bodyMat);
    bodyMesh.rotation.x = Math.PI / 2;
    astronautAvatar.add(bodyMesh);

    // High-Reflectivity Gold Visor
    const visorGeo = new THREE.SphereGeometry(19, 16, 16, 0, Math.PI);
    const visorMat = new THREE.MeshStandardMaterial({
      color: 0xffb703,
      emissive: 0xd97706,
      emissiveIntensity: 0.65,
      metalness: 0.95,
      roughness: 0.08,
    });
    const visorMesh = new THREE.Mesh(visorGeo, visorMat);
    visorMesh.position.set(18, 0, 14);
    visorMesh.rotation.y = Math.PI / 2;
    astronautAvatar.add(visorMesh);

    // Life-Support Backpack Unit & Oxygen Cylinders
    const backpackGeo = new THREE.BoxGeometry(32, 28, 44);
    const backpackMat = new THREE.MeshStandardMaterial({ color: 0x334155, metalness: 0.85, roughness: 0.3 });
    const backpackMesh = new THREE.Mesh(backpackGeo, backpackMat);
    backpackMesh.position.set(-20, 0, 0);
    astronautAvatar.add(backpackMesh);

    for (let t = -1; t <= 1; t += 2) {
      const tankGeo = new THREE.CylinderGeometry(5.5, 5.5, 36, 12);
      const tankMat = new THREE.MeshStandardMaterial({ color: 0xe2e8f0, metalness: 0.9, roughness: 0.15 });
      const tankMesh = new THREE.Mesh(tankGeo, tankMat);
      tankMesh.position.set(-34, t * 10, 0);
      tankMesh.rotation.z = Math.PI / 2;
      astronautAvatar.add(tankMesh);
    }

    // Dual RCS Thruster Bells & Dynamic Flame Plumes
    rcsFlames = [];
    for (let s = -1; s <= 1; s += 2) {
      const nozzleGeo = new THREE.ConeGeometry(4, 7, 8);
      const nozzleMat = new THREE.MeshStandardMaterial({ color: 0x1e293b, metalness: 0.95, roughness: 0.2 });
      const nozzleMesh = new THREE.Mesh(nozzleGeo, nozzleMat);
      nozzleMesh.position.set(-26, s * 16, 0);
      nozzleMesh.rotation.z = s * (Math.PI / 2);
      astronautAvatar.add(nozzleMesh);

      const flameGeo = new THREE.ConeGeometry(3.5, 14, 8);
      const flameMat = new THREE.MeshStandardMaterial({
        color: 0x00f0ff,
        emissive: 0x00f0ff,
        emissiveIntensity: 2.5,
        transparent: true,
        opacity: 0.0
      });
      const flameMesh = new THREE.Mesh(flameGeo, flameMat);
      flameMesh.position.set(-26, s * 23, 0);
      flameMesh.rotation.z = -s * (Math.PI / 2);
      astronautAvatar.add(flameMesh);
      rcsFlames.push(flameMesh);
    }

    // Chest Telemetry Status Indicator
    const stripGeo = new THREE.BoxGeometry(4, 16, 8);
    suitTelemetryMaterial = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00f0ff,
      emissiveIntensity: 1.8
    });
    const chestStrip = new THREE.Mesh(stripGeo, suitTelemetryMaterial);
    chestStrip.position.set(16, 0, 0);
    astronautAvatar.add(chestStrip);

    // 3. Volumetric Mechanical Appendage Assembly with Hydraulic Pistons
    appendageGroup = new THREE.Group();
    scene.add(appendageGroup);

    const socketGeo = new THREE.SphereGeometry(18, 16, 16);
    const socketMat = new THREE.MeshStandardMaterial({ color: 0x334155, metalness: 0.9, roughness: 0.2 });
    appendageSocketMesh = new THREE.Mesh(socketGeo, socketMat);
    appendageGroup.add(appendageSocketMesh);

    const armGeo = new THREE.CylinderGeometry(9, 13, 1, 16);
    const armMat = new THREE.MeshStandardMaterial({
      color: 0x475569,
      metalness: 0.85,
      roughness: 0.25,
    });
    appendageArmMesh = new THREE.Mesh(armGeo, armMat);
    appendageGroup.add(appendageArmMesh);

    // Dual Hydraulic Chrome Piston Rods
    const pistonGeo = new THREE.CylinderGeometry(3.5, 3.5, 1, 12);
    const pistonMat = new THREE.MeshStandardMaterial({ color: 0xd1d5db, metalness: 0.95, roughness: 0.1 });
    appendagePistonMeshLeft = new THREE.Mesh(pistonGeo, pistonMat);
    appendagePistonMeshRight = new THREE.Mesh(pistonGeo, pistonMat);
    appendageGroup.add(appendagePistonMeshLeft);
    appendageGroup.add(appendagePistonMeshRight);

    appendageJointBands = [];
    for (let j = 0; j < 3; j++) {
      const bandGeo = new THREE.TorusGeometry(13, 3.5, 12, 24);
      const bandMat = new THREE.MeshStandardMaterial({
        color: 0x00f0ff,
        emissive: 0x00f0ff,
        emissiveIntensity: 1.2,
        roughness: 0.1,
      });
      const band = new THREE.Mesh(bandGeo, bandMat);
      appendageGroup.add(band);
      appendageJointBands.push(band);
    }

    // Heavy Kinetic Mace Head
    maceHeadGroup = new THREE.Group();
    appendageGroup.add(maceHeadGroup);

    const ramCoreGeo = new THREE.CylinderGeometry(34, 42, 54, 8);
    const ramCoreMat = new THREE.MeshStandardMaterial({
      color: 0x1e293b,
      metalness: 0.9,
      roughness: 0.2,
      flatShading: true,
    });
    const ramCoreMesh = new THREE.Mesh(ramCoreGeo, ramCoreMat);
    maceHeadGroup.add(ramCoreMesh);

    const ramRingGeo = new THREE.TorusGeometry(38, 7.5, 16, 32);
    const ramRingMat = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00f0ff,
      emissiveIntensity: 1.0,
      metalness: 0.95,
      roughness: 0.1,
    });
    const frontRing = new THREE.Mesh(ramRingGeo, ramRingMat);
    frontRing.position.set(0, 20, 0);
    frontRing.rotation.x = Math.PI / 2;
    maceHeadGroup.add(frontRing);

    const backRing = new THREE.Mesh(ramRingGeo, ramRingMat);
    backRing.position.set(0, -20, 0);
    backRing.rotation.x = Math.PI / 2;
    maceHeadGroup.add(backRing);

    for (let h = 0; h < 4; h++) {
      const hornGeo = new THREE.ConeGeometry(11, 28, 4);
      const hornMat = new THREE.MeshStandardMaterial({
        color: 0xffb703,
        emissive: 0x7c4f00,
        metalness: 0.85,
        roughness: 0.25,
      });
      const horn = new THREE.Mesh(hornGeo, hornMat);
      const hornAngle = (h / 4) * Math.PI * 2;
      horn.position.set(Math.cos(hornAngle) * 38, 0, Math.sin(hornAngle) * 38);
      horn.rotation.z = Math.PI / 2;
      horn.rotation.y = -hornAngle;
      maceHeadGroup.add(horn);
    }

    const crystalGeo = new THREE.OctahedronGeometry(24, 1);
    const crystalMat = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00f0ff,
      emissiveIntensity: 1.6,
      roughness: 0.1,
      metalness: 0.9,
    });
    maceCrystalMesh = new THREE.Mesh(crystalGeo, crystalMat);
    maceHeadGroup.add(maceCrystalMesh);

    macePointLight = new THREE.PointLight(0x00f0ff, 4.5, 800);
    maceHeadGroup.add(macePointLight);

    // Directional Holographic Beam Reticle
    const reticleGeo = new THREE.BufferGeometry().setFromPoints([
      new THREE.Vector3(0, 0, 0),
      new THREE.Vector3(0, 200, 0)
    ]);
    const reticleMat = new THREE.LineBasicMaterial({
      color: 0x00f0ff,
      transparent: true,
      opacity: 0.75,
    });
    appendageBeamLine = new THREE.Line(reticleGeo, reticleMat);
    maceHeadGroup.add(appendageBeamLine);

    // 4. Directional Holographic Arrow (Departure Aim at Launch & Velocity Vector in Flight)
    createDirectionalVectorArrow();

    // 5. Step 5 Niagara: Speed-Reactive Plasma Ribbon Trail (NS_MacePlasmaTrail)
    createMacePlasmaRibbonTrail();

    // 6. Step 5 Niagara: Hyperspace Directional Speed Dust (NS_HyperspaceDust)
    createHyperspaceSpeedDust();

    // 7. Kinetic Spark Burst Pool (NS_KineticImpactBurst)
    createKineticSparkPool();

    // Mace Orbit Ring (YZ plane guide)
    const ringGeo = new THREE.BufferGeometry();
    const ringPoints = [];
    for (let i = 0; i <= 64; i++) {
      const theta = (i / 64) * Math.PI * 2;
      ringPoints.push(new THREE.Vector3(0, Math.cos(theta) * Tuning.TetherLengthCm, Math.sin(theta) * Tuning.TetherLengthCm));
    }
    ringGeo.setFromPoints(ringPoints);
    const ringMat = new THREE.LineBasicMaterial({ color: 0x38bdf8, transparent: true, opacity: 0.35 });
    orbitRingMesh = new THREE.Line(ringGeo, ringMat);
    scene.add(orbitRingMesh);

    // Push reach ring (range cue, shown only while SPACE is held; not a trajectory line)
    const reachPoints = [];
    for (let i = 0; i < 96; i++) {
      const theta = (i / 96) * Math.PI * 2;
      reachPoints.push(new THREE.Vector3(0, Math.cos(theta), Math.sin(theta)));
    }
    const reachGeo = new THREE.BufferGeometry().setFromPoints(reachPoints);
    const reachMat = new THREE.LineBasicMaterial({ color: 0x00e5ff, transparent: true, opacity: 0.12, depthWrite: false });
    pushReachRing = new THREE.LineLoop(reachGeo, reachMat);
    pushReachRing.visible = false;
    scene.add(pushReachRing);

    // Debug Arrows
    tangentArrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 1), new THREE.Vector3(), 50, 0xffe600, 15, 8);
    scene.add(tangentArrow);

    // Trajectory Line (Blind Math default hidden)
    const trajGeo = new THREE.BufferGeometry();
    const trajMat = new THREE.LineBasicMaterial({ color: 0x00f0ff, linewidth: 2.5, transparent: true, opacity: 0.85 });
    trajectoryLine = new THREE.Line(trajGeo, trajMat);
    trajectoryLine.visible = false;
    scene.add(trajectoryLine);

    // Kinetic Repulsion Shockwave Visual Mesh
    const shockwaveGeo = new THREE.TorusGeometry(80, 8, 16, 48);
    const shockwaveMat = new THREE.MeshBasicMaterial({
      color: 0x00f0ff,
      transparent: true,
      opacity: 0.0,
      blending: THREE.AdditiveBlending
    });
    kineticShockwaveMesh = new THREE.Mesh(shockwaveGeo, shockwaveMat);
    kineticShockwaveMesh.visible = false;
    scene.add(kineticShockwaveMesh);

    // Environmental Generation & Sector 1 Initialization
    createStarfield();
    loadSector(1);

    window.addEventListener('resize', onWindowResize);
  }

  // --- Starting Asteroid Celestial Anchor (§1.4) ---
  function createStartingAsteroidAnchor() {
    startingAsteroidGroup = new THREE.Group();
    startingAsteroidGroup.position.set(-160, 0, 0);

    // Cratered massive celestial asteroid
    const asteroidGeo = new THREE.DodecahedronGeometry(240, 2);
    const asteroidMat = new THREE.MeshStandardMaterial({
      color: 0x1e293b,
      roughness: 0.92,
      metalness: 0.2,
      flatShading: true,
    });
    startingAsteroidMesh = new THREE.Mesh(asteroidGeo, asteroidMat);
    startingAsteroidGroup.add(startingAsteroidMesh);

    // Metallic reaction bracing pad (where astronaut appendage plants)
    const padGeo = new THREE.CylinderGeometry(55, 65, 30, 8);
    const padMat = new THREE.MeshStandardMaterial({
      color: 0x64748b,
      metalness: 0.95,
      roughness: 0.2,
    });
    startingAnchorPad = new THREE.Mesh(padGeo, padMat);
    startingAnchorPad.rotation.z = Math.PI / 2;
    startingAnchorPad.position.set(160, 0, 0);
    startingAsteroidGroup.add(startingAnchorPad);

    // Neon Anchor Guide Ring
    const padRingGeo = new THREE.TorusGeometry(60, 4, 12, 24);
    const padRingMat = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00f0ff,
      emissiveIntensity: 1.2,
    });
    const padRing = new THREE.Mesh(padRingGeo, padRingMat);
    padRing.rotation.y = Math.PI / 2;
    padRing.position.set(175, 0, 0);
    startingAsteroidGroup.add(padRing);

    scene.add(startingAsteroidGroup);
  }

  // --- Directional Vector Arrow (Rule 4 Telemetry) ---
  function createDirectionalVectorArrow() {
    directionalArrowGroup = new THREE.Group();

    // Shaft
    const shaftGeo = new THREE.CylinderGeometry(4, 4, 140, 12);
    const arrowMat = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00f0ff,
      emissiveIntensity: 1.5,
      transparent: true,
      opacity: 0.85,
    });
    const shaft = new THREE.Mesh(shaftGeo, arrowMat);
    shaft.position.set(0, 70, 0);
    directionalArrowGroup.add(shaft);

    // Arrowhead
    const coneGeo = new THREE.ConeGeometry(14, 38, 12);
    const cone = new THREE.Mesh(coneGeo, arrowMat);
    cone.position.set(0, 155, 0);
    directionalArrowGroup.add(cone);

    // Rotate so default orientation points along +X
    directionalArrowGroup.rotation.z = -Math.PI / 2;

    scene.add(directionalArrowGroup);
  }

  // --- Step 5 Niagara: Speed-Reactive Plasma Ribbon Trail ---
  function createMacePlasmaRibbonTrail() {
    const maxPoints = 28;
    plasmaTrailPositions = [];
    plasmaTrailColors = [];

    const positionsArray = new Float32Array(maxPoints * 3);
    const colorsArray = new Float32Array(maxPoints * 3);

    plasmaTrailGeo = new THREE.BufferGeometry();
    plasmaTrailGeo.setAttribute('position', new THREE.BufferAttribute(positionsArray, 3));
    plasmaTrailGeo.setAttribute('color', new THREE.BufferAttribute(colorsArray, 3));

    const trailMat = new THREE.LineBasicMaterial({
      vertexColors: true,
      linewidth: 3.5,
      transparent: true,
      opacity: 0.9,
      blending: THREE.AdditiveBlending,
    });

    plasmaTrailMesh = new THREE.Line(plasmaTrailGeo, trailMat);
    scene.add(plasmaTrailMesh);
  }

  function updatePlasmaRibbonTrail(maceHeadWorldPos) {
    if (!plasmaTrailGeo) return;
    const maxPoints = 28;

    plasmaTrailPositions.unshift(maceHeadWorldPos.clone());
    if (plasmaTrailPositions.length > maxPoints) {
      plasmaTrailPositions.pop();
    }

    const posAttr = plasmaTrailGeo.attributes.position;
    const colAttr = plasmaTrailGeo.attributes.color;

    const maxSpeedRad = Tuning.MaxAngularSpeedDegPerSec * (Math.PI / 180);
    const speedRatio = Math.min(1.0, Math.abs(State.AngularVelocityRadPerSec) / maxSpeedRad);

    // Cyan (0x00f0ff) at rest -> Magenta/Violet (0xd946ef) at max angular velocity
    const baseColor = new THREE.Color().lerpColors(new THREE.Color(0x00f0ff), new THREE.Color(0xd946ef), speedRatio);

    for (let i = 0; i < maxPoints; i++) {
      if (i < plasmaTrailPositions.length) {
        posAttr.setXYZ(i, plasmaTrailPositions[i].x, plasmaTrailPositions[i].y, plasmaTrailPositions[i].z);
        const fade = 1.0 - (i / maxPoints);
        colAttr.setXYZ(i, baseColor.r * fade, baseColor.g * fade, baseColor.b * fade);
      } else {
        posAttr.setXYZ(i, maceHeadWorldPos.x, maceHeadWorldPos.y, maceHeadWorldPos.z);
        colAttr.setXYZ(i, 0, 0, 0);
      }
    }

    posAttr.needsUpdate = true;
    colAttr.needsUpdate = true;
  }

  // --- Step 5 Niagara: Hyperspace Directional Speed Dust ---
  function createHyperspaceSpeedDust() {
    const dustCount = 420;
    hyperspaceDustGeo = new THREE.BufferGeometry();
    const positions = new Float32Array(dustCount * 3);

    for (let i = 0; i < dustCount * 3; i += 3) {
      positions[i] = (Math.random() - 0.5) * 4000;
      positions[i + 1] = (Math.random() - 0.5) * 1600;
      positions[i + 2] = (Math.random() - 0.5) * 1600;
    }

    hyperspaceDustGeo.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    const dustMat = new THREE.PointsMaterial({
      color: 0x00f0ff,
      size: 4.5,
      transparent: true,
      opacity: 0.65,
      blending: THREE.AdditiveBlending,
    });

    hyperspaceDustParticles = new THREE.Points(hyperspaceDustGeo, dustMat);
    scene.add(hyperspaceDustParticles);
  }

  function updateHyperspaceSpeedDust(dt) {
    if (!hyperspaceDustGeo) return;
    const positions = hyperspaceDustGeo.attributes.position.array;
    const dustCount = positions.length / 3;

    // Dust streams along -X proportional to forward speed
    const streamSpeed = State.RunPhase === 'Running' ? (State.CoMVelocity.x * dt * 1.5) : (dt * 120.0);

    for (let i = 0; i < dustCount; i++) {
      const idx = i * 3;
      positions[idx] -= streamSpeed;

      // Wrap around player window
      if (positions[idx] < State.CoMPosition.x - 1200) {
        positions[idx] = State.CoMPosition.x + 2800;
        positions[idx + 1] = State.CoMPosition.y + (Math.random() - 0.5) * 1600;
        positions[idx + 2] = State.CoMPosition.z + (Math.random() - 0.5) * 1600;
      }
    }

    hyperspaceDustGeo.attributes.position.needsUpdate = true;
  }

  // --- Step 5 Niagara: Kinetic Spark Explosion Pool ---
  function createKineticSparkPool() {
    sparkParticlesGroup = new THREE.Group();
    scene.add(sparkParticlesGroup);
    sparkParticles = [];

    const sparkMat = new THREE.MeshBasicMaterial({
      color: 0x00f0ff,
      transparent: true,
      opacity: 1.0,
      blending: THREE.AdditiveBlending,
    });
    const sparkGeo = new THREE.OctahedronGeometry(4, 0);

    for (let i = 0; i < 60; i++) {
      const mesh = new THREE.Mesh(sparkGeo, sparkMat.clone());
      mesh.visible = false;
      sparkParticlesGroup.add(mesh);

      sparkParticles.push({
        mesh: mesh,
        velocity: new THREE.Vector3(),
        life: 0,
        maxLife: 0.45,
      });
    }
  }

  function spawnKineticSparks(originPos, count) {
    if (Sim.silent) return;
    let spawned = 0;
    for (const p of sparkParticles) {
      if (p.life <= 0) {
        p.mesh.position.copy(originPos);
        p.velocity.set(
          (Math.random() - 0.5) * 1800,
          (Math.random() - 0.5) * 1800,
          (Math.random() - 0.5) * 1800
        );
        p.life = 0.45 + Math.random() * 0.2;
        p.maxLife = p.life;
        p.mesh.visible = true;
        p.mesh.material.opacity = 1.0;
        p.mesh.material.color.setHex(Math.random() > 0.4 ? 0x00f0ff : 0xffb703);

        spawned++;
        if (spawned >= count) break;
      }
    }
  }

  function updateKineticSparks(dt) {
    for (const p of sparkParticles) {
      if (p.life > 0) {
        p.life -= dt;
        p.mesh.position.addScaledVector(p.velocity, dt);
        p.velocity.multiplyScalar(0.92); // deceleration
        p.mesh.material.opacity = Math.max(0, p.life / p.maxLife);
        if (p.life <= 0) {
          p.mesh.visible = false;
        }
      }
    }
  }

  // --- Background Starfield & Cylindrical Tunnel Runway ---
  function createStarfield() {
    const starCount = 4500;
    const starGeo = new THREE.BufferGeometry();
    const positions = new Float32Array(starCount * 3);

    for (let i = 0; i < starCount * 3; i += 3) {
      positions[i] = (Math.random() - 0.5) * 16000;
      positions[i + 1] = (Math.random() - 0.5) * 8000;
      positions[i + 2] = (Math.random() - 0.5) * 8000;
    }

    starGeo.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    const starMat = new THREE.PointsMaterial({ color: 0xffffff, size: 4, transparent: true, opacity: 0.8 });
    starsParticles = new THREE.Points(starGeo, starMat);
    scene.add(starsParticles);
  }

  // --- Step 6: Multi-Sector Environment Generation ---
  function clearActiveSectorEnvironment() {
    // Remove corridor rings
    for (const ring of tunnelRings) {
      scene.remove(ring);
      if (ring.geometry) ring.geometry.dispose();
      if (ring.material) ring.material.dispose();
    }
    tunnelRings = [];

    // Remove slalom gate meshes
    for (const target of SlalomTargets) {
      if (target.meshGroup) {
        scene.remove(target.meshGroup);
        target.meshGroup.traverse(child => {
          if (child.geometry) child.geometry.dispose();
          if (child.material) child.material.dispose();
        });
      }
    }
    SlalomTargets = [];

    // Remove dense obstacles
    for (const obs of denseObstacles) {
      if (obs.mesh) {
        scene.remove(obs.mesh);
        if (obs.mesh.geometry) obs.mesh.geometry.dispose();
        if (obs.mesh.material) obs.mesh.material.dispose();
      }
    }
    denseObstacles = [];

    // Remove milestone gates
    for (const gate of milestoneGateGroups) {
      scene.remove(gate);
      gate.traverse(child => {
        if (child.geometry) child.geometry.dispose();
        if (child.material) child.material.dispose();
      });
    }
    milestoneGateGroups = [];

    // Remove exit portal
    if (exitPortalGroup) {
      scene.remove(exitPortalGroup);
      exitPortalGroup.traverse(child => {
        if (child.geometry) child.geometry.dispose();
        if (child.material) child.material.dispose();
      });
      exitPortalGroup = null;
      exitPortalTitaniumRing = null;
      exitPortalAccretionDisk = null;
      exitPortalEventHorizon = null;
      exitPortalLight = null;
    }

    // Remove colossal asteroid belt and incoming hazards
    if (colossalBeltGroup) {
      scene.remove(colossalBeltGroup);
      colossalBeltGroup.traverse(child => {
        if (child.geometry) child.geometry.dispose();
        if (child.material) child.material.dispose();
      });
      colossalBeltGroup = null;
    }
    colossalBackgroundAsteroids = [];
    colossalIncomingAsteroids = [];
  }

  // --- Step 8: Colossal Asteroids, Belt Vista & Kinetic Push Mechanics ---
  const ColossalGreekLetters = ['Alpha', 'Beta', 'Gamma', 'Delta', 'Epsilon', 'Zeta', 'Theta', 'Iota', 'Kappa', 'Lambda', 'Sigma', 'Omega', 'Titan', 'Ares', 'Gorgon'];
  let colossalAsteroidSeq = 1;

  function spawnIncomingColossalAsteroid(spawnForwardX) {
    if (!Sim.silent && !colossalBeltGroup) return;

    // Physics-affecting randomness comes from the seeded sim RNG (same draw order live and silent);
    // cosmetic randomness (crag shape, spin, label number) stays on Math.random.
    // Radius between 450cm and 1800cm (9m to 36m diameter), scaled by tuning knob
    const baseRadius = (450 + simRandom() * 1350) * Tuning.ColossalAsteroidScale;
    const offsetTheta = simRandom() * Math.PI * 2;
    const offsetR = 250 + simRandom() * 1800;
    const speed = Tuning.IncomingAsteroidSpeedCmPerSec * (0.85 + simRandom() * 0.35);
    // Polar angle theta from negative X axis (0 to 28 degrees)
    const angleFromForward = (simRandom() * 28.0) * (Math.PI / 180.0);
    const azim = simRandom() * Math.PI * 2;

    let mesh;
    if (Sim.silent) {
      // Silent re-runs only need the collider: position/rotation stand-ins, no geometry or scene node
      mesh = { position: new THREE.Vector3(), rotation: { x: 0, y: 0, z: 0 }, geometry: null, material: null };
    } else {
      const geo = new THREE.IcosahedronGeometry(baseRadius, 1);

      // Perturb vertices for rugged craggy asteroid shape
      const posAttr = geo.attributes.position;
      for (let i = 0; i < posAttr.count; i++) {
        const vx = posAttr.getX(i);
        const vy = posAttr.getY(i);
        const vz = posAttr.getZ(i);
        const perturb = 1.0 + (Math.random() - 0.5) * 0.28;
        posAttr.setXYZ(i, vx * perturb, vy * perturb, vz * perturb);
      }
      geo.computeVertexNormals();

      const mat = new THREE.MeshStandardMaterial({
        color: 0x334155,
        roughness: 0.85,
        metalness: 0.25,
        flatShading: true,
      });

      mesh = new THREE.Mesh(geo, mat);
      mesh.rotation.set(Math.random() * Math.PI, Math.random() * Math.PI, Math.random() * Math.PI);
      colossalBeltGroup.add(mesh);
    }

    // Position: forward at spawnForwardX, radial offset in Y, Z
    const spawnY = State.CoMPosition.y + Math.cos(offsetTheta) * offsetR;
    const spawnZ = State.CoMPosition.z + Math.sin(offsetTheta) * offsetR;
    mesh.position.set(spawnForwardX, spawnY, spawnZ);

    // Velocity: MUST have at least one component from the forward (vx < 0)
    // and can approach from ANY radial angle

    const vx = -speed * Math.cos(angleFromForward);
    const vy = speed * Math.sin(angleFromForward) * Math.cos(azim);
    const vz = speed * Math.sin(angleFromForward) * Math.sin(azim);

    // Cubic mass scaling: M = 4/3 * pi * (r_m)^3 * density (2600 kg/m^3)
    const radiusMeters = baseRadius * 0.01;
    const volumeM3 = (4.0 / 3.0) * Math.PI * Math.pow(radiusMeters, 3);
    const massKg = Math.round(volumeM3 * 2600.0);

    const uid = 'c' + colossalAsteroidSeq;
    const nameLetter = ColossalGreekLetters[(colossalAsteroidSeq++) % ColossalGreekLetters.length];
    const designation = `ASTEROID [${nameLetter}-${Math.floor(10 + Math.random() * 90)}]`;

    colossalIncomingAsteroids.push({
      uid: uid,
      mesh: mesh,
      radius: baseRadius,
      massKg: massKg,
      velocity: new THREE.Vector3(vx, vy, vz),
      rotSpeed: new THREE.Vector3(
        (Math.random() - 0.5) * 0.3,
        (Math.random() - 0.5) * 0.3,
        (Math.random() - 0.5) * 0.3
      ),
      name: designation,
      destroyed: false,
      rangeCm: 99999.0,
      closingSpeedCms: 0.0,
      ttiSeconds: 99.9,
      bIsCritical: false
    });
  }

  function createColossalAsteroidBeltEnvironment() {
    clearActiveSectorEnvironment();
    colossalBeltGroup = new THREE.Group();
    scene.add(colossalBeltGroup);

    // 1. Vast background belt (outside view: r in [160m, 450m], x in [-200m, +800m])
    const outerBeltCount = 55;
    const asteroidGeos = [
      new THREE.DodecahedronGeometry(1, 1),
      new THREE.IcosahedronGeometry(1, 1),
      new THREE.OctahedronGeometry(1, 2),
    ];

    const outerMat = new THREE.MeshStandardMaterial({
      color: 0x242d38,
      roughness: 0.88,
      metalness: 0.15,
      flatShading: true,
    });

    for (let i = 0; i < outerBeltCount; i++) {
      const geo = asteroidGeos[i % asteroidGeos.length];
      const mesh = new THREE.Mesh(geo, outerMat);

      // Huge size: 10m to 45m radius (1000cm to 4500cm)
      const radius = 1000 + Math.random() * 3500;
      mesh.scale.set(
        radius * (0.8 + Math.random() * 0.4),
        radius * (0.8 + Math.random() * 0.4),
        radius * (0.8 + Math.random() * 0.4)
      );

      // Belt cylindrical coordinates: radial distance outside standard view (16,000 to 45,000 cm)
      const theta = Math.random() * Math.PI * 2;
      const r = 16000 + Math.random() * 28000;
      const posX = -20000 + Math.random() * 80000;
      const posY = Math.cos(theta) * r;
      const posZ = Math.sin(theta) * r;

      mesh.position.set(posX, posY, posZ);
      mesh.rotation.set(Math.random() * Math.PI, Math.random() * Math.PI, Math.random() * Math.PI);
      colossalBeltGroup.add(mesh);

      colossalBackgroundAsteroids.push({
        mesh: mesh,
        rotSpeed: new THREE.Vector3(
          (Math.random() - 0.5) * 0.05,
          (Math.random() - 0.5) * 0.05,
          (Math.random() - 0.5) * 0.05
        )
      });
    }

    // 2. Initial Dynamic Incoming Colossal Asteroids
    colossalIncomingAsteroids = [];
    const initialIncomingCount = 5;
    for (let i = 0; i < initialIncomingCount; i++) {
      spawnIncomingColossalAsteroid(4500 + i * 2800);
    }

    // 3. Kinetic Shockwave Ring Mesh
    if (!kineticShockwaveMesh) {
      const ringGeo = new THREE.RingGeometry(10, 30, 32);
      const ringMat = new THREE.MeshBasicMaterial({
        color: 0x00f0ff,
        side: THREE.DoubleSide,
        transparent: true,
        opacity: 0.0,
        blending: THREE.AdditiveBlending
      });
      kineticShockwaveMesh = new THREE.Mesh(ringGeo, ringMat);
      kineticShockwaveMesh.visible = false;
      scene.add(kineticShockwaveMesh);
    }
  }

  function updateColossalAsteroidBelt(dt) {
    if (!colossalBeltGroup) return;

    const playerPos = new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );

    // 1. Rotate background outer belt monoliths
    if (!Sim.silent) {
      for (const bg of colossalBackgroundAsteroids) {
        bg.mesh.rotation.x += bg.rotSpeed.x * dt;
        bg.mesh.rotation.y += bg.rotSpeed.y * dt;
        bg.mesh.rotation.z += bg.rotSpeed.z * dt;
      }
    }

    // 2. Advance oncoming asteroids
    for (let i = colossalIncomingAsteroids.length - 1; i >= 0; i--) {
      const ast = colossalIncomingAsteroids[i];
      if (ast.destroyed) continue;

      ast.mesh.position.x += ast.velocity.x * dt;
      ast.mesh.position.y += ast.velocity.y * dt;
      ast.mesh.position.z += ast.velocity.z * dt;

      ast.mesh.rotation.x += ast.rotSpeed.x * dt;
      ast.mesh.rotation.y += ast.rotSpeed.y * dt;
      ast.mesh.rotation.z += ast.rotSpeed.z * dt;

      // Check direct collision with player body
      const distToPlayer = ast.mesh.position.distanceTo(playerPos);
      const collisionThreshold = ast.radius + 40.0;

      if (distToPlayer <= collisionThreshold && State.RunPhase === 'Running') {
        const relSpeed = Math.hypot(
          ast.velocity.x - State.CoMVelocity.x,
          ast.velocity.y - State.CoMVelocity.y,
          ast.velocity.z - State.CoMVelocity.z
        );
        // Contact normal (rock -> player) and closing speed along it
        const n = new THREE.Vector3().subVectors(playerPos, ast.mesh.position);
        if (n.lengthSq() < 1e-6) n.set(-1, 0, 0);
        n.normalize();
        const closing = -((State.CoMVelocity.x - ast.velocity.x) * n.x
                        + (State.CoMVelocity.y - ast.velocity.y) * n.y
                        + (State.CoMVelocity.z - ast.velocity.z) * n.z);

        if (closing > Tuning.ColossalCrashSpeedCmPerSec) {
          triggerKineticCrash({ name: ast.name, isAsteroid: true, uid: ast.uid }, relSpeed);
          return;
        }

        if (closing > 1.0) {
          // Bump: the rock pushes you. Match its velocity along the contact normal, then separate.
          State.CoMVelocity.x += n.x * closing;
          State.CoMVelocity.y += n.y * closing;
          State.CoMVelocity.z += n.z * closing;
          const sep = collisionThreshold - distToPlayer + 5.0;
          State.CoMPosition.x += n.x * sep;
          State.CoMPosition.y += n.y * sep;
          State.CoMPosition.z += n.z * sep;

          if (!Sim.silent) {
            playAsteroidOverpoweredSound();
            triggerAsteroidOverpowerFlash();
            spawnKineticSparks(playerPos, 40);
          }
          State.CameraTrauma = 1.0;
        }
      }

      // Despawn if far behind player or deflected far out
      if (ast.mesh.position.x < State.CoMPosition.x - 6000 || ast.mesh.position.length() > 90000) {
        if (!Sim.silent) {
          colossalBeltGroup.remove(ast.mesh);
          if (ast.mesh.geometry) ast.mesh.geometry.dispose();
          if (ast.mesh.material) ast.mesh.material.dispose();
        }
        colossalIncomingAsteroids.splice(i, 1);
      }
    }

    // Maintain 4 to 6 oncoming colossal asteroids
    colossalSpawnTimer += dt;
    if (colossalIncomingAsteroids.length < 5 && colossalSpawnTimer >= 1.8) {
      colossalSpawnTimer = 0;
      spawnIncomingColossalAsteroid(State.CoMPosition.x + 16000 + simRandom() * 4000);
    }

    // 3. Update expanding kinetic shockwave ring
    if (!Sim.silent && kineticShockwaveMesh && kineticShockwaveMesh.visible) {
      kineticShockwaveMesh.scale.addScalar(dt * 35.0);
      kineticShockwaveMesh.material.opacity -= dt * 2.8;
      if (kineticShockwaveMesh.material.opacity <= 0.0) {
        kineticShockwaveMesh.visible = false;
      }
    }
  }

  // --- One-Button Push-Off (Newton's third law, no fudge factors) ---
  function getPushBodyPos() {
    return new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );
  }

  function getPushCandidates() {
    return Tuning.ActiveScreenMode === 'ColossalBelt' ? colossalIncomingAsteroids : denseObstacles;
  }

  function getPushObstacleCenter(obs) {
    return obs.position || obs.mesh.position;
  }

  // Obstacle whose surface is nearest the body within PushReachCm, or null.
  function findPushAnchor() {
    const body = getPushBodyPos();
    let best = null;
    let bestDist = Tuning.PushReachCm;
    for (const obs of getPushCandidates()) {
      if (obs.destroyed) continue;
      const c = getPushObstacleCenter(obs);
      if (Math.abs(c.x - body.x) > Tuning.PushReachCm + obs.radius) continue;
      const d = c.distanceTo(body) - obs.radius;
      if (d <= bestDist) {
        bestDist = d;
        best = obs;
      }
    }
    return best;
  }

  // Closing speed of a movable rock toward the player (cm/s), along the contact normal.
  function getPushClosingSpeed(obs) {
    if (!obs.velocity) return 0.0;
    const body = getPushBodyPos();
    const n = new THREE.Vector3().subVectors(body, getPushObstacleCenter(obs));
    if (n.lengthSq() < 1e-6) return 0.0;
    n.normalize();
    return -((State.CoMVelocity.x - obs.velocity.x) * n.x
           + (State.CoMVelocity.y - obs.velocity.y) * n.y
           + (State.CoMVelocity.z - obs.velocity.z) * n.z);
  }

  function setPushAnchorHighlight(obs, bOn, charge, bDanger) {
    if (Sim.silent || !obs || !obs.mesh || !obs.mesh.material || !obs.mesh.material.emissive) return;
    const mat = obs.mesh.material;
    if (bOn) {
      if (!obs.pushRimSaved) {
        obs.pushRimSaved = { hex: mat.emissive.getHex(), intensity: mat.emissiveIntensity };
      }
      mat.emissive.setHex(bDanger ? 0xff3b5c : 0x00e5ff);
      mat.emissiveIntensity = 0.7 + charge * 0.9;
    } else if (obs.pushRimSaved) {
      mat.emissive.setHex(obs.pushRimSaved.hex);
      mat.emissiveIntensity = obs.pushRimSaved.intensity;
      obs.pushRimSaved = null;
    }
  }

  function clearPushState() {
    setPushAnchorHighlight(State.PushAnchor, false, 0);
    State.PushAnchor = null;
    State.PushBuffer = null;
    State.MaceSnap = null;
  }

  function resetPushCue() {
    State.PushCue = null;
    State.PushCueShown = false;
  }

  function startPushCharge() {
    if (State.RunPhase !== 'Running') return;
    State.bIsChargingKineticPush = true;
    State.KineticPushCharge = 0.0;
    State.PushBuffer = null;
  }

  function releasePush() {
    if (!State.bIsChargingKineticPush) return;
    State.bIsChargingKineticPush = false;
    const charge = Math.max(Tuning.MinPushCharge, State.KineticPushCharge);
    State.KineticPushCharge = 0.0;
    if (State.RunPhase !== 'Running') return;

    const anchor = findPushAnchor();
    if (anchor) {
      executePushOff(anchor, charge);
    } else {
      State.PushBuffer = { until: State.SimTimeMs + Tuning.PushReleaseBufferMs, charge: charge };
    }
  }

  function updatePushOff(dt) {
    // First-runs teaching cue: the first time any rock enters reach in a run
    if (!Sim.silent && !State.PushCueShown && HUD.hintRuns <= HINT_MAX_RUNS) {
      const first = findPushAnchor();
      if (first) {
        State.PushCueShown = true;
        State.PushCue = { obs: first, until: State.SimTimeMs + 1000 };
      }
    }

    // Buffered early release: fire when a rock enters reach, otherwise whiff.
    if (State.PushBuffer) {
      const anchor = findPushAnchor();
      if (anchor) {
        const charge = State.PushBuffer.charge;
        State.PushBuffer = null;
        executePushOff(anchor, charge);
      } else if (State.SimTimeMs > State.PushBuffer.until) {
        State.PushBuffer = null;
        playPushWhiffSound();
      }
    }

    // In-reach highlight while winding up
    const wanted = State.bIsChargingKineticPush ? findPushAnchor() : null;
    if (wanted !== State.PushAnchor) {
      setPushAnchorHighlight(State.PushAnchor, false, 0);
      State.PushAnchor = wanted;
    }
    if (State.PushAnchor) {
      // TOO CLOSE: rose rim when the rock's closing speed exceeds what the current charge can cancel
      const cancel = Math.max(Tuning.MinPushCharge, State.KineticPushCharge) * Tuning.MaxPushDeltaVCmPerSec;
      const bDanger = getPushClosingSpeed(State.PushAnchor) - cancel > Tuning.ColossalCrashSpeedCmPerSec;
      setPushAnchorHighlight(State.PushAnchor, true, State.KineticPushCharge, bDanger);
    }

    // Mace arm visibly reaches for the rock (internal move, ~100 ms)
    if (State.MaceSnap) {
      const snap = State.MaceSnap;
      let delta = snap.targetRad - State.OrbitAngleRad;
      delta = Math.atan2(Math.sin(delta), Math.cos(delta));
      const step = Math.min(1.0, dt / Math.max(dt, snap.remaining));
      State.OrbitAngleRad += delta * step;
      State.OrbitAngleRad = ((State.OrbitAngleRad % (2 * Math.PI)) + 2 * Math.PI) % (2 * Math.PI);
      snap.remaining -= dt;
      if (snap.remaining <= 0) State.MaceSnap = null;
    }
  }

  function executePushOff(anchor, charge) {
    // Crash analysis: "what if that push had been different?" swaps or drops the real push at its moment
    const ov = Sim.override;
    if (ov && !ov.done && State.SimTimeMs >= ov.atMs) {
      ov.done = true;
      if (ov.charge === null) return;
      charge = ov.charge;
    }
    if (!Sim.silent && Hist.active) {
      Hist.events.push({ t: State.SimTimeMs, type: 'pushRelease', charge: charge, anchorUid: anchor.uid || null });
    }
    const body = getPushBodyPos();
    const center = getPushObstacleCenter(anchor);
    const n = new THREE.Vector3().subVectors(body, center);
    if (n.lengthSq() < 1e-6) n.set(-1, 0, 0);
    n.normalize();

    const playerMassKg = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const impulse = charge * Tuning.MaxPushDeltaVCmPerSec * playerMassKg; // kg*cm/s

    State.CoMVelocity.x += n.x * impulse / playerMassKg;
    State.CoMVelocity.y += n.y * impulse / playerMassKg;
    State.CoMVelocity.z += n.z * impulse / playerMassKg;

    if (anchor.massKg && anchor.velocity) {
      anchor.velocity.x -= n.x * impulse / anchor.massKg;
      anchor.velocity.y -= n.y * impulse / anchor.massKg;
      anchor.velocity.z -= n.z * impulse / anchor.massKg;
    }

    const contact = center.clone().addScaledVector(n, anchor.radius);

    // The arm reaches toward the rock's lateral direction
    const latY = center.y - State.CoMPosition.y;
    const latZ = center.z - State.CoMPosition.z;
    if (Math.hypot(latY, latZ) > 1.0) {
      State.MaceSnap = { targetRad: Math.atan2(latZ, latY), remaining: 0.1 };
    }

    if (!Sim.silent && anchor.massKg && kineticShockwaveMesh) {
      kineticShockwaveMesh.position.copy(contact);
      kineticShockwaveMesh.lookAt(body);
      kineticShockwaveMesh.scale.set(1, 1, 1);
      kineticShockwaveMesh.material.opacity = 1.0;
      kineticShockwaveMesh.visible = true;
    }

    State.StrikeCount++;
    playKineticPushBlastSound(charge * 50.0);
    spawnKineticSparks(contact, Math.round(20 + charge * 35));
    State.CameraTrauma = Math.min(1.0, State.CameraTrauma + 0.15 + charge * 0.45);
    State.TargetFOV = Math.min(106.0, State.CurrentFOV + 4.0 + charge * 10.0);
    showToast('PUSH', 'gold');
  }

  function playPushWhiffSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;
    const len = Math.floor(ctx.sampleRate * 0.18);
    const buf = ctx.createBuffer(1, len, ctx.sampleRate);
    const data = buf.getChannelData(0);
    for (let i = 0; i < len; i++) data[i] = (Math.random() * 2 - 1) * (1 - i / len);
    const src = ctx.createBufferSource();
    src.buffer = buf;
    const filter = ctx.createBiquadFilter();
    filter.type = 'highpass';
    filter.frequency.setValueAtTime(2400, now);
    filter.frequency.exponentialRampToValueAtTime(900, now + 0.18);
    const gain = ctx.createGain();
    gain.gain.setValueAtTime(0.12 * Tuning.AudioMasterVolume, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.18);
    src.connect(filter);
    filter.connect(gain);
    gain.connect(ctx.destination);
    src.start(now);
  }

  function triggerAsteroidOverpowerFlash() {
    if (Sim.silent) return;
    showToast('OVERPOWERED!', 'rose');
    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = 'brightness(2.2) drop-shadow(0 0 60px rgba(239,68,68,0.95)) hue-rotate(300deg)';
      setTimeout(() => { container.style.filter = 'none'; }, 220);
    }
  }

  function setScreenMode(mode) {
    Tuning.ActiveScreenMode = mode;
    const btnGauntlet = document.getElementById('btn-screen-gauntlet');
    const btnColossal = document.getElementById('btn-screen-colossal');

    if (mode === 'ColossalBelt') {
      if (btnGauntlet) btnGauntlet.classList.remove('active');
      if (btnColossal) btnColossal.classList.add('active');

      const launchBanner = document.getElementById('launch-guidance-banner');
      if (launchBanner) launchBanner.classList.add('hidden');

      loadSector(State.CurrentSectorIndex);
    } else {
      if (btnGauntlet) btnGauntlet.classList.add('active');
      if (btnColossal) btnColossal.classList.remove('active');

      loadSector(State.CurrentSectorIndex);
    }
  }

  function createMilestoneGates(sector) {
    milestoneGateGroups = [];
    if (!sector.milestones) return;

    for (const milestone of sector.milestones) {
      const group = new THREE.Group();
      group.position.set(milestone.distCm, 0, 0);

      // Outer hexagonal titanium arch
      const archRadius = sector.corridorRadiusCm * 1.05;
      const archGeo = new THREE.TorusGeometry(archRadius, 18, 8, 6);
      const archMat = new THREE.MeshStandardMaterial({
        color: 0x334155,
        metalness: 0.9,
        roughness: 0.25,
      });
      const archMesh = new THREE.Mesh(archGeo, archMat);
      archMesh.rotation.y = Math.PI / 2;
      group.add(archMesh);

      // Inner glowing neon threshold border
      const innerBorderGeo = new THREE.TorusGeometry(archRadius - 12, 6, 8, 6);
      const innerBorderMat = new THREE.MeshStandardMaterial({
        color: milestone.color,
        emissive: milestone.color,
        emissiveIntensity: 1.6,
        metalness: 0.8,
        roughness: 0.1,
      });
      const innerBorderMesh = new THREE.Mesh(innerBorderGeo, innerBorderMat);
      innerBorderMesh.rotation.y = Math.PI / 2;
      group.add(innerBorderMesh);

      // Floating milestone beacon diamond at arch keystone
      const beaconGeo = new THREE.OctahedronGeometry(42, 0);
      const beaconMat = new THREE.MeshStandardMaterial({
        color: milestone.color,
        emissive: milestone.color,
        emissiveIntensity: 2.2,
        roughness: 0.05,
      });
      const beaconMesh = new THREE.Mesh(beaconGeo, beaconMat);
      beaconMesh.position.set(0, 0, archRadius + 20);
      group.add(beaconMesh);

      // Milestone localized beacon light
      const mLight = new THREE.PointLight(milestone.color, 3.5, 900);
      mLight.position.set(0, 0, archRadius);
      group.add(mLight);

      scene.add(group);
      milestoneGateGroups.push(group);
    }
  }

  function createExitPortal(sector) {
    exitPortalGroup = new THREE.Group();
    exitPortalGroup.position.set(sector.corridorLengthCm, 0, 0);

    const portalRadius = sector.corridorRadiusCm * 1.15;

    // 1. Titanium Outer Warp Ring Frame
    const ringGeo = new THREE.TorusGeometry(portalRadius, 32, 16, 48);
    const ringMat = new THREE.MeshStandardMaterial({
      color: 0x1e293b,
      metalness: 0.95,
      roughness: 0.15,
    });
    exitPortalTitaniumRing = new THREE.Mesh(ringGeo, ringMat);
    exitPortalTitaniumRing.rotation.y = Math.PI / 2;
    exitPortalGroup.add(exitPortalTitaniumRing);

    // 2. Rotating Kinetic Accretion Disk
    const accretionGeo = new THREE.RingGeometry(portalRadius * 0.45, portalRadius * 0.95, 36);
    const accretionMat = new THREE.MeshBasicMaterial({
      color: sector.portalColor,
      transparent: true,
      opacity: 0.65,
      side: THREE.DoubleSide,
      blending: THREE.AdditiveBlending,
    });
    exitPortalAccretionDisk = new THREE.Mesh(accretionGeo, accretionMat);
    exitPortalAccretionDisk.rotation.y = Math.PI / 2;
    exitPortalGroup.add(exitPortalAccretionDisk);

    // 3. Central Event Horizon Membrane Disc
    const horizonGeo = new THREE.CircleGeometry(portalRadius * 0.55, 36);
    const horizonMat = new THREE.MeshStandardMaterial({
      color: sector.portalColor,
      emissive: sector.portalColor,
      emissiveIntensity: 2.5,
      transparent: true,
      opacity: 0.88,
      side: THREE.DoubleSide,
    });
    exitPortalEventHorizon = new THREE.Mesh(horizonGeo, horizonMat);
    exitPortalEventHorizon.rotation.y = Math.PI / 2;
    exitPortalGroup.add(exitPortalEventHorizon);

    // 4. Terminus Beacon Point Light
    exitPortalLight = new THREE.PointLight(sector.portalColor, 12000.0, 4500);
    exitPortalLight.position.set(0, 0, 0);
    exitPortalGroup.add(exitPortalLight);

    scene.add(exitPortalGroup);
  }

  function createCorridorRunway(sector) {
    const corridorRadius = sector.corridorRadiusCm;
    const totalLength = sector.corridorLengthCm;
    const ringInterval = 3000;

    const standardRingGeo = new THREE.TorusGeometry(corridorRadius, 6, 8, 36);
    const standardRingMat = new THREE.MeshBasicMaterial({ color: 0x1e293b, transparent: true, opacity: 0.35 });

    const milestoneRingGeo = new THREE.TorusGeometry(corridorRadius, 14, 8, 36);
    const milestoneRingMat = new THREE.MeshBasicMaterial({ color: sector.portalColor, transparent: true, opacity: 0.75 });

    const ringCount = Math.floor(totalLength / ringInterval);
    for (let i = 0; i <= ringCount; i++) {
      const ringX = i * ringInterval;
      const isMilestone = (ringX % 15000 === 0);

      const ring = new THREE.Mesh(
        isMilestone ? milestoneRingGeo : standardRingGeo,
        isMilestone ? milestoneRingMat : standardRingMat
      );
      ring.rotation.y = Math.PI / 2;
      ring.position.set(ringX, 0, 0);
      scene.add(ring);
      tunnelRings.push(ring);
    }

    // Slalom calibration gates
    for (const target of SlalomTargets) {
      const group = new THREE.Group();
      group.position.set(target.forwardX, target.targetY, target.targetZ);

      const targetRingGeo = new THREE.TorusGeometry(target.radius, 10, 8, 32);
      const targetRingMat = new THREE.MeshStandardMaterial({
        color: target.color,
        emissive: target.color,
        emissiveIntensity: 0.8,
        metalness: 0.8,
        roughness: 0.2,
      });
      const ringMesh = new THREE.Mesh(targetRingGeo, targetRingMat);
      ringMesh.rotation.y = Math.PI / 2;
      group.add(ringMesh);

      const coreGeo = new THREE.IcosahedronGeometry(target.radius * 0.45, 1);
      const coreMat = new THREE.MeshStandardMaterial({
        color: target.color,
        emissive: target.color,
        emissiveIntensity: 0.5,
        metalness: 0.9,
        roughness: 0.1,
      });
      const coreMesh = new THREE.Mesh(coreGeo, coreMat);
      group.add(coreMesh);

      scene.add(group);
      target.meshGroup = group;
    }
  }

  function createDenseHazardGauntlet(sector) {
    denseObstacles = [];
    const totalCorridorCm = sector.corridorLengthCm;
    const corridorRadius = sector.corridorRadiusCm;
    const density = sector.hazardDensity || 1.0;

    let seed = 42 + sector.index * 1337;
    function rand() {
      seed = (seed * 9301 + 49297) % 233280;
      return seed / 233280;
    }

    let currentX = 4500; // Start after 45m safe runway

    while (currentX < (totalCorridorCm - 3500)) { // Clear runway right before jump portal
      const cadenceSec = (1.8 + rand() * 2.0) / density;
      currentX += cadenceSec * Tuning.ForwardSpeedCmPerSec;

      const clusterSize = 1 + Math.floor(rand() * (2 + Math.floor(density)));

      for (let c = 0; c < clusterSize; c++) {
        const angle = rand() * Math.PI * 2;
        const radial = 130.0 + rand() * (corridorRadius * 0.72);
        const jitterX = (rand() - 0.5) * 350.0;

        const posX = currentX + jitterX;
        const posY = Math.cos(angle) * radial;
        const posZ = Math.sin(angle) * radial;

        const isAsteroid = rand() > 0.4;
        const radius = isAsteroid ? (90 + rand() * 110) : (80 + rand() * 90);

        let mesh;
        if (isAsteroid) {
          const geo = new THREE.DodecahedronGeometry(radius, 1);
          const mat = new THREE.MeshStandardMaterial({
            color: sector.index === 3 ? 0x24242e : 0x334155,
            roughness: 0.85,
            metalness: sector.index === 3 ? 0.65 : 0.25,
            flatShading: true,
          });
          mesh = new THREE.Mesh(geo, mat);
        } else {
          const geo = new THREE.BoxGeometry(radius * 1.5, radius * 0.8, radius * 0.8);
          const mat = new THREE.MeshStandardMaterial({
            color: sector.index === 2 ? 0x78716c : 0x64748b,
            metalness: 0.85,
            roughness: 0.25,
          });
          mesh = new THREE.Mesh(geo, mat);
        }

        mesh.position.set(posX, posY, posZ);
        mesh.rotation.set(rand() * Math.PI, rand() * Math.PI, rand() * Math.PI);
        scene.add(mesh);

        denseObstacles.push({
          uid: 'd' + denseObstacles.length,
          position: new THREE.Vector3(posX, posY, posZ),
          radius: radius,
          mesh: mesh,
          restitution: 0.80,
          destroyed: false,
          isAsteroid: isAsteroid,
          name: isAsteroid ? 'Dense Asteroid' : 'Station Debris',
        });
      }
    }
  }

  function updateSectorAtmosphere(sectorIndex) {
    if (!scene || !scene.fog) return;
    if (sectorIndex === 1) {
      scene.fog.color.setHex(0x05070d);
      if (ambientLight) ambientLight.color.setHex(0x08101e);
      if (keyCosmicLight) keyCosmicLight.color.setHex(0x00f0ff);
      if (rimCosmicLight) rimCosmicLight.color.setHex(0xbd00ff);
    } else if (sectorIndex === 2) {
      scene.fog.color.setHex(0x0c0804);
      if (ambientLight) ambientLight.color.setHex(0x181008);
      if (keyCosmicLight) keyCosmicLight.color.setHex(0xf59e0b);
      if (rimCosmicLight) rimCosmicLight.color.setHex(0x00f0ff);
    } else if (sectorIndex === 3) {
      scene.fog.color.setHex(0x0a040d);
      if (ambientLight) ambientLight.color.setHex(0x15081c);
      if (keyCosmicLight) keyCosmicLight.color.setHex(0xd946ef);
      if (rimCosmicLight) rimCosmicLight.color.setHex(0xffb703);
    }
  }

  function loadSector(sectorIndex) {
    if (!SectorCatalog[sectorIndex]) sectorIndex = 1;
    State.CurrentSectorIndex = sectorIndex;
    const sector = SectorCatalog[sectorIndex];

    clearActiveSectorEnvironment();
    updateSectorAtmosphere(sectorIndex);

    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      createColossalAsteroidBeltEnvironment();
      restartRun();
      return;
    }

    // Populate slalom gates
    SlalomTargets = sector.gates.map(g => ({ ...g, evaluated: false, cleared: false }));

    createCorridorRunway(sector);
    createDenseHazardGauntlet(sector);
    createMilestoneGates(sector);
    createExitPortal(sector);

    // Update UI Badges
    const btnSector = document.getElementById('btn-sector-select');
    if (btnSector) {
      btnSector.textContent = `${sector.name.toUpperCase()} ▾`;
      btnSector.style.borderColor = sector.portalThemeColorStr;
      btnSector.style.color = sector.portalThemeColorStr;
    }

    const distEl = document.getElementById('stat-track-dist');
    if (distEl) {
      distEl.textContent = `0.0 m / ${(sector.corridorLengthCm * 0.01).toFixed(0)} m`;
    }

    // Update active sector card in modal
    const cards = document.querySelectorAll('.sector-choice-card');
    cards.forEach(card => {
      card.classList.toggle('active', parseInt(card.dataset.sector, 10) === sectorIndex);
    });

    restartRun();
  }

  function onWindowResize() {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
  }

  function triggerKickFlash() {
    if (Sim.silent) return;
    const container = document.getElementById('canvas-container');
    container.style.filter = 'brightness(1.4) drop-shadow(0 0 24px rgba(0,240,255,0.7))';
    setTimeout(() => { container.style.filter = 'none'; }, 130);
  }

  // --- Render Loop ---
  let lastTime = performance.now();
  let simAccumulator = 0;

  // One fixed 120 Hz sim step: the same unit the silent crash re-runs use
  function simStep(dt) {
    physicsTick(dt);
    if (Tuning.ActiveScreenMode === 'ColossalBelt') updateColossalAsteroidBelt(dt);
    if (!Sim.silent && Hist.active) histAfterStep();
  }

  function animate() {
    requestAnimationFrame(animate);

    const now = performance.now();
    let dt = (now - lastTime) / 1000.0;
    lastTime = now;
    if (dt > 0.05) dt = 0.05;

    // Title / pause freeze the simulation but keep rendering
    const simActive = UI.screen !== 'paused' && UI.screen !== 'title';

    if (Replay.active) updateReplay(dt);

    if (simActive) {
      // Fixed-step accumulator at 120 Hz (replayable: every step has the same dt); frozen during the ghost replay
      if (!Replay.active) {
        simAccumulator += dt;
        while (simAccumulator >= SIM_DT) {
          simStep(SIM_DT);
          simAccumulator -= SIM_DT;
        }
      }

      updateHyperspaceSpeedDust(dt);
      updateKineticSparks(dt);
    }
    update3DVisuals();
    if (Replay.active) updateReplayOverlay();
    updateHUD();
    drawTacticalRadar();

    renderer.render(scene, camera);
  }

  function update3DVisuals() {
    const totalMass = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const cosT = Math.cos(State.OrbitAngleRad);
    const sinT = Math.sin(State.OrbitAngleRad);

    // 1. Astronaut Body Position & Counter-Rotation (The Dodge)
    astronautAvatar.position.set(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );
    astronautAvatar.rotation.x = State.CounterRotationRad;

    // 2. Mace Head World Position
    let maceWorldPos;

    if (State.RunPhase === 'LaunchPushOff') {
      // In LaunchPushOff, the mechanical appendage is planted firmly against the starting asteroid
      const aimRad = State.LaunchAimAngleDeg * (Math.PI / 180.0);
      const compressedArmLen = Tuning.TetherLengthCm - (State.LaunchPushCharge * 45.0);

      maceWorldPos = new THREE.Vector3(
        -compressedArmLen * Math.cos(aimRad),
        -compressedArmLen * Math.sin(aimRad),
        0
      );
    } else {
      const maceRelY = (Tuning.AstronautMassKg / totalMass) * cosT * Tuning.TetherLengthCm;
      const maceRelZ = (Tuning.AstronautMassKg / totalMass) * sinT * Tuning.TetherLengthCm;
      maceWorldPos = new THREE.Vector3(
        State.CoMPosition.x,
        State.CoMPosition.y + maceRelY,
        State.CoMPosition.z + maceRelZ
      );
    }

    // 3. Volumetric Mechanical Appendage Placement & Alignment
    appendageSocketMesh.position.copy(astronautAvatar.position);

    const armDir = new THREE.Vector3().subVectors(maceWorldPos, astronautAvatar.position);
    const armDist = armDir.length();
    const upVector = new THREE.Vector3(0, 1, 0);
    const armOrientation = new THREE.Quaternion().setFromUnitVectors(upVector, armDir.clone().normalize());

    // Volumetric Shaft
    const armMid = new THREE.Vector3().addVectors(astronautAvatar.position, maceWorldPos).multiplyScalar(0.5);
    appendageArmMesh.position.copy(armMid);
    appendageArmMesh.scale.set(1, Math.max(1, armDist), 1);
    appendageArmMesh.quaternion.copy(armOrientation);

    // Dual Hydraulic Chrome Pistons
    if (appendagePistonMeshLeft && appendagePistonMeshRight) {
      const perpOffset = new THREE.Vector3(0, 0, 1).applyQuaternion(armOrientation).multiplyScalar(10.0);
      appendagePistonMeshLeft.position.copy(armMid).add(perpOffset);
      appendagePistonMeshLeft.scale.set(1, Math.max(1, armDist * 0.95), 1);
      appendagePistonMeshLeft.quaternion.copy(armOrientation);

      appendagePistonMeshRight.position.copy(armMid).sub(perpOffset);
      appendagePistonMeshRight.scale.set(1, Math.max(1, armDist * 0.95), 1);
      appendagePistonMeshRight.quaternion.copy(armOrientation);
    }

    // Joint Rings
    for (let j = 0; j < appendageJointBands.length; j++) {
      const frac = 0.25 + j * 0.25;
      const bandPos = new THREE.Vector3().lerpVectors(astronautAvatar.position, maceWorldPos, frac);
      appendageJointBands[j].position.copy(bandPos);
      appendageJointBands[j].quaternion.copy(armOrientation);
    }

    // Mace Head Group Positioning & Orientation
    maceHeadGroup.position.copy(maceWorldPos);
    maceHeadGroup.quaternion.copy(armOrientation);

    // Speed-reactive Coloration on Mace Crystal & Point Light
    const maxSpeedRad = Tuning.MaxAngularSpeedDegPerSec * (Math.PI / 180);
    const speedRatio = Math.min(1.0, Math.abs(State.AngularVelocityRadPerSec) / maxSpeedRad);
    const emissiveColor = new THREE.Color().lerpColors(new THREE.Color(0x00f0ff), new THREE.Color(0xff00bb), speedRatio);
    maceCrystalMesh.material.color = emissiveColor;
    maceCrystalMesh.material.emissive = emissiveColor;
    maceCrystalMesh.material.emissiveIntensity = (1.0 + speedRatio * 2.0) * Tuning.PlasmaHeatMultiplier;
    maceCrystalMesh.scale.setScalar(1.0 + speedRatio * 0.35 * Tuning.PlasmaHeatMultiplier);
    macePointLight.color = emissiveColor;
    macePointLight.intensity = (3.8 + speedRatio * 3.5) * Tuning.PlasmaHeatMultiplier;

    maceCrystalMesh.rotation.y += 0.05 + speedRatio * 0.15;

    // Animate RCS Thruster Flames during lateral kicks
    const latMag = Math.hypot(State.CoMVelocity.y, State.CoMVelocity.z);
    const latRatio = Math.min(1.0, latMag / 1400.0);
    if (rcsFlames.length >= 2) {
      rcsFlames[0].material.opacity = latRatio;
      rcsFlames[1].material.opacity = latRatio;
      rcsFlames[0].scale.set(1, 0.5 + latRatio * 1.5, 1);
      rcsFlames[1].scale.set(1, 0.5 + latRatio * 1.5, 1);
    }

    // Update Suit Telemetry Strip
    if (suitTelemetryMaterial) {
      if (State.RunPhase === 'CrashedKineticImpact') {
        suitTelemetryMaterial.color.setHex(0xef4444);
        suitTelemetryMaterial.emissive.setHex(0xef4444);
      } else if (State.ProximityAlarm.bIsActive) {
        suitTelemetryMaterial.color.setHex(0xf59e0b);
        suitTelemetryMaterial.emissive.setHex(0xf59e0b);
      } else {
        suitTelemetryMaterial.color.setHex(0x00f0ff);
        suitTelemetryMaterial.emissive.setHex(0x00f0ff);
      }
    }

    // Step 6: Animate Exit Jump Portal & Milestone Beacon Crystals
    if (exitPortalAccretionDisk) {
      exitPortalAccretionDisk.rotation.z += 0.015;
    }
    if (exitPortalEventHorizon) {
      exitPortalEventHorizon.rotation.z -= 0.008;
    }
    for (const mg of milestoneGateGroups) {
      if (mg.children.length >= 3) {
        mg.children[2].rotation.y += 0.02;
        mg.children[2].rotation.z += 0.015;
      }
    }

    // 4. Update Step 5 Niagara Plasma Trail Ribbon
    updatePlasmaRibbonTrail(maceWorldPos);

    // 5. Update 3D Directional Vector Arrow (Rule 4 Telemetry)
    if (directionalArrowGroup) {
      if (State.RunPhase === 'LaunchPushOff') {
        const aimRad = State.LaunchAimAngleDeg * (Math.PI / 180.0);
        directionalArrowGroup.position.set(0, 0, 0);
        directionalArrowGroup.rotation.set(0, 0, 0);
        directionalArrowGroup.rotation.z = aimRad;
        directionalArrowGroup.visible = true;
      } else {
        const velY = State.CoMVelocity.y;
        const velZ = State.CoMVelocity.z;
        const velLen = Math.hypot(State.CoMVelocity.x, velY, velZ);

        directionalArrowGroup.position.copy(astronautAvatar.position);
        if (velLen > 50) {
          const headingDir = new THREE.Vector3(State.CoMVelocity.x, velY, velZ).normalize();
          const arrowDir = new THREE.Vector3(0, 1, 0);
          directionalArrowGroup.quaternion.setFromUnitVectors(arrowDir, headingDir);
        }
        directionalArrowGroup.visible = true;
      }
    }

    // 6. Orbit Ring Center aligns with CoM
    orbitRingMesh.position.set(State.CoMPosition.x, State.CoMPosition.y, State.CoMPosition.z);
    orbitRingMesh.scale.set(1, Tuning.TetherLengthCm / 180.0, Tuning.TetherLengthCm / 180.0);

    // 6.5 Push reach ring follows the body while winding up
    pushReachRing.visible = State.RunPhase === 'Running' && State.bIsChargingKineticPush;
    if (pushReachRing.visible) {
      pushReachRing.position.set(
        State.CoMPosition.x,
        State.CoMPosition.y + State.InternalDisplacement.y,
        State.CoMPosition.z + State.InternalDisplacement.z
      );
      pushReachRing.scale.setScalar(Tuning.PushReachCm);
    }

    // 7. Debug Gizmo Visibility (Strict Rule 4 Blind Math)
    const bDrawGizmos = !State.bBlindMath && State.bShowGizmos;
    tangentArrow.visible = bDrawGizmos;
    trajectoryLine.visible = bDrawGizmos;

    if (bDrawGizmos) {
      const sign = State.AngularVelocityRadPerSec >= 0 ? 1.0 : -1.0;
      const tanY = -sinT * sign;
      const tanZ = cosT * sign;
      const tanSpeed = Math.abs(State.AngularVelocityRadPerSec) * Tuning.TetherLengthCm;

      tangentArrow.position.copy(maceWorldPos);
      tangentArrow.setDirection(new THREE.Vector3(0, tanY, tanZ).normalize());
      tangentArrow.setLength(Math.max(1, tanSpeed * 0.12), 15, 8);

      const trajPoints = [];
      for (let i = 0; i <= 25; i++) {
        const t = (i / 25) * 1.5;
        trajPoints.push(new THREE.Vector3(
          State.CoMPosition.x + State.CoMVelocity.x * t,
          State.CoMPosition.y + State.CoMVelocity.y * t,
          State.CoMPosition.z + State.CoMVelocity.z * t
        ));
      }
      trajectoryLine.geometry.setFromPoints(trajPoints);
    }

    // 8. Dynamic Step 5 Chase Camera Rig
    const camPos = Replay.active ? Replay.camPos : State.CoMPosition;
    const camVel = Replay.active ? Replay.camVel : State.CoMVelocity;
    const fwdSpeed = camVel.x || 3000.0;
    const camSpeedRatio = Math.min(1.5, Math.max(0, (fwdSpeed - 2000.0) / 2500.0));
    const targetArmDist = 650.0 + camSpeedRatio * 120.0;
    State.CameraDistance += (targetArmDist - State.CameraDistance) * 0.12;

    const targetCamX = camPos.x - State.CameraDistance;
    const targetCamY = camPos.y * 0.45;
    const targetCamZ = camPos.z * 0.45 + 120;
    camera.position.x += (targetCamX - camera.position.x) * 0.18;
    camera.position.y += (targetCamY - camera.position.y) * 0.18;
    camera.position.z += (targetCamZ - camera.position.z) * 0.18;
    camera.lookAt(camPos.x + 800, camPos.y * 0.8, camPos.z * 0.8);

    // Dutch roll banking during lateral dodge/momentum kick
    const totalLatY = camVel.y;
    const targetRoll = -(totalLatY / 1600.0) * (Tuning.MaxDutchRollDeg * Math.PI / 180.0);
    State.CameraRollDeg += (targetRoll - State.CameraRollDeg) * 0.15;
    camera.rotation.z += State.CameraRollDeg;

    // Camera trauma / kinetic screen shake ($Shake = Trauma^2$)
    if (State.CameraTrauma > 0.001) {
      const shakeMag = State.CameraTrauma * State.CameraTrauma * 5.0;
      camera.position.y += (Math.random() - 0.5) * shakeMag;
      camera.position.z += (Math.random() - 0.5) * shakeMag;
      camera.rotation.z += (Math.random() - 0.5) * (shakeMag * 0.002);
      State.CameraTrauma = Math.max(0, State.CameraTrauma - 0.016 * Tuning.TraumaDecayRate);
    }

    camera.fov = State.CurrentFOV;
    camera.updateProjectionMatrix();

    if (starsParticles) {
      starsParticles.position.x = camPos.x;
    }
  }

  // --- Tactical Navigational Radar Renderer (120m Lookahead Cockpit Mirror) ---
  function drawTacticalRadar() {
    const canvas = HUD.radarCanvas || document.getElementById('radar-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;
    const center = width / 2;
    const radarRangeCm = 12000.0; // 120 meters lookahead
    const scale = (center - 10) / radarRangeCm;

    ctx.clearRect(0, 0, width, height);
    ctx.save();
    ctx.beginPath();
    ctx.arc(center, center, center, 0, Math.PI * 2);
    ctx.clip(); // circular radar disc

    // 1. Concentric Range Rings (30m, 60m, 90m, 120m)
    const ringSteps = [3000, 6000, 9000, 12000];
    ctx.lineWidth = 1;
    ctx.strokeStyle = 'rgba(0, 240, 255, 0.18)';

    for (const rCm of ringSteps) {
      const radius = rCm * scale;
      ctx.beginPath();
      ctx.arc(center, center, radius, 0, Math.PI * 2);
      ctx.stroke();
    }

    // 2. Tunnel Corridor Boundaries (800cm width lines)
    const tunnelHalfWidth = 800 * scale;
    ctx.strokeStyle = 'rgba(56, 189, 248, 0.28)';
    ctx.setLineDash([4, 4]);
    ctx.beginPath();
    ctx.moveTo(center - tunnelHalfWidth, 10);
    ctx.lineTo(center - tunnelHalfWidth, height - 10);
    ctx.moveTo(center + tunnelHalfWidth, 10);
    ctx.lineTo(center + tunnelHalfWidth, height - 10);
    ctx.stroke();
    ctx.setLineDash([]);

    // 3. Rotating Radar Sweep Beam
    const sweepAngle = (performance.now() * 0.003) % (Math.PI * 2);
    const sweepGradient = ctx.createRadialGradient(center, center, 10, center, center, center - 10);
    sweepGradient.addColorStop(0, 'rgba(0, 240, 255, 0.22)');
    sweepGradient.addColorStop(1, 'rgba(0, 240, 255, 0.0)');

    ctx.save();
    ctx.beginPath();
    ctx.moveTo(center, center);
    ctx.arc(center, center, center - 10, sweepAngle - 0.4, sweepAngle);
    ctx.closePath();
    ctx.fillStyle = sweepGradient;
    ctx.fill();
    ctx.restore();

    // 4. Render Proximate Contacts within 120m Sphere
    const playerX = State.CoMPosition.x;
    const playerY = State.CoMPosition.y;
    let contactCount = 0;

    // Obstacles
    for (const obs of denseObstacles) {
      if (obs.destroyed) continue;
      const relX = obs.position.x - playerX;
      const relY = obs.position.y - playerY;
      const dist = Math.hypot(relX, relY);

      if (dist <= radarRangeCm) {
        contactCount++;
        // On radar: forward (+X) is UP (-CanvasY), lateral (+Y) is RIGHT (+CanvasX)
        const canvasX = center + relY * scale;
        const canvasY = center - relX * scale;

        ctx.fillStyle = obs.isAsteroid ? '#f59e0b' : '#f97316';
        ctx.beginPath();
        const blipSize = Math.max(2.5, (obs.radius * scale) * 0.5);
        ctx.arc(canvasX, canvasY, blipSize, 0, Math.PI * 2);
        ctx.fill();
      }
    }

    // Colossal Incoming Asteroids (In ColossalBelt mode)
    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      for (const ast of colossalIncomingAsteroids) {
        if (ast.destroyed) continue;
        const relX = ast.mesh.position.x - playerX;
        const relY = ast.mesh.position.y - playerY;
        const dist = Math.hypot(relX, relY);

        if (dist <= radarRangeCm) {
          contactCount++;
          const canvasX = center + relY * scale;
          const canvasY = center - relX * scale;

          const isLocked = (State.PushAnchor === ast);
          ctx.strokeStyle = isLocked ? '#f59e0b' : (ast.bIsCritical ? '#ef4444' : '#00f0ff');
          ctx.fillStyle = isLocked ? 'rgba(245,158,11,0.35)' : (ast.bIsCritical ? 'rgba(239,68,68,0.35)' : 'rgba(0,240,255,0.25)');
          ctx.lineWidth = isLocked ? 2.5 : 1.5;

          const radarRadius = Math.max(4.5, (ast.radius * 0.01) * 0.7);
          ctx.beginPath();
          ctx.arc(canvasX, canvasY, radarRadius, 0, Math.PI * 2);
          ctx.fill();
          ctx.stroke();

          if (isLocked) {
            ctx.strokeRect(canvasX - radarRadius - 3, canvasY - radarRadius - 3, (radarRadius + 3) * 2, (radarRadius + 3) * 2);
          }
        }
      }
    }

    // Slalom Gates
    for (const gate of SlalomTargets) {
      const relX = gate.forwardX - playerX;
      const relY = gate.targetY - playerY;
      const dist = Math.hypot(relX, relY);

      if (dist <= radarRangeCm) {
        contactCount++;
        const canvasX = center + relY * scale;
        const canvasY = center - relX * scale;

        ctx.strokeStyle = '#00f0ff';
        ctx.lineWidth = 1.5;
        ctx.strokeRect(canvasX - 3.5, canvasY - 3.5, 7, 7);
      }
    }

    // Milestone Checkpoint Arches
    const currentSector = SectorCatalog[State.CurrentSectorIndex];
    if (currentSector && currentSector.milestones) {
      for (const m of currentSector.milestones) {
        const relX = m.distCm - playerX;
        if (relX >= -1000 && relX <= radarRangeCm) {
          contactCount++;
          const canvasX = center;
          const canvasY = center - relX * scale;
          if (canvasY >= 10 && canvasY <= height - 10) {
            ctx.fillStyle = '#' + m.color.toString(16).padStart(6, '0');
            ctx.beginPath();
            ctx.moveTo(canvasX, canvasY - 6);
            ctx.lineTo(canvasX + 6, canvasY);
            ctx.lineTo(canvasX, canvasY + 6);
            ctx.lineTo(canvasX - 6, canvasY);
            ctx.closePath();
            ctx.fill();
          }
        }
      }
    }

    // Exit Jump Portal (Celestial Terminus)
    if (currentSector) {
      const relX = currentSector.corridorLengthCm - playerX;
      if (relX >= -2000 && relX <= radarRangeCm) {
        contactCount++;
        const canvasX = center;
        const canvasY = center - relX * scale;
        if (canvasY >= 10 && canvasY <= height - 10) {
          ctx.strokeStyle = currentSector.portalThemeColorStr;
          ctx.fillStyle = currentSector.portalThemeColorStr;
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.arc(canvasX, canvasY, 8, 0, Math.PI * 2);
          ctx.stroke();
          ctx.beginPath();
          ctx.arc(canvasX, canvasY, 3.5, 0, Math.PI * 2);
          ctx.fill();
        }
      }
    }

    // Starting Asteroid (if within range)
    if (Math.abs(playerX) <= radarRangeCm) {
      const relX = -160 - playerX;
      const canvasX = center;
      const canvasY = center - relX * scale;
      if (canvasY >= 10 && canvasY <= height - 10) {
        ctx.fillStyle = '#a855f7';
        ctx.beginPath();
        ctx.arc(canvasX, canvasY, 5, 0, Math.PI * 2);
        ctx.fill();
      }
    }

    // 5. Player Central Blip & Heading Arrow
    ctx.fillStyle = '#ffffff';
    ctx.beginPath();
    ctx.arc(center, center, 3.5, 0, Math.PI * 2);
    ctx.fill();

    // Direction pointer
    let headingRad = 0;
    if (State.RunPhase === 'LaunchPushOff') {
      headingRad = State.LaunchAimAngleDeg * (Math.PI / 180.0);
    } else {
      const velY = State.CoMVelocity.y;
      headingRad = Math.atan2(velY, State.CoMVelocity.x || 1.0);
    }

    ctx.strokeStyle = '#00f0ff';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(center, center);
    ctx.lineTo(center + Math.sin(headingRad) * 16, center - Math.cos(headingRad) * 16);
    ctx.stroke();

    ctx.restore();
  }

  // --- HUD Updates ---
  function updateHUD() {
    updateGameHUD();
    if (UI.devOpen) updateDevHUD();
  }

  // --- Minimal Game HUD ---
  // Creator: "make the game screen minimalistic with only the information needed.
  //           the interface doesnt look vary playagle and game like right now."
  const HUD_ARC_CIRCUMFERENCE = 2 * Math.PI * 52; // matches r=52 in #game-hud SVG dial
  const HUD = { ready: false, markerKey: '', gateTicks: [], lastScore: 0, lastArcQ: -1, arcHot: false };

  // First-runs key hint counter (localStorage, tolerant of blocked storage)
  const HINT_MAX_RUNS = 3;
  const HINT_STORAGE_KEY = 'SPACEMENTUM_HINT_RUNS_V1';

  function readHintRuns() {
    try {
      return parseInt(localStorage.getItem(HINT_STORAGE_KEY), 10) || 0;
    } catch (e) {
      return 0;
    }
  }

  function countHintRun() {
    HUD.hintRuns = readHintRuns() + 1;
    try {
      localStorage.setItem(HINT_STORAGE_KEY, String(HUD.hintRuns));
    } catch (e) {}
  }

  function cacheHudRefs() {
    HUD.progress = document.getElementById('hud-progress');
    HUD.progressFill = document.getElementById('hud-progress-fill');
    HUD.progressMarks = document.getElementById('hud-progress-marks');
    HUD.progressPortal = document.getElementById('hud-progress-portal');
    HUD.progressText = document.getElementById('hud-progress-text');
    HUD.scoreVal = document.getElementById('hud-score-val');
    HUD.streak = document.getElementById('hud-streak');
    HUD.speedVal = document.getElementById('hud-speed-val');
    HUD.powerArc = document.getElementById('hud-power-arc');
    HUD.radarCanvas = document.getElementById('radar-canvas');
    HUD.launchHeading = document.getElementById('launch-aim-text');
    HUD.launchKeyA = document.getElementById('launch-key-a');
    HUD.launchKeyD = document.getElementById('launch-key-d');
    HUD.launchHold = document.getElementById('btn-charge-push');
    HUD.launchHoldText = document.getElementById('charge-btn-text');
    HUD.hints = document.getElementById('hud-hints');
    HUD.pushCue = document.getElementById('push-cue');
    HUD.vignette = document.getElementById('vignette-proximity');
    HUD.chevron = document.getElementById('threat-chevron');
    HUD.impactTag = document.getElementById('impact-tag');
    HUD.doomOverlay = document.getElementById('doom-overlay');
    HUD.hintRuns = readHintRuns();
    HUD.ready = true;
  }

  function updatePushCue() {
    const el = HUD.pushCue;
    if (!el) return;
    const cue = State.PushCue;
    if (!cue) {
      if (!el.classList.contains('hidden')) el.classList.add('hidden');
      return;
    }
    const remainingMs = cue.until - State.SimTimeMs;
    if (remainingMs <= 0 || cue.obs.destroyed) {
      State.PushCue = null;
      el.classList.add('hidden');
      return;
    }
    const p = getPushObstacleCenter(cue.obs).clone().project(camera);
    if (p.z > 1.0) {
      el.classList.add('hidden');
      return;
    }
    el.style.left = `${(p.x * 0.5 + 0.5) * window.innerWidth}px`;
    el.style.top = `${(-p.y * 0.5 + 0.5) * window.innerHeight}px`;
    el.style.opacity = String(Math.min(1, remainingMs / 300));
    el.classList.remove('hidden');
  }

  // Only touch the DOM when a value actually changed
  function setHudText(el, value) {
    if (el && el._hudV !== value) {
      el._hudV = value;
      el.textContent = value;
    }
  }

  function setHudClass(el, className) {
    if (el && el.className !== className) el.className = className;
  }

  // Rebuild gate ticks / milestone diamonds / portal color when sector or mode changes
  function rebuildProgressMarkers(sector) {
    HUD.progressMarks.innerHTML = '';
    HUD.gateTicks = [];
    HUD.diamonds = [];
    const len = sector.corridorLengthCm;
    for (const g of sector.gates) {
      const tick = document.createElement('div');
      tick.className = 'hud-tick';
      tick.style.left = `${(g.forwardX / len) * 100}%`;
      HUD.progressMarks.appendChild(tick);
      HUD.gateTicks.push(tick);
    }
    for (const m of sector.milestones || []) {
      const d = document.createElement('div');
      d.className = 'hud-diamond';
      d.style.left = `${(m.distCm / len) * 100}%`;
      d.dataset.dist = m.distCm;
      HUD.progressMarks.appendChild(d);
      HUD.diamonds.push(d);
    }
    HUD.progressPortal.style.borderColor = sector.portalThemeColorStr;
    HUD.progressPortal.style.color = sector.portalThemeColorStr;
  }

  const _alertVec = new THREE.Vector3();

  function updateHudAlerts() {
    const alarm = State.ProximityAlarm;
    const running = State.RunPhase === 'Running';
    const threat = running && alarm.bIsActive ? alarm.threatRatio : 0.0;

    // Vignette opacity scales with the existing threat ratio (quantized to avoid per-frame style churn)
    const vq = Math.round(threat * 40) / 40;
    if (HUD.vignetteQ !== vq) {
      HUD.vignetteQ = vq;
      HUD.vignette.style.opacity = String(Math.min(1, vq * 1.1));
    }

    // Edge chevron pointing at the nearest threat
    const showChevron = threat > 0.05 && alarm.nearestPos && camera;
    if (showChevron) {
      _alertVec.copy(alarm.nearestPos).project(camera);
      let nx = _alertVec.x;
      let ny = _alertVec.y;
      if (_alertVec.z > 1.0) { nx = -nx; ny = -ny; } // behind camera: mirror to the opposite edge
      const m = Math.max(Math.abs(nx), Math.abs(ny));
      const s = m > 0.0001 ? 0.88 / m : 0;
      if (m <= 0.0001) ny = -1; // dead centre: point up
      const ex = m > 0.0001 ? nx * s : 0;
      const ey = m > 0.0001 ? ny * s : -0.88;
      const px = (ex * 0.5 + 0.5) * window.innerWidth;
      const py = (-ey * 0.5 + 0.5) * window.innerHeight;
      const angle = Math.atan2(-ey, ex) * (180 / Math.PI);
      HUD.chevron.style.transform = `translate(${px.toFixed(0)}px, ${py.toFixed(0)}px) rotate(${angle.toFixed(0)}deg)`;
    }
    HUD.chevron.classList.toggle('hidden', !showChevron);

    // "IMPACT" only when closer than 40% of the alarm distance (threat ratio > 0.6)
    HUD.impactTag.classList.toggle('hidden', !(threat > 0.6));

    // Doom: only shown while a doom verdict is pending
    const doom = running && State.DoomSnapshot.bIsDoomImminent;
    HUD.doomOverlay.classList.toggle('hidden', !doom);
  }

  // Player-facing HUD (always runs)
  function updateGameHUD() {
    if (!HUD.ready) cacheHudRefs();

    const isCorridor = Tuning.ActiveScreenMode !== 'ColossalBelt';
    const sector = SectorCatalog[State.CurrentSectorIndex];

    // Progress bar (corridor only)
    HUD.progress.style.visibility = isCorridor ? 'visible' : 'hidden';
    if (isCorridor && sector) {
      const markerKey = `${State.CurrentSectorIndex}|${SlalomTargets.length}`;
      if (HUD.markerKey !== markerKey) {
        HUD.markerKey = markerKey;
        rebuildProgressMarkers(sector);
      }
      const frac = Math.max(0, Math.min(1, State.CoMPosition.x / sector.corridorLengthCm));
      const pct = (frac * 100).toFixed(1);
      if (HUD.progressFill._pct !== pct) {
        HUD.progressFill._pct = pct;
        HUD.progressFill.style.width = `${pct}%`;
      }
      const distM = Math.max(0, Math.round(State.CoMPosition.x * 0.01));
      setHudText(HUD.progressText, `${distM} / ${Math.round(sector.corridorLengthCm * 0.01)} m`);
      for (let i = 0; i < HUD.gateTicks.length && i < SlalomTargets.length; i++) {
        const t = SlalomTargets[i];
        setHudClass(HUD.gateTicks[i], t.evaluated ? (t.cleared ? 'hud-tick cleared' : 'hud-tick missed') : 'hud-tick');
      }
      for (const d of HUD.diamonds) {
        setHudClass(d, State.CoMPosition.x >= parseFloat(d.dataset.dist) ? 'hud-diamond reached' : 'hud-diamond');
      }
    } else {
      HUD.markerKey = '';
    }

    // Score + streak chip
    const score = State.CurrentCompositeScore;
    setHudText(HUD.scoreVal, score.toLocaleString());
    if (score > HUD.lastScore) {
      HUD.scoreVal.classList.remove('pop');
      void HUD.scoreVal.offsetWidth; // restart the pop animation
      HUD.scoreVal.classList.add('pop');
    }
    HUD.lastScore = score;
    const showStreak = State.CleanEvasionStreak > 1;
    HUD.streak.classList.toggle('hidden', !showStreak);
    if (showStreak) setHudText(HUD.streak, `x${State.CleanEvasionStreak}`);

    // Speed dial
    const speedCms = Math.hypot(
      State.CoMVelocity.x,
      State.CoMVelocity.y,
      State.CoMVelocity.z
    );
    setHudText(HUD.speedVal, String(Math.round(speedCms * 0.036)));

    // Launch prompt (corridor LaunchPushOff only; hidden/faded by fireLaunchPush / restartRun)
    if (isCorridor && State.RunPhase === 'LaunchPushOff') {
      const a = State.LaunchAimAngleDeg;
      setHudText(HUD.launchHeading, `${a >= 0 ? '+' : ''}${a.toFixed(1)}\u00B0`);
      HUD.launchKeyA.classList.toggle('active', Input.isAPressed);
      HUD.launchKeyD.classList.toggle('active', Input.isDPressed);
      HUD.launchHold.classList.toggle('charging', State.bIsChargingPush);
      setHudText(HUD.launchHoldText, State.bIsChargingPush ? 'RELEASE TO LAUNCH' : 'HOLD SPACE TO PUSH');
    }

    // Key hints: launch phase, plus first 8s of flight on the first 3 runs
    let hintText = '';
    if (isCorridor && State.RunPhase === 'LaunchPushOff') {
      hintText = 'A / D aim \u00B7 ESC pause';
    } else if (State.RunPhase === 'Running' && HUD.hintRuns <= HINT_MAX_RUNS && State.ActiveFlightTimeSeconds < 8.0) {
      hintText = 'HOLD SPACE \u00B7 LET GO NEAR A ROCK TO PUSH OFF\nA / D swing the mace';
    }
    setHudText(HUD.hints, hintText);
    HUD.hints.classList.toggle('hidden', hintText === '');

    // Contextual SPACE glyph beside the first rock that comes into push reach (first runs only)
    updatePushCue();

    // Alerts: proximity vignette + edge chevron + IMPACT tag, doom overlay
    updateHudAlerts();

    // Power arc: launch charge / push charge (Colossal) / spin energy in flight
    let powerRatio;
    if (!isCorridor) {
      powerRatio = State.KineticPushCharge;
    } else if (State.RunPhase === 'LaunchPushOff') {
      powerRatio = State.LaunchPushCharge;
    } else if (State.bIsChargingKineticPush) {
      powerRatio = State.KineticPushCharge;
    } else {
      const tanSpeed = Math.abs(State.AngularVelocityRadPerSec) * (Tuning.TetherLengthCm * 0.01);
      powerRatio = (0.5 * Tuning.MaceMassKg * tanSpeed * tanSpeed) / 4500.0;
    }
    powerRatio = Math.max(0, Math.min(1, powerRatio));
    // Spin energy is shown dimmed; push charge at full brightness
    HUD.powerArc.style.opacity = (isCorridor && State.RunPhase === 'Running' && !State.bIsChargingKineticPush) ? '0.45' : '';
    const arcQ = Math.round(powerRatio * 500);
    if (arcQ !== HUD.lastArcQ) {
      HUD.lastArcQ = arcQ;
      HUD.powerArc.style.strokeDashoffset = (HUD_ARC_CIRCUMFERENCE * (1 - arcQ / 500)).toFixed(2);
      const hot = powerRatio > 0.85;
      if (hot !== HUD.arcHot) {
        HUD.arcHot = hot;
        HUD.powerArc.classList.toggle('hot', hot);
      }
    }
  }

  // Engineering telemetry (only while the Dev Overlay is visible)
  function updateDevHUD() {
    // Phase Display
    const phaseEl = document.getElementById('stat-run-phase');
    const phaseSubEl = document.getElementById('stat-run-sub');
    if (phaseEl) {
      if (Tuning.ActiveScreenMode === 'ColossalBelt') {
        phaseEl.textContent = 'COLOSSAL BELT (PUSH FIELD)';
        phaseEl.className = 'metric-value highlight-gold';
        if (phaseSubEl) phaseSubEl.textContent = 'KINETIC RETICLE ACTIVE [F/SPACE]';
      } else if (State.RunPhase === 'LaunchPushOff') {
        phaseEl.textContent = 'LAUNCH PUSH-OFF';
        phaseEl.className = 'metric-value highlight-gold';
        if (phaseSubEl) phaseSubEl.textContent = 'ANCHORED ON ASTEROID';
      } else if (State.RunPhase === 'Running') {
        phaseEl.textContent = 'RUNNING (FLIGHT)';
        phaseEl.className = 'metric-value highlight-cyan';
        if (phaseSubEl) phaseSubEl.textContent = 'UNIFORM FORWARD DRIFT';
      } else if (State.RunPhase === 'LostToTheVoid') {
        phaseEl.textContent = 'LOST TO THE VOID';
        phaseEl.className = 'metric-value highlight-rose';
        if (phaseSubEl) phaseSubEl.textContent = 'UNREACHABLE TRAJECTORY';
      } else if (State.RunPhase === 'CrashedKineticImpact') {
        phaseEl.textContent = 'CRASHED (IMPACT)';
        phaseEl.className = 'metric-value highlight-rose';
        if (phaseSubEl) phaseSubEl.textContent = 'HULL OBLITERATION';
      }
    }

    const rpm = (Math.abs(State.AngularVelocityRadPerSec) * 60) / (2 * Math.PI);
    const degPerSec = Math.abs(State.AngularVelocityRadPerSec) * (180 / Math.PI);
    document.getElementById('stat-rpm').textContent = `${rpm.toFixed(1)} RPM`;
    document.getElementById('stat-deg-per-sec').textContent = `${degPerSec.toFixed(0)}°/s`;

    const angleDeg = State.OrbitAngleRad * (180 / Math.PI);
    document.getElementById('stat-angle').textContent = `${angleDeg.toFixed(1)}°`;
    let quadrant = 'RIGHT (+Y)';
    if (angleDeg >= 45 && angleDeg < 135) quadrant = 'UP (+Z)';
    else if (angleDeg >= 135 && angleDeg < 225) quadrant = 'LEFT (-Y)';
    else if (angleDeg >= 225 && angleDeg < 315) quadrant = 'DOWN (-Z)';
    document.getElementById('stat-quadrant').textContent = quadrant;

    const dodgeMag = Math.hypot(State.InternalDisplacement.y, State.InternalDisplacement.z);
    const dodgeRoll = (State.CounterRotationRad * (180 / Math.PI)).toFixed(1);
    const dodgeDispEl = document.getElementById('stat-dodge-disp');
    if (dodgeDispEl) {
      dodgeDispEl.textContent = `Δr: ${dodgeMag.toFixed(1)} cm`;
    }
    const dodgeRotEl = document.getElementById('stat-dodge-rot');
    if (dodgeRotEl) {
      dodgeRotEl.textContent = `Counter-Roll: ${dodgeRoll}°`;
    }

    const comFwdSpeed = State.CoMVelocity.x.toFixed(0);
    const comDefY = State.CoMVelocity.y.toFixed(1);
    const comDefZ = State.CoMVelocity.z.toFixed(1);
    const fwdSpeedEl = document.getElementById('stat-fwd-speed');
    if (fwdSpeedEl) {
      fwdSpeedEl.textContent = `${comFwdSpeed} cm/s`;
    }
    const comDefEl = document.getElementById('stat-com-deflection');
    if (comDefEl) {
      comDefEl.textContent = `Deflect: Y:${comDefY >= 0 ? '+' : ''}${comDefY} | Z:${comDefZ >= 0 ? '+' : ''}${comDefZ}`;
    }

    const doomStatusEl = document.getElementById('stat-doom-status');
    const doomProxiesEl = document.getElementById('stat-doom-proxies');
    if (doomStatusEl && doomProxiesEl) {
      if (State.DoomSnapshot.bIsDoomImminent) {
        doomStatusEl.textContent = 'DOOM IMMINENT (VOID)';
        doomStatusEl.className = 'metric-value highlight-rose';
        doomProxiesEl.textContent = 'ZERO REACHABLE IN 13–40s';
      } else {
        doomStatusEl.textContent = 'SAFE (ANCHORS DETECTED)';
        doomStatusEl.className = 'metric-value highlight-emerald';
        doomProxiesEl.textContent = `${State.DoomSnapshot.reachableCount} Reachable Anchors in Window`;
      }
    }

    const currentSector = SectorCatalog[State.CurrentSectorIndex];
    const distMeters = (State.CoMPosition.x / 100.0).toFixed(1);
    const maxTrackMeters = currentSector ? (currentSector.corridorLengthCm * 0.01).toFixed(0) : '1200';
    const clearedCount = SlalomTargets.filter(t => t.cleared).length;
    const distEl = document.getElementById('stat-track-dist');
    if (distEl) {
      distEl.textContent = `${distMeters} m / ${maxTrackMeters} m`;
    }
    const clearedEl = document.getElementById('stat-slalom-cleared');
    if (clearedEl) {
      clearedEl.textContent = `${clearedCount} / ${SlalomTargets.length} GATES | ${State.StrikeCount} DEFLECTIONS`;
    }

    // Step 6: Live Composite Score & Performance Grade Telemetry
    const scoreValEl = document.getElementById('stat-score');
    const scoreSubEl = document.getElementById('stat-score-sub');
    if (scoreValEl) {
      scoreValEl.textContent = `${State.CurrentCompositeScore.toLocaleString()} PTS`;
    }
    if (scoreSubEl) {
      let projectedGrade = 'C';
      if (State.CurrentCompositeScore >= 16000) projectedGrade = 'S+';
      else if (State.CurrentCompositeScore >= 13500) projectedGrade = 'S';
      else if (State.CurrentCompositeScore >= 10500) projectedGrade = 'A';
      else if (State.CurrentCompositeScore >= 7500) projectedGrade = 'B';
      scoreSubEl.textContent = `STREAK: ${State.CleanEvasionStreak}x | GRADE: ${projectedGrade}`;
    }

    // --- Cockpit Speedometer Updates (Rule 4 Telemetry) ---
    const totalSpeedCms = Math.hypot(
      State.CoMVelocity.x,
      State.CoMVelocity.y,
      State.CoMVelocity.z
    );
    const speedKmh = totalSpeedCms * 0.036;

    const speedoKmhEl = document.getElementById('speedo-kmh');
    const speedoCmsEl = document.getElementById('speedo-cms');
    const speedBarFillEl = document.getElementById('speed-bar-fill');
    const speedVxEl = document.getElementById('speed-vx');
    const speedVlatEl = document.getElementById('speed-vlat');

    if (speedoKmhEl) speedoKmhEl.textContent = speedKmh.toFixed(1);
    if (speedoCmsEl) speedoCmsEl.textContent = `${Math.round(totalSpeedCms)} cm/s`;

    if (speedBarFillEl) {
      const speedPct = Math.min(100, (speedKmh / 140.0) * 100);
      speedBarFillEl.style.width = `${speedPct}%`;
    }

    if (speedVxEl) speedVxEl.textContent = `${Math.round(State.CoMVelocity.x)} cm/s`;
    const latMag = Math.hypot(State.CoMVelocity.y, State.CoMVelocity.z);
    if (speedVlatEl) speedVlatEl.textContent = `${Math.round(latMag)} cm/s`;

    // --- Push Power / Rotational Kinetic Energy Widget Updates ---
    const powerLabelEl = document.getElementById('power-widget-label');
    const powerStatEl = document.getElementById('power-widget-stat');
    const powerBarFillEl = document.getElementById('power-bar-fill');
    const powerDescEl = document.getElementById('power-widget-desc');

    if (State.RunPhase === 'LaunchPushOff') {
      if (powerLabelEl) powerLabelEl.textContent = 'PUSH POWER ACCUMULATOR';
      const chargePct = Math.round(State.LaunchPushCharge * 100);
      const forceN = Math.round(State.LaunchPushCharge * Tuning.MaxLaunchForceNewtons);
      if (powerStatEl) powerStatEl.textContent = `${chargePct}% (${forceN} N)`;
      if (powerBarFillEl) powerBarFillEl.style.width = `${chargePct}%`;
      if (powerDescEl) powerDescEl.textContent = 'Hold [SPACE] to compress hydraulic ram against asteroid';
    } else {
      if (powerLabelEl) powerLabelEl.textContent = 'ROTATIONAL STRIKE ENERGY';
      const tanSpeed = Math.abs(State.AngularVelocityRadPerSec) * (Tuning.TetherLengthCm * 0.01);
      const Ek = 0.5 * Tuning.MaceMassKg * Math.pow(tanSpeed, 2);
      const revs = Math.abs(State.AngularVelocityRadPerSec) / (2 * Math.PI);
      if (powerStatEl) powerStatEl.textContent = `${Math.round(Ek)} J (${revs.toFixed(1)} REV/S)`;
      if (powerBarFillEl) {
        const ekPct = Math.min(100, (Ek / 4500.0) * 100);
        powerBarFillEl.style.width = `${ekPct}%`;
      }
      if (powerDescEl) powerDescEl.textContent = `Tangential strike ready: 40kg mace @ ${Math.round(tanSpeed * 100)} cm/s`;
    }

    const pushForceEl = document.getElementById('stat-push-force');
    const pushStatusEl = document.getElementById('stat-push-status');
    if (pushForceEl && pushStatusEl) {
      const chargePct = Math.round(State.KineticPushCharge * 100);
      const dvCms = Math.round(State.KineticPushCharge * Tuning.MaxPushDeltaVCmPerSec);
      pushForceEl.textContent = `${chargePct}% (\u0394v ${dvCms} cm/s)`;
      if (State.bIsChargingKineticPush) {
        pushForceEl.className = 'metric-value highlight-gold';
        pushStatusEl.textContent = State.PushAnchor ? 'Rock in reach | Release to push off' : 'Winding up | No rock in reach';
      } else {
        pushForceEl.className = 'metric-value highlight-cyan';
        pushStatusEl.textContent = 'Hold SPACE to wind up';
      }
    }

    // Key Pill Active States
  }

  // --- Dev Overlay Toggle ---
  function setDevOverlay(open) {
    UI.devOpen = open;
    const overlay = document.getElementById('dev-overlay');
    if (overlay) overlay.classList.toggle('hidden', !open);
    if (open) updateDevHUD();
  }

  // --- Game Flow: screens, menus, pause, settings ---
  // Creator: "make the game screen minimalistic with only the information needed.
  //           the interface doesnt look vary playagle and game like right now."
  // UI.screen is the run-flow state ('title' | 'playing' | 'paused' | 'results'); it is separate
  // from State.RunPhase. Title/pause freeze the sim (see animate) but keep rendering.
  const SETTINGS_KEY = 'SPACEMENTUM_SETTINGS_V1';
  const SCREEN_IDS = { title: 'screen-title', paused: 'screen-pause' };
  const SUB_IDS = { howto: 'screen-howto', settings: 'screen-settings', sector: 'sector-select-modal' };
  const AUDIO_SETTINGS = [
    // [menu slider, Tuning prop, menu label, dev-drawer slider, dev-drawer label]
    ['set-master', 'AudioMasterVolume', 'set-master-val', 'slider-audio-volume', 'val-audio-volume'],
    ['set-hum', 'EngineHumVolume', 'set-hum-val', 'slider-engine-hum', 'val-engine-hum'],
    ['set-prox', 'ProximityAlarmVolume', 'set-prox-val', 'slider-prox-alarm-vol', 'val-prox-alarm-vol'],
  ];

  // Results (crash / void / victory): UI.screen = 'results' at once (blocks pause, keeps sim rendering),
  // the card itself appears after a short beat so the crash flash / shake is seen first.
  const RESULTS_DELAY_MS = 600;

  function showResults(modalId) {
    UI.screen = 'results';
    UI.resultsModal = modalId;
    document.body.dataset.screen = 'results';
    clearHeldInput();
    clearTimeout(UI.resultsTimer);
    UI.resultsTimer = setTimeout(() => {
      if (UI.screen !== 'results' || UI.resultsModal !== modalId) return;
      const modal = document.getElementById(modalId);
      if (modal) modal.classList.remove('hidden');
      UI.menuIndex = 0;
      selectMenuItem(0);
    }, RESULTS_DELAY_MS);
  }

  // Distance / score / personal best for the crash + void cards
  function fillRunStats(prefix) {
    const dist = document.getElementById(`${prefix}-dist-val`);
    const score = document.getElementById(`${prefix}-score-val`);
    const best = document.getElementById(`${prefix}-best-val`);
    if (dist) dist.textContent = `${Math.max(0, Math.round(State.CoMPosition.x * 0.01))} m`;
    if (score) score.textContent = State.CurrentCompositeScore.toLocaleString();
    if (best) {
      const pb = Tuning.ActiveScreenMode !== 'ColossalBelt' ? SaveGame.getProfile().personalBests[State.CurrentSectorIndex] : null;
      best.textContent = pb ? pb.compositeScore.toLocaleString() : '—';
    }
  }

  function showScreen(name) {
    clearTimeout(UI.resultsTimer);
    UI.screen = name;
    closeSubScreen(true);
    document.body.dataset.screen = name;
    for (const key in SCREEN_IDS) {
      const el = document.getElementById(SCREEN_IDS[key]);
      if (el) el.classList.toggle('hidden', key !== name);
    }
    if (document.activeElement && document.activeElement.blur) document.activeElement.blur();
    if (name === 'title' || name === 'paused') clearHeldInput();
    UI.audioDucked = name === 'title' || name === 'paused';
    applyAudioDuck();
    if (name === 'title') refreshTitleMenu();
    UI.menuIndex = 0;
    selectMenuItem(0);
  }

  // Drop held keys + in-progress charges so releases during a menu never fire kicks/pushes
  function clearHeldInput() {
    if (!Sim.silent && Hist.active) Hist.events.push({ t: State.SimTimeMs, type: 'input', name: 'clear' });
    Input.isAPressed = false;
    Input.isDPressed = false;
    Input.isSpacePressed = false;
    State.HeldSpinDirection = 'None';
    State.bIsChargingPush = false;
    State.bIsChargingKineticPush = false;
    State.LaunchPushCharge = 0.0;
    State.KineticPushCharge = 0.0;
    clearPushState();
    const btn = Sim.silent ? null : document.getElementById('btn-charge-push');
    if (btn) btn.classList.remove('charging');
  }

  function pauseGame() {
    if (UI.screen !== 'playing' || UI.sub) return;
    if (State.RunPhase !== 'LaunchPushOff' && State.RunPhase !== 'Running') return;
    showScreen('paused');
  }

  function resumeGame() {
    if (UI.screen !== 'paused') return;
    lastTime = performance.now();
    showScreen('playing');
  }

  function quitToTitle() {
    restartRun();
    UI.title.sector = State.CurrentSectorIndex;
    UI.title.mode = Tuning.ActiveScreenMode;
    showScreen('title');
  }

  function startRunFromTitle() {
    State.CurrentSectorIndex = UI.title.sector;
    setScreenMode(UI.title.mode); // reloads the sector and restarts the run -> UI.screen = 'playing'
  }

  function refreshTitleMenu() {
    const sector = SectorCatalog[UI.title.sector];
    const isCorridor = UI.title.mode !== 'ColossalBelt';
    const sectorEl = document.getElementById('title-sector-val');
    const modeEl = document.getElementById('title-mode-val');
    const bestEl = document.getElementById('title-best');
    if (sectorEl && sector) sectorEl.textContent = sector.name.replace(/^Sector \d+:\s*/, '');
    if (modeEl) modeEl.textContent = isCorridor ? 'CORRIDOR' : 'COLOSSAL BELT';
    if (bestEl) {
      const pb = SaveGame.getProfile().personalBests[UI.title.sector];
      bestEl.textContent = isCorridor && pb ? `BEST ${pb.compositeScore.toLocaleString()}` : ' ';
    }
  }

  function refreshSectorCards() {
    document.querySelectorAll('.sector-choice-card').forEach(card => {
      card.classList.toggle('active', parseInt(card.dataset.sector, 10) === UI.title.sector);
    });
  }

  function openSubScreen(name) {
    UI.returnIndex = UI.menuIndex;
    UI.sub = name;
    const el = document.getElementById(SUB_IDS[name]);
    if (el) el.classList.remove('hidden');
    if (name === 'settings') syncAudioUI();
    if (name === 'sector') refreshSectorCards();
    UI.menuIndex = 0;
    selectMenuItem(0);
  }

  function closeSubScreen(silent) {
    if (!UI.sub) return;
    const el = document.getElementById(SUB_IDS[UI.sub]);
    if (el) el.classList.add('hidden');
    UI.sub = null;
    if (document.activeElement && document.activeElement.blur) document.activeElement.blur();
    if (!silent) selectMenuItem(UI.returnIndex || 0);
  }

  function getMenuRoot() {
    if (UI.sub === 'howto') return document.getElementById('screen-howto');
    if (UI.sub === 'settings') return document.getElementById('screen-settings');
    if (UI.sub === 'sector') return null;
    if (UI.screen === 'title') return document.getElementById('title-menu');
    if (UI.screen === 'paused') return document.getElementById('pause-menu');
    if (UI.screen === 'results') return document.querySelector('.results-modal:not(.hidden) .menu-list');
    return null;
  }

  function getMenuItems(root) {
    if (!root) return [];
    return Array.from(root.querySelectorAll('.menu-item, input[type=range]')).filter(el => el.offsetParent !== null);
  }

  function selectMenuItem(index) {
    const items = getMenuItems(getMenuRoot());
    if (items.length === 0) return;
    const i = ((index % items.length) + items.length) % items.length;
    UI.menuIndex = i;
    items.forEach((el, n) => el.classList.toggle('selected', n === i));
    if (items[i].tagName === 'INPUT') items[i].focus();
    else if (document.activeElement && document.activeElement.blur) document.activeElement.blur();
  }

  function playMenuBlip(confirm) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;
    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(confirm ? 880 : 620, now);
    gain.gain.setValueAtTime(0.08 * Tuning.AudioMasterVolume, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.07);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.075);
  }

  function cycleTitleOption(action, dir) {
    if (action === 'mode') {
      UI.title.mode = UI.title.mode === 'ColossalBelt' ? 'Gauntlet' : 'ColossalBelt';
    } else if (action === 'sector') {
      UI.title.sector = ((UI.title.sector - 1 + dir + 3) % 3) + 1;
    } else {
      return false;
    }
    refreshTitleMenu();
    return true;
  }

  function activateMenuAction(action) {
    switch (action) {
      case 'play': startRunFromTitle(); break;
      case 'sector': openSubScreen('sector'); break;
      case 'mode': cycleTitleOption('mode', 1); break;
      case 'howto': openSubScreen('howto'); break;
      case 'settings': openSubScreen('settings'); break;
      case 'back': closeSubScreen(); break;
      case 'resume': resumeGame(); break;
      case 'restart':
      case 'retry': restartRun(); break;
      case 'quit':
      case 'menu': quitToTitle(); break;
      case 'watch': startCrashReplay(); break;
      case 'next': loadSector(State.CurrentSectorIndex < 3 ? State.CurrentSectorIndex + 1 : 1); break;
    }
  }

  // Returns true when the key was consumed by the game-flow layer
  function handleMenuKey(e, key) {
    const screen = UI.screen;
    const playing = screen === 'playing';

    // While the ghost replay plays only Esc / Space / Enter do anything (skip)
    if (Replay.active) {
      if (key === 'escape' || key === ' ' || key === 'enter') {
        e.preventDefault();
        stopCrashReplay();
      }
      return true;
    }
    if (!UI.sub && key === 'w' && screen === 'results' && canWatchReplay()) {
      startCrashReplay();
      return true;
    }

    if (key === 'escape') {
      e.preventDefault();
      if (UI.sub) {
        playMenuBlip(false);
        closeSubScreen();
      } else if (playing) {
        pauseGame();
      } else if (screen === 'paused') {
        resumeGame();
      }
      return true;
    }

    if (!UI.sub && key === 'p' && (playing || screen === 'paused')) {
      if (playing) pauseGame(); else resumeGame();
      return true;
    }

    if (!UI.sub && key === 'r' && (playing || screen === 'paused' || screen === 'results')) {
      restartRun();
      return true;
    }

    if (playing && !UI.sub) return false;

    // Sector selector (title screen): up/down picks, enter confirms
    if (UI.sub === 'sector') {
      if (key === 'arrowup' || key === 'arrowdown') {
        e.preventDefault();
        cycleTitleOption('sector', key === 'arrowup' ? -1 : 1);
        refreshSectorCards();
        playMenuBlip(false);
      } else if (key === 'enter') {
        e.preventDefault();
        playMenuBlip(true);
        closeSubScreen();
      }
      return true;
    }

    const items = getMenuItems(getMenuRoot());
    if (key === 'arrowup' || key === 'arrowdown') {
      e.preventDefault();
      selectMenuItem(UI.menuIndex + (key === 'arrowup' ? -1 : 1));
      playMenuBlip(false);
    } else if (key === 'arrowleft' || key === 'arrowright') {
      const item = items[UI.menuIndex];
      if (item && item.dataset && screen === 'title' && !UI.sub) {
        e.preventDefault();
        if (cycleTitleOption(item.dataset.action, key === 'arrowleft' ? -1 : 1)) playMenuBlip(false);
      }
    } else if (key === 'enter') {
      const item = items[UI.menuIndex];
      if (item && item.tagName !== 'INPUT') {
        e.preventDefault();
        item.click();
      }
    }
    return true;
  }

  // --- Audio settings (menu sliders mirror the dev-drawer sliders; persisted to localStorage) ---
  function syncAudioUI() {
    for (const [menuId, prop, menuValId, drawerId, drawerValId] of AUDIO_SETTINGS) {
      const pct = Math.round(Tuning[prop] * 100);
      for (const [id, valId] of [[menuId, menuValId], [drawerId, drawerValId]]) {
        const slider = document.getElementById(id);
        const label = document.getElementById(valId);
        if (slider) slider.value = pct;
        if (label) label.textContent = pct;
      }
    }
  }

  function saveSettings() {
    try {
      const data = {};
      for (const [, prop] of AUDIO_SETTINGS) data[prop] = Tuning[prop];
      localStorage.setItem(SETTINGS_KEY, JSON.stringify(data));
    } catch (e) {}
  }

  function loadSettings() {
    try {
      const data = JSON.parse(localStorage.getItem(SETTINGS_KEY));
      if (data) {
        for (const [, prop] of AUDIO_SETTINGS) {
          const v = parseFloat(data[prop]);
          if (isFinite(v)) Tuning[prop] = Math.max(0, Math.min(1, v));
        }
      }
    } catch (e) {}
    syncAudioUI();
  }

  function setupMenus() {
    loadSettings();

    for (const [menuId, prop] of AUDIO_SETTINGS) {
      const slider = document.getElementById(menuId);
      if (!slider) continue;
      slider.addEventListener('input', () => {
        Tuning[prop] = parseFloat(slider.value) / 100.0;
        syncAudioUI();
        saveSettings();
      });
    }
    // Dev drawer edits (its own handlers already updated Tuning) keep the menu + storage in step
    for (const [, , , drawerId] of AUDIO_SETTINGS) {
      const slider = document.getElementById(drawerId);
      if (slider) slider.addEventListener('input', () => { syncAudioUI(); saveSettings(); });
    }
    const resetBtn = document.getElementById('btn-reset-feel');
    if (resetBtn) resetBtn.addEventListener('click', () => { syncAudioUI(); saveSettings(); });

    document.addEventListener('click', (e) => {
      const btn = e.target.closest ? e.target.closest('.menu-item[data-action]') : null;
      if (!btn) return;
      playMenuBlip(true);
      btn.blur();
      activateMenuAction(btn.dataset.action);
    });

    document.addEventListener('mouseover', (e) => {
      const btn = e.target.closest ? e.target.closest('.menu-item') : null;
      if (!btn) return;
      const idx = getMenuItems(getMenuRoot()).indexOf(btn);
      if (idx >= 0 && idx !== UI.menuIndex) selectMenuItem(idx);
    });

    // Auto-pause when the window loses focus / tab is hidden
    window.addEventListener('blur', pauseGame);
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) pauseGame();
    });
  }

  // --- Keyboard & Mouse Event Listeners ---
  window.addEventListener('keydown', (e) => {
    if (e.repeat) return;
    const key = e.key.toLowerCase();

    if (e.code === 'Backquote' || key === 'f3') {
      e.preventDefault();
      setDevOverlay(!UI.devOpen);
      return;
    }

    if (handleMenuKey(e, key)) return;
    if (UI.screen !== 'playing') return;

    if (key === ' ' || e.code === 'Space') {
      e.preventDefault();
      applyInputEvent('spaceDown');
    } else if (key === 'a') {
      applyInputEvent('aDown');
    } else if (key === 'd') {
      applyInputEvent('dDown');
    }
  });

  window.addEventListener('keyup', (e) => {
    const key = e.key.toLowerCase();

    if (key === ' ' || e.code === 'Space') {
      if (UI.screen === 'playing') e.preventDefault();
      if (UI.screen !== 'playing') { Input.isSpacePressed = false; return; }
      applyInputEvent('spaceUp');
    } else if (key === 'a') {
      if (UI.screen === 'playing') applyInputEvent('aUp'); else Input.isAPressed = false;
    } else if (key === 'd') {
      if (UI.screen === 'playing') applyInputEvent('dUp'); else Input.isDPressed = false;
    }
  });

  // --- Tuning Drawer UI Wiring ---
  function setupTuningDrawer() {
    const drawer = document.getElementById('tuning-drawer');
    const toggleBtn = document.getElementById('drawer-toggle');
    toggleBtn.addEventListener('click', () => {
      drawer.classList.toggle('open');
    });

    function bindSlider(id, targetObj, prop, valId, stepDecimals) {
      const slider = document.getElementById(id);
      const valDisplay = document.getElementById(valId);
      if (!slider || !valDisplay) return;
      slider.addEventListener('input', () => {
        const val = parseFloat(slider.value);
        targetObj[prop] = val;
        valDisplay.textContent = stepDecimals ? val.toFixed(stepDecimals) : val;
      });
    }

    bindSlider('slider-launch-speed', Tuning, 'LaunchSpeedCmPerSec', 'val-launch-speed', 0);
    bindSlider('slider-tether', Tuning, 'TetherLengthCm', 'val-tether', 0);
    bindSlider('slider-mace-mass', Tuning, 'MaceMassKg', 'val-mace-mass', 0);
    bindSlider('slider-accel', Tuning, 'SpinAccelerationDegPerSec2', 'val-accel', 0);
    bindSlider('slider-max-speed', Tuning, 'MaxAngularSpeedDegPerSec', 'val-max-speed', 0);
    bindSlider('slider-push-reach', Tuning, 'PushReachCm', 'val-push-reach', 0);
    bindSlider('slider-push-charge-rate', Tuning, 'PushChargeRatePerSec', 'val-push-charge-rate', 2);
    bindSlider('slider-push-min-charge', Tuning, 'MinPushCharge', 'val-push-min-charge', 2);
    bindSlider('slider-push-max-dv', Tuning, 'MaxPushDeltaVCmPerSec', 'val-push-max-dv', 0);
    bindSlider('slider-push-buffer', Tuning, 'PushReleaseBufferMs', 'val-push-buffer', 0);
    bindSlider('slider-fov-dilation', Tuning, 'MaxSpeedFovDilationDeg', 'val-fov-dilation', 0);
    bindSlider('slider-dutch-roll', Tuning, 'MaxDutchRollDeg', 'val-dutch-roll', 1);

    // Step 7 Sliders
    bindSlider('slider-prox-dist', Tuning, 'ProximityDistanceThresholdCm', 'val-prox-dist', 0);
    const sliderProxDist = document.getElementById('slider-prox-dist');
    if (sliderProxDist) {
      sliderProxDist.addEventListener('input', () => {
        Tuning.ProximityDistanceThresholdCm = parseFloat(sliderProxDist.value) * 100.0;
      });
    }

    const sliderEngineHum = document.getElementById('slider-engine-hum');
    const valEngineHum = document.getElementById('val-engine-hum');
    if (sliderEngineHum && valEngineHum) {
      sliderEngineHum.addEventListener('input', () => {
        const val = parseFloat(sliderEngineHum.value);
        Tuning.EngineHumVolume = val / 100.0;
        valEngineHum.textContent = val;
      });
    }

    const sliderProxAlarmVol = document.getElementById('slider-prox-alarm-vol');
    const valProxAlarmVol = document.getElementById('val-prox-alarm-vol');
    if (sliderProxAlarmVol && valProxAlarmVol) {
      sliderProxAlarmVol.addEventListener('input', () => {
        const val = parseFloat(sliderProxAlarmVol.value);
        Tuning.ProximityAlarmVolume = val / 100.0;
        valProxAlarmVol.textContent = val;
      });
    }

    const sliderPlasmaHeat = document.getElementById('slider-plasma-heat');
    const valPlasmaHeat = document.getElementById('val-plasma-heat');
    if (sliderPlasmaHeat && valPlasmaHeat) {
      sliderPlasmaHeat.addEventListener('input', () => {
        const val = parseFloat(sliderPlasmaHeat.value);
        Tuning.PlasmaHeatMultiplier = val / 100.0;
        valPlasmaHeat.textContent = val;
      });
    }

    const sliderAudioVol = document.getElementById('slider-audio-volume');
    const valAudioVol = document.getElementById('val-audio-volume');
    if (sliderAudioVol && valAudioVol) {
      sliderAudioVol.addEventListener('input', () => {
        const val = parseFloat(sliderAudioVol.value);
        Tuning.AudioMasterVolume = val / 100.0;
        valAudioVol.textContent = val;
      });
    }

    const btnResetFeel = document.getElementById('btn-reset-feel');
    if (btnResetFeel) {
      btnResetFeel.addEventListener('click', () => {
        Tuning.LaunchSpeedCmPerSec = 3000.0;
        Tuning.TetherLengthCm = 180.0;
        Tuning.MaceMassKg = 40.0;
        Tuning.SpinAccelerationDegPerSec2 = 1440.0;
        Tuning.MaxAngularSpeedDegPerSec = 1080.0;
        Tuning.PushReachCm = 650.0;
        Tuning.PushChargeRatePerSec = 0.9;
        Tuning.MinPushCharge = 0.15;
        Tuning.MaxPushDeltaVCmPerSec = 1400.0;
        Tuning.PushReleaseBufferMs = 150.0;
        [['push-reach', 650, '650'], ['push-charge-rate', 0.9, '0.90'], ['push-min-charge', 0.15, '0.15'],
         ['push-max-dv', 1400, '1400'], ['push-buffer', 150, '150']].forEach(([id, v, txt]) => {
          document.getElementById('slider-' + id).value = v;
          document.getElementById('val-' + id).textContent = txt;
        });
        Tuning.MaxSpeedFovDilationDeg = 25.0;
        Tuning.MaxDutchRollDeg = 4.5;
        Tuning.AudioMasterVolume = 1.0;
        Tuning.EngineHumVolume = 0.45;
        Tuning.ProximityAlarmVolume = 0.75;
        Tuning.ProximityDistanceThresholdCm = 2500.0;
        Tuning.PlasmaHeatMultiplier = 1.0;

        document.getElementById('slider-launch-speed').value = 3000;
        document.getElementById('val-launch-speed').textContent = '3000';
        document.getElementById('slider-tether').value = 180;
        document.getElementById('val-tether').textContent = '180';
        document.getElementById('slider-mace-mass').value = 40;
        document.getElementById('val-mace-mass').textContent = '40';
        document.getElementById('slider-accel').value = 1440;
        document.getElementById('val-accel').textContent = '1440';
        document.getElementById('slider-max-speed').value = 1080;
        document.getElementById('val-max-speed').textContent = '1080';
        document.getElementById('slider-fov-dilation').value = 25;
        document.getElementById('val-fov-dilation').textContent = '25';
        document.getElementById('slider-dutch-roll').value = 4.5;
        document.getElementById('val-dutch-roll').textContent = '4.5';
        document.getElementById('slider-audio-volume').value = 100;
        document.getElementById('val-audio-volume').textContent = '100';
        document.getElementById('slider-engine-hum').value = 45;
        document.getElementById('val-engine-hum').textContent = '45';
        document.getElementById('slider-prox-alarm-vol').value = 75;
        document.getElementById('val-prox-alarm-vol').textContent = '75';
        document.getElementById('slider-prox-dist').value = 25;
        document.getElementById('val-prox-dist').textContent = '25';
        document.getElementById('slider-plasma-heat').value = 100;
        document.getElementById('val-plasma-heat').textContent = '100';

        Tuning.ColossalAsteroidScale = 1.0;
        Tuning.OuterBeltRadiusCm = 35000.0;
        Tuning.IncomingAsteroidSpeedCmPerSec = 1600.0;
        Tuning.ColossalCrashSpeedCmPerSec = 900.0;

        const sScale = document.getElementById('slider-colossal-scale');
        if (sScale) sScale.value = 1.0;
        const vScale = document.getElementById('val-colossal-scale');
        if (vScale) vScale.textContent = '1.0';

        const sBelt = document.getElementById('slider-belt-radius');
        if (sBelt) sBelt.value = 350;
        const vBelt = document.getElementById('val-belt-radius');
        if (vBelt) vBelt.textContent = '350';

        const sSpd = document.getElementById('slider-incoming-speed');
        if (sSpd) sSpd.value = 1600;
        const vSpd = document.getElementById('val-incoming-speed');
        if (vSpd) vSpd.textContent = '1600';

        const sCrash = document.getElementById('slider-crash-speed');
        if (sCrash) sCrash.value = 900;
        const vCrash = document.getElementById('val-crash-speed');
        if (vCrash) vCrash.textContent = '900';
      });
    }

    const blindMathBtn = document.getElementById('btn-toggle-blind-math');
    if (blindMathBtn) {
      blindMathBtn.addEventListener('click', () => {
        State.bBlindMath = !State.bBlindMath;
        blindMathBtn.textContent = `BLIND MATH: ${State.bBlindMath ? 'ON' : 'OFF'}`;
        blindMathBtn.className = State.bBlindMath ? 'btn-primary' : 'btn-secondary';
      });
    }

    const gizmoBtn = document.getElementById('btn-toggle-gizmos');
    if (gizmoBtn) {
      gizmoBtn.addEventListener('click', () => {
        State.bShowGizmos = !State.bShowGizmos;
        gizmoBtn.textContent = `GIZMOS: ${State.bShowGizmos ? 'ON' : 'OFF'}`;
        gizmoBtn.className = State.bShowGizmos ? 'btn-primary' : 'btn-secondary';
      });
    }

    // Step 6: Multi-Sector Selection Modal Wiring
    const sectorSelectBtn = document.getElementById('btn-sector-select');
    const sectorModal = document.getElementById('sector-select-modal');
    if (sectorSelectBtn && sectorModal) {
      sectorSelectBtn.addEventListener('click', () => {
        sectorModal.classList.toggle('hidden');
      });
    }

    const closeSectorBtn = document.getElementById('btn-close-sector-modal');
    if (closeSectorBtn && sectorModal) {
      closeSectorBtn.addEventListener('click', () => {
        if (UI.sub === 'sector') closeSubScreen();
        else sectorModal.classList.add('hidden');
      });
    }

    const sectorCards = document.querySelectorAll('.sector-choice-card');
    sectorCards.forEach(card => {
      card.addEventListener('click', () => {
        const idx = parseInt(card.dataset.sector, 10);
        if (UI.screen === 'title') {
          // On the title screen the selector only records the choice; PLAY starts the run
          UI.title.sector = idx;
          closeSubScreen();
          refreshTitleMenu();
          return;
        }
        loadSector(idx);
        if (sectorModal) sectorModal.classList.add('hidden');
      });
    });

    // Step 8 Sliders & Buttons
    bindSlider('slider-colossal-scale', Tuning, 'ColossalAsteroidScale', 'val-colossal-scale', 1);
    bindSlider('slider-incoming-speed', Tuning, 'IncomingAsteroidSpeedCmPerSec', 'val-incoming-speed', 0);
    bindSlider('slider-crash-speed', Tuning, 'ColossalCrashSpeedCmPerSec', 'val-crash-speed', 0);

    const sliderBeltRadius = document.getElementById('slider-belt-radius');
    const valBeltRadius = document.getElementById('val-belt-radius');
    if (sliderBeltRadius && valBeltRadius) {
      sliderBeltRadius.addEventListener('input', () => {
        const val = parseFloat(sliderBeltRadius.value);
        Tuning.OuterBeltRadiusCm = val * 100.0;
        valBeltRadius.textContent = val;
      });
    }

    const btnScreenGauntlet = document.getElementById('btn-screen-gauntlet');
    if (btnScreenGauntlet) {
      btnScreenGauntlet.addEventListener('click', () => setScreenMode('Gauntlet'));
    }

    const btnScreenColossal = document.getElementById('btn-screen-colossal');
    if (btnScreenColossal) {
      btnScreenColossal.addEventListener('click', () => setScreenMode('ColossalBelt'));
    }

    // Interactive Launch Button (Touch/Mouse)
    const chargeBtn = document.getElementById('btn-charge-push');
    if (chargeBtn) {
      chargeBtn.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        applyInputEvent('launchDown');
      });
      window.addEventListener('pointerup', () => {
        if (State.bIsChargingPush) {
          applyInputEvent('launchUp');
        }
      });
    }
  }

  // Dev overlay: REPLAY CHECK
  function setupReplayCheck() {
    const btn = document.getElementById('btn-replay-check');
    if (btn) btn.addEventListener('click', () => { runReplayCheck(); btn.blur(); });
    // Console / test-harness access (?dev=1 only)
    if (/[?&]dev=1/.test(window.location.search)) {
      window.__sm = { State, Input, Sim, Hist, Tuning, applyInputEvent, runReplayCheck, restartRun, setScreenMode,
        loadSector, getDense: () => denseObstacles, getColossal: () => colossalIncomingAsteroids, UI,
        showScreen, startRunFromTitle, simStep, startCrashReplay, stopCrashReplay, Replay, getCamera: () => camera, rebaseHistory: () => { Hist.samples.length = 0; Hist.events.length = 0; Hist.prelaunch = null; recordHistorySample(); }, Analysis, analyzeCrashSync, chooseAnswer, Settings,
        restoreSnapshot, takeSnapshot, withSilent, simAdvance, findObstacleByUid };
    }
  }

  // Initialize
  init3D();
  setupReplayCheck();
  setupTuningDrawer();
  setupMenus();
  if (/[?&]dev=1/.test(window.location.search)) setDevOverlay(true);
  UI.title.sector = State.CurrentSectorIndex;
  UI.title.mode = Tuning.ActiveScreenMode;
  showScreen('title');
  animate();
})();
