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
    ImpulseEfficiencyAlpha: 0.65,
    KickModel: 'ReactionRecoil', // 'ReactionRecoil' or 'CarryAlong'
    MinAngularSpeedForKickDegPerSec: 90.0,
    LateralDampingPerSec: 1.5,
    MaxLateralSpeedCmPerSec: 1800.0,

    // FForwardDriftTuning
    ForwardSpeedCmPerSec: 3000.0, // 108 km/h sacred forward drift (30 m/s)

    // Doom Detection & Lookahead Bounds (Creator Mandate)
    MinLookaheadSeconds: 13.0,
    MaxLookaheadSeconds: 40.0,
    ReachToleranceCm: 80.0,
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
    MaxPushForceKiloNewtons: 50.0,
    CriticalDangerDistanceCm: 1800.0, // 18m danger threshold
    OverpowerRatio: 1.35,
    KineticChargeRatePerSec: 0.95,
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

    // Internal Dodging Velocity (exponentially damped kicks)
    LateralVelocity: { y: 0.0, z: 0.0 },

    // Internal 1:4 Two-Body Counter-Displacement (The Dodge)
    InternalDisplacement: { y: 0.0, z: 0.0 },
    CounterRotationRad: 0.0,

    // Stats
    StrikeCount: 0,
    LastStrikeTime: 0,

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

    // Step 8: Kinetic Aiming, Locking & Push State
    KineticTarget: null,
    CandidateTarget: null,
    KineticLockStatus: 'NoTarget',
    KineticPushCharge: 0.0,
    bIsChargingKineticPush: false,
    AimScreenCoords: { x: window.innerWidth / 2, y: window.innerHeight / 2 },
    AimNormalized: { x: 0, y: 0 },
    AimDirectionWorld: new THREE.Vector3(1, 0, 0),
  };

  // --- Held Input Flags ---
  const Input = {
    isAPressed: false,
    isDPressed: false,
    isSpacePressed: false,
    lastAPressTime: 0,
    lastDPressTime: 0,
  };

  // --- Game Flow / UI State (separate from State.RunPhase physics phases) ---
  const UI = {
    screen: 'playing', // 'title' | 'playing' | 'paused' | 'results'
    devOpen: false,
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
    if (!audioCtx) {
      const AudioContextClass = window.AudioContext || window.webkitAudioContext;
      if (AudioContextClass) {
        audioCtx = new AudioContextClass();
        setupContinuousSpinSynth();
      }
    }
    if (audioCtx && audioCtx.state === 'suspended') {
      audioCtx.resume();
    }
    return audioCtx;
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
    if (!audioCtx || !continuousSpinOsc || !continuousSpinGain) return;
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
    if (!audioCtx || !continuousReactorHumOsc || !continuousReactorHumGain) return;
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

  function updateProximityRadarAlarm(minDistCm, nowSeconds) {
    const thresholdCm = Tuning.ProximityDistanceThresholdCm;
    const proxWidget = document.getElementById('proximity-warning-widget');
    const proxStatus = document.getElementById('prox-status-text');
    const proxDist = document.getElementById('prox-distance-text');
    const proxMeter = document.getElementById('prox-meter-bar');

    if (minDistCm <= thresholdCm && State.RunPhase === 'Running') {
      const threatRatio = Math.max(0.0, Math.min(1.0, 1.0 - (minDistCm / thresholdCm)));
      State.ProximityAlarm.bIsActive = true;
      State.ProximityAlarm.threatRatio = threatRatio;
      State.ProximityAlarm.distanceCm = minDistCm;

      const pingInterval = 1.0 - (threatRatio * 0.92);
      if (nowSeconds - lastProximityPingTime >= pingInterval) {
        lastProximityPingTime = nowSeconds;
        playProximityRadarPing(threatRatio);
      }

      if (proxWidget) {
        if (threatRatio > 0.65) {
          proxWidget.className = 'proximity-warning-widget danger';
          if (proxStatus) proxStatus.textContent = 'COLLISION IMMINENT!';
        } else {
          proxWidget.className = 'proximity-warning-widget warning';
          if (proxStatus) proxStatus.textContent = 'HAZARD PROXIMITY!';
        }
        if (proxDist) proxDist.textContent = (minDistCm * 0.01).toFixed(1) + 'm';
        if (proxMeter) proxMeter.style.width = (threatRatio * 100).toFixed(0) + '%';
      }
    } else {
      State.ProximityAlarm.bIsActive = false;
      State.ProximityAlarm.threatRatio = 0.0;
      State.ProximityAlarm.distanceCm = minDistCm;

      if (proxWidget) {
        proxWidget.className = 'proximity-warning-widget safe';
        if (proxStatus) proxStatus.textContent = 'PROXIMITY: CLEAR';
        if (proxDist) proxDist.textContent = '> ' + (thresholdCm * 0.01).toFixed(0) + 'm';
        if (proxMeter) proxMeter.style.width = '0%';
      }
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

  function playTargetLockBeep(isCritical) {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const freqs = isCritical ? [440, 220, 110] : [780, 1180];
    freqs.forEach((freq, idx) => {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.type = isCritical ? 'sawtooth' : 'sine';
      osc.frequency.setValueAtTime(freq, now + idx * 0.05);
      gain.gain.setValueAtTime((isCritical ? 0.35 : 0.22) * Tuning.AudioMasterVolume, now + idx * 0.05);
      gain.gain.exponentialRampToValueAtTime(0.001, now + idx * 0.05 + 0.08);
      osc.connect(gain);
      gain.connect(ctx.destination);
      osc.start(now + idx * 0.05);
      osc.stop(now + idx * 0.05 + 0.08);
    });
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

  function playKickSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();

    osc.type = 'triangle';
    osc.frequency.setValueAtTime(190, now);
    osc.frequency.exponentialRampToValueAtTime(32, now + 0.18);

    gain.gain.setValueAtTime(0.65, now);
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
        executeMomentumKick();
      }
      State.HeldSpinDirection = 'None';
    }
  }

  // --- Momentum Kick (The Internal Release) ---
  function executeMomentumKick() {
    const absOmega = Math.abs(State.AngularVelocityRadPerSec);
    const minOmegaRad = Tuning.MinAngularSpeedForKickDegPerSec * (Math.PI / 180);

    if (absOmega < minOmegaRad) {
      State.AngularVelocityRadPerSec = 0;
      return;
    }

    const cosT = Math.cos(State.OrbitAngleRad);
    const sinT = Math.sin(State.OrbitAngleRad);
    const sign = State.AngularVelocityRadPerSec >= 0 ? 1.0 : -1.0;
    const tanSpeed = absOmega * Tuning.TetherLengthCm;

    const tanVelY = -sinT * sign * tanSpeed;
    const tanVelZ = cosT * sign * tanSpeed;

    const impulseMag = Tuning.ImpulseEfficiencyAlpha * Tuning.MaceMassKg * tanSpeed;
    const modelMult = Tuning.KickModel === 'ReactionRecoil' ? -1.0 : 1.0;
    const totalMass = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const deltaV = (impulseMag * modelMult) / totalMass;

    const tanLen = Math.hypot(tanVelY, tanVelZ) || 1.0;
    const kickUnitY = (tanVelY / tanLen) * modelMult;
    const kickUnitZ = (tanVelZ / tanLen) * modelMult;

    State.LateralVelocity.y += kickUnitY * deltaV;
    State.LateralVelocity.z += kickUnitZ * deltaV;

    const currentLatSpeed = Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z);
    if (currentLatSpeed > Tuning.MaxLateralSpeedCmPerSec) {
      const scale = Tuning.MaxLateralSpeedCmPerSec / currentLatSpeed;
      State.LateralVelocity.y *= scale;
      State.LateralVelocity.z *= scale;
    }

    State.AngularVelocityRadPerSec = 0.0;

    playKickSound();
    triggerKickFlash();

    const kickRatio = Math.min(1.0, currentLatSpeed / Tuning.MaxLateralSpeedCmPerSec);
    State.TargetFOV = Math.min(104.0, 80.0 + kickRatio * 18.0);
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
    State.LastStrikeTime = performance.now();

    playStrikeSound();
    triggerStrikeFlash(obstacle, deltaV, u);
    spawnKineticSparks(hitWorldPos, 35);

    State.TargetFOV = Math.min(106.0, State.CurrentFOV + 14.0);
    State.CameraTrauma = Math.min(1.0, State.CameraTrauma + 0.45);
  }

  function triggerStrikeFlash(obstacle, deltaV, u) {
    const banner = document.getElementById('strike-banner');
    const titleEl = document.querySelector('.strike-title');
    const detailEl = document.getElementById('strike-detail');
    if (banner && detailEl) {
      const defMag = deltaV.length().toFixed(0);
      if (titleEl) {
        titleEl.textContent = u > 0.65 ? 'MACE IMPACT DEFLECTION!' : 'MECHANICAL ARM PUSH!';
      }
      detailEl.textContent = `Pushed +${defMag} cm/s off ${obstacle.name || 'Asteroid'}! Trajectory shifted.`;
      banner.classList.remove('hidden');

      setTimeout(() => {
        banner.classList.add('hidden');
      }, 2400);
    }

    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = 'brightness(1.6) drop-shadow(0 0 35px rgba(0, 240, 255, 0.9))';
      setTimeout(() => { container.style.filter = 'none'; }, 160);
    }
  }

  // --- Rule 3 Fail State Triggers ---
  function triggerVoidDoom(snapshot) {
    if (State.RunPhase !== 'Running') return;
    State.RunPhase = 'LostToTheVoid';

    playVoidDoomSound();

    const modal = document.getElementById('void-doom-modal');
    const horizonEl = document.getElementById('doom-horizon-val');
    if (horizonEl) {
      horizonEl.textContent = `${snapshot.lookaheadHorizon.toFixed(1)} SECONDS`;
    }
    if (modal) {
      modal.classList.remove('hidden');
    }
  }

  function triggerKineticCrash(obstacle, impactSpeed) {
    if (State.RunPhase !== 'Running') return;
    State.RunPhase = 'CrashedKineticImpact';

    playCrashSound();
    State.CameraTrauma = 1.0;

    const modal = document.getElementById('crash-modal');
    const speedEl = document.getElementById('crash-speed-val');
    if (speedEl) {
      speedEl.textContent = `${impactSpeed.toFixed(0)} cm/s (${(impactSpeed * 0.036).toFixed(1)} km/h)`;
    }
    if (modal) {
      modal.classList.remove('hidden');
    }
  }

  // --- Launch Push-Off Initiation & Departure (§1.4) ---
  function startChargingPush() {
    if (State.RunPhase !== 'LaunchPushOff') return;
    State.bIsChargingPush = true;
    const btn = document.getElementById('btn-charge-push');
    if (btn) btn.classList.add('charging');
  }

  function fireLaunchPush() {
    if (State.RunPhase !== 'LaunchPushOff' || !State.bIsChargingPush) return;
    State.bIsChargingPush = false;

    const btn = document.getElementById('btn-charge-push');
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

    // Audio & Screen Effects
    playLaunchExplosionThump();
    triggerKickFlash();
    spawnKineticSparks(new THREE.Vector3(0, 0, 0), 45);

    // Punch camera FOV & trauma
    State.TargetFOV = Math.min(106.0, 80.0 + finalCharge * 18.0);
    State.CameraTrauma = finalCharge * 0.45;

    const banner = document.getElementById('launch-guidance-banner');
    if (banner) banner.classList.add('hidden');
  }

  function restartRun() {
    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      State.RunPhase = 'Running';
      State.CoMPosition.set(0, 0, 0);
      State.CoMVelocity.set(400, 0, 0);
      State.LateralVelocity.set(0, 0, 0);
      State.AngularVelocityRadPerSec = 0.0;
      State.OrbitAngleRad = 0.0;
      State.HeldSpinDirection = 'None';
      State.StrikeCount = 0;
      State.ActiveFlightTimeSeconds = 0.0;
      State.KineticPushCharge = 0.0;
      State.bIsChargingKineticPush = false;
      unlockKineticTarget();

      const banner = document.getElementById('launch-guidance-banner');
      if (banner) banner.classList.add('hidden');
      const colossalBanner = document.getElementById('colossal-mode-banner');
      if (colossalBanner) colossalBanner.classList.remove('hidden');

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

      updateHUD();
      return;
    }

    State.RunPhase = 'LaunchPushOff';
    State.LaunchAimAngleDeg = 0.0;
    State.LaunchPushCharge = 0.0;
    State.bIsChargingPush = false;

    State.CoMPosition.x = 0.0;
    State.CoMPosition.y = 0.0;
    State.CoMPosition.z = 0.0;
    State.CoMVelocity.x = 0.0;
    State.CoMVelocity.y = 0.0;
    State.CoMVelocity.z = 0.0;
    State.LateralVelocity.y = 0.0;
    State.LateralVelocity.z = 0.0;
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
    if (banner) banner.classList.remove('hidden');

    // Hide modals and banners
    const voidModal = document.getElementById('void-doom-modal');
    if (voidModal) voidModal.classList.add('hidden');
    const crashModal = document.getElementById('crash-modal');
    if (crashModal) crashModal.classList.add('hidden');
    const victoryModal = document.getElementById('sector-completed-modal');
    if (victoryModal) victoryModal.classList.add('hidden');
    const sectorModal = document.getElementById('sector-select-modal');
    if (sectorModal) sectorModal.classList.add('hidden');
    const strikeBanner = document.getElementById('strike-banner');
    if (strikeBanner) strikeBanner.classList.add('hidden');
    const slalomBanner = document.getElementById('slalom-banner');
    if (slalomBanner) slalomBanner.classList.add('hidden');
    const milestoneBanner = document.getElementById('milestone-banner');
    if (milestoneBanner) milestoneBanner.classList.add('hidden');

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

  function toggleKickModel() {
    Tuning.KickModel = Tuning.KickModel === 'ReactionRecoil' ? 'CarryAlong' : 'ReactionRecoil';
    updateHUD();
  }

  // --- Fixed 120Hz Physics Tick ---
  let doomCheckAccumulator = 0;

  function physicsTick(dt) {
    if (dt <= 0) return;

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
      State.KineticPushCharge = Math.min(1.0, State.KineticPushCharge + dt * Tuning.KineticChargeRatePerSec);
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

    // 5. Exponential decay on internal lateral dodging kicks
    if (Tuning.LateralDampingPerSec > 0) {
      const decay = Math.exp(-Tuning.LateralDampingPerSec * dt);
      State.LateralVelocity.y *= decay;
      State.LateralVelocity.z *= decay;
      if (Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z) < 0.1) {
        State.LateralVelocity.y = 0;
        State.LateralVelocity.z = 0;
      }
    }

    // 6. Update System Center of Mass World Position
    State.CoMPosition.x += State.CoMVelocity.x * dt;
    State.CoMPosition.y += (State.CoMVelocity.y + State.LateralVelocity.y) * dt;
    State.CoMPosition.z += (State.CoMVelocity.z + State.LateralVelocity.z) * dt;

    // 7. Collision Evaluations (Appendage Deflections & Body Crashes)
    evaluateContinuousCollisions();

    // 7.5. Step 7 Proximity Radar Alarm & Engine Hum Audio
    let minObsDistCm = 999999.0;
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
      }
    }
    updateProximityRadarAlarm(minObsDistCm, performance.now() / 1000.0);
    updateContinuousEngineHumAudio(State.CoMVelocity.x, Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z));

    // 8. Slalom Gate Evasions
    evaluateSlalomCrossings();

    // 9. Step 6: Flight Telemetry & Live Composite Scoring
    State.ActiveFlightTimeSeconds += dt;
    const currentTotalSpeed = Math.hypot(
      State.CoMVelocity.x,
      State.CoMVelocity.y + State.LateralVelocity.y,
      State.CoMVelocity.z + State.LateralVelocity.z
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
      evaluateBackgroundDoomLookahead();
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
      const timeSinceLastDeflection = (performance.now() - State.LastStrikeTime) / 1000.0;
      if (!bDeflectedByAppendage && timeSinceLastDeflection > 0.12) {
        const distToBody = astronautWorldPos.distanceTo(obs.position);
        if (distToBody <= (obs.radius + astronautBodyRadius)) {
          const totalSpeed = Math.hypot(
            State.CoMVelocity.x,
            State.CoMVelocity.y + State.LateralVelocity.y,
            State.CoMVelocity.z + State.LateralVelocity.z
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

  function showSlalomBanner(isSuccess, target, clearance) {
    const banner = document.getElementById('slalom-banner');
    const icon = document.getElementById('slalom-icon');
    const title = document.getElementById('slalom-title');
    const detail = document.getElementById('slalom-detail');

    if (banner && title && detail) {
      if (isSuccess) {
        banner.className = 'slalom-banner success';
        icon.textContent = '✓';
        const streakText = State.CleanEvasionStreak > 1 ? ` (STREAK ${State.CleanEvasionStreak}x!)` : '';
        title.textContent = `CLEAN EVASION!${streakText}`;
        detail.textContent = `${target.name} Cleared: +${clearance.toFixed(0)} cm clearance!`;
      } else {
        banner.className = 'slalom-banner graze';
        icon.textContent = '⚠';
        title.textContent = 'GRAZED BOUNDARY!';
        detail.textContent = `${target.name} Missed by ${clearance.toFixed(0)} cm! Streak reset.`;
      }

      banner.classList.remove('hidden');
      setTimeout(() => { banner.classList.add('hidden'); }, 2200);
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
    const banner = document.getElementById('milestone-banner');
    const titleEl = document.getElementById('milestone-title');
    const detailEl = document.getElementById('milestone-detail');
    if (banner && titleEl && detailEl) {
      titleEl.textContent = 'CELESTIAL MILESTONE REACHED!';
      detailEl.textContent = `${title} • +${bonus} PTS BONUS!`;
      banner.classList.remove('hidden');
      setTimeout(() => { banner.classList.add('hidden'); }, 2800);
    }
  }

  // --- Step 6: Sector Run Victory & Jump Portal Completion ---
  function completeSectorRun() {
    if (State.RunPhase !== 'Running') return;
    State.RunPhase = 'Completed';

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
    const desc = document.getElementById('victory-grade-desc');

    if (circle && letter && title && desc) {
      circle.className = `grade-circle ${gradeClass}`;
      letter.textContent = grade;
      title.textContent = gradeTitle;
      desc.textContent = gradeDesc;
    }

    const timeVal = document.getElementById('victory-time-val');
    if (timeVal) timeVal.textContent = `${timeSec.toFixed(1)}s (Par: ${currentSector.parTimeSeconds.toFixed(1)}s)`;

    const avgVal = document.getElementById('victory-avg-speed-val');
    if (avgVal) avgVal.textContent = `${Math.round(avgSpeedCms)} cm/s (${(avgSpeedCms * 0.036).toFixed(1)} km/h)`;

    const peakVal = document.getElementById('victory-peak-speed-val');
    if (peakVal) peakVal.textContent = `${Math.round(peakSpeedCms)} cm/s (${(peakSpeedCms * 0.036).toFixed(1)} km/h)`;

    const gatesVal = document.getElementById('victory-gates-val');
    const pct = SlalomTargets.length > 0 ? Math.round((State.CleanGatesCleared / SlalomTargets.length) * 100) : 100;
    if (gatesVal) gatesVal.textContent = `${State.CleanGatesCleared} / ${SlalomTargets.length} (${pct}% CLEAN)`;

    const strikesVal = document.getElementById('victory-strikes-val');
    if (strikesVal) strikesVal.textContent = `${State.StrikeCount} LANDED (+${State.StrikeCount * 300} PTS)`;

    const scoreVal = document.getElementById('victory-score-val');
    if (scoreVal) scoreVal.textContent = `${finalScore.toLocaleString()} PTS`;

    const pbTag = document.getElementById('leaderboard-pb-tag');
    if (pbTag) pbTag.classList.toggle('hidden', !isNewPB);

    // Populate Leaderboard table
    const tbody = document.getElementById('leaderboard-tbody');
    if (tbody) {
      tbody.innerHTML = '';
      const entries = profile.leaderboards[State.CurrentSectorIndex] || [];
      entries.forEach((e, i) => {
        const tr = document.createElement('tr');
        if (i === 0) tr.className = 'rank-1';
        tr.innerHTML = `
          <td>${i + 1}</td>
          <td>${e.callsign}</td>
          <td>${e.score.toLocaleString()}</td>
          <td><span class="badge ${e.grade.startsWith('S') ? 'badge-amber' : ''}">${e.grade}</span></td>
          <td>${e.time}</td>
          <td>${e.speed}</td>
          <td>${e.date}</td>
        `;
        tbody.appendChild(tr);
      });
    }

    // Configure Next Sector button
    const nextBtn = document.getElementById('btn-next-sector');
    if (nextBtn) {
      if (State.CurrentSectorIndex < 3) {
        nextBtn.textContent = `WARP TO SECTOR ${State.CurrentSectorIndex + 1} ❯`;
        nextBtn.style.display = 'block';
      } else {
        nextBtn.textContent = 'ALL SECTORS MASTERED ★';
        nextBtn.style.display = 'block';
      }
    }

    // Play victorious jump portal fanfare & vacuum collapse
    playPortalWarpVacuumCollapse();
    playPortalWarpFanfare();
    triggerWarpJumpFlash(currentSector.portalThemeColorStr);

    if (modal) modal.classList.remove('hidden');
  }

  function triggerWarpJumpFlash(colorStr) {
    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = `brightness(2.2) drop-shadow(0 0 60px ${colorStr || '#00f0ff'})`;
      setTimeout(() => { container.style.filter = 'none'; }, 350);
    }
  }

  // --- Background 13s–40s Void Doom Trajectory Evaluator ---
  function evaluateBackgroundDoomLookahead() {
    const totalMass = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const maxReachCm = (Tuning.AstronautMassKg / totalMass) * Tuning.TetherLengthCm + Tuning.ReachToleranceCm;

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
  let astronautAvatar, orbitRingMesh, tangentArrow, kickArrow, trajectoryLine;
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

  // Step 8: Colossal Asteroid Belt, Multi-Angle Hazards & Kinetic Flux Beam Entities
  let colossalBeltGroup = null;
  let colossalBackgroundAsteroids = [];
  let colossalIncomingAsteroids = [];
  let kineticFluxLine = null;
  let kineticShockwaveMesh = null;
  let aimRaycaster = new THREE.Raycaster();
  let mouseNormalized = new THREE.Vector2(0, 0);
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

    // Debug Arrows
    tangentArrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 1), new THREE.Vector3(), 50, 0xffe600, 15, 8);
    kickArrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 1), new THREE.Vector3(), 60, 0x10b981, 18, 10);
    scene.add(tangentArrow);
    scene.add(kickArrow);

    // Trajectory Line (Blind Math default hidden)
    const trajGeo = new THREE.BufferGeometry();
    const trajMat = new THREE.LineBasicMaterial({ color: 0x00f0ff, linewidth: 2.5, transparent: true, opacity: 0.85 });
    trajectoryLine = new THREE.Line(trajGeo, trajMat);
    trajectoryLine.visible = false;
    scene.add(trajectoryLine);

    // Kinetic Flux Beam Linking Astronaut to Locked Asteroid
    const fluxGeo = new THREE.BufferGeometry().setFromPoints([
      new THREE.Vector3(0, 0, 0),
      new THREE.Vector3(0, 0, 0)
    ]);
    const fluxMat = new THREE.LineBasicMaterial({
      color: 0x00f0ff,
      linewidth: 3.5,
      transparent: true,
      opacity: 0.9,
      blending: THREE.AdditiveBlending
    });
    kineticFluxLine = new THREE.Line(fluxGeo, fluxMat);
    kineticFluxLine.visible = false;
    scene.add(kineticFluxLine);

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
    window.addEventListener('mousemove', onMouseMoveAim);
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
    State.KineticTarget = null;
    State.CandidateTarget = null;
    State.KineticLockStatus = 'NoTarget';
    if (kineticFluxLine) kineticFluxLine.visible = false;
  }

  // --- Step 8: Colossal Asteroids, Belt Vista & Kinetic Push Mechanics ---
  const ColossalGreekLetters = ['Alpha', 'Beta', 'Gamma', 'Delta', 'Epsilon', 'Zeta', 'Theta', 'Iota', 'Kappa', 'Lambda', 'Sigma', 'Omega', 'Titan', 'Ares', 'Gorgon'];
  let colossalAsteroidSeq = 1;

  function spawnIncomingColossalAsteroid(spawnForwardX) {
    if (!colossalBeltGroup) return;

    // Radius between 450cm and 1800cm (9m to 36m diameter), scaled by tuning knob
    const baseRadius = (450 + Math.random() * 1350) * Tuning.ColossalAsteroidScale;
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

    const mesh = new THREE.Mesh(geo, mat);

    // Position: forward at spawnForwardX, radial offset in Y, Z
    const offsetTheta = Math.random() * Math.PI * 2;
    const offsetR = 250 + Math.random() * 1800;
    const spawnY = State.CoMPosition.y + Math.cos(offsetTheta) * offsetR;
    const spawnZ = State.CoMPosition.z + Math.sin(offsetTheta) * offsetR;
    mesh.position.set(spawnForwardX, spawnY, spawnZ);
    mesh.rotation.set(Math.random() * Math.PI, Math.random() * Math.PI, Math.random() * Math.PI);
    colossalBeltGroup.add(mesh);

    // Velocity: MUST have at least one component from the forward (vx < 0)
    // and can approach from ANY radial angle
    const speed = Tuning.IncomingAsteroidSpeedCmPerSec * (0.85 + Math.random() * 0.35);
    // Polar angle theta from negative X axis (0 to 28 degrees)
    const angleFromForward = (Math.random() * 28.0) * (Math.PI / 180.0);
    const azim = Math.random() * Math.PI * 2;

    const vx = -speed * Math.cos(angleFromForward);
    const vy = speed * Math.sin(angleFromForward) * Math.cos(azim);
    const vz = speed * Math.sin(angleFromForward) * Math.sin(azim);

    // Cubic mass scaling: M = 4/3 * pi * (r_m)^3 * density (2600 kg/m^3)
    const radiusMeters = baseRadius * 0.01;
    const volumeM3 = (4.0 / 3.0) * Math.PI * Math.pow(radiusMeters, 3);
    const massKg = Math.round(volumeM3 * 2600.0);

    const nameLetter = ColossalGreekLetters[(colossalAsteroidSeq++) % ColossalGreekLetters.length];
    const designation = `ASTEROID [${nameLetter}-${Math.floor(10 + Math.random() * 90)}]`;

    colossalIncomingAsteroids.push({
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

    // 3. Kinetic Flux Line Mesh (Visual link between player and target)
    if (!kineticFluxLine) {
      const lineGeo = new THREE.BufferGeometry().setFromPoints([
        new THREE.Vector3(0, 0, 0),
        new THREE.Vector3(0, 0, 100)
      ]);
      const lineMat = new THREE.LineBasicMaterial({
        color: 0x00f0ff,
        linewidth: 3,
        transparent: true,
        opacity: 0.85,
        blending: THREE.AdditiveBlending
      });
      kineticFluxLine = new THREE.Line(lineGeo, lineMat);
      kineticFluxLine.visible = false;
      scene.add(kineticFluxLine);
    }

    // 4. Kinetic Shockwave Ring Mesh
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
    for (const bg of colossalBackgroundAsteroids) {
      bg.mesh.rotation.x += bg.rotSpeed.x * dt;
      bg.mesh.rotation.y += bg.rotSpeed.y * dt;
      bg.mesh.rotation.z += bg.rotSpeed.z * dt;
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
        const relSpeed = ast.velocity.length();
        triggerKineticCrash({ name: ast.name, isAsteroid: true }, relSpeed);
        return;
      }

      // Despawn if far behind player or deflected far out
      if (ast.mesh.position.x < State.CoMPosition.x - 6000 || ast.mesh.position.length() > 90000) {
        if (State.KineticTarget === ast) {
          unlockKineticTarget();
        }
        colossalBeltGroup.remove(ast.mesh);
        if (ast.mesh.geometry) ast.mesh.geometry.dispose();
        if (ast.mesh.material) ast.mesh.material.dispose();
        colossalIncomingAsteroids.splice(i, 1);
      }
    }

    // Maintain 4 to 6 oncoming colossal asteroids
    colossalSpawnTimer += dt;
    if (colossalIncomingAsteroids.length < 5 && colossalSpawnTimer >= 1.8) {
      colossalSpawnTimer = 0;
      spawnIncomingColossalAsteroid(State.CoMPosition.x + 16000 + Math.random() * 4000);
    }

    // 3. Update expanding kinetic shockwave ring
    if (kineticShockwaveMesh && kineticShockwaveMesh.visible) {
      kineticShockwaveMesh.scale.addScalar(dt * 35.0);
      kineticShockwaveMesh.material.opacity -= dt * 2.8;
      if (kineticShockwaveMesh.material.opacity <= 0.0) {
        kineticShockwaveMesh.visible = false;
      }
    }
  }

  function updateKineticTargetingAndAim() {
    const playerPos = new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );

    const reticleEl = document.getElementById('kinetic-target-reticle');
    const dilemmaEl = document.getElementById('target-proximity-dilemma');
    const desigEl = document.getElementById('target-designation');
    const rangeEl = document.getElementById('target-range');
    const speedEl = document.getElementById('target-speed');
    const ttiEl = document.getElementById('target-tti');
    const sizeEl = document.getElementById('target-size');

    aimRaycaster.setFromCamera(mouseNormalized, camera);

    if (State.KineticLockStatus !== 'Locked') {
      let bestCandidate = null;
      let minAngularDist = 0.45; // ~25 deg cone

      for (const ast of colossalIncomingAsteroids) {
        if (ast.destroyed) continue;
        const toAst = new THREE.Vector3().subVectors(ast.mesh.position, camera.position);
        const dist = toAst.length();
        if (dist > 30000 || toAst.x < 0) continue;

        const dirToAst = toAst.normalize();
        const rayDir = aimRaycaster.ray.direction;
        const angle = rayDir.angleTo(dirToAst);

        if (angle < minAngularDist) {
          minAngularDist = angle;
          bestCandidate = ast;
        }
      }

      State.CandidateTarget = bestCandidate;
      if (reticleEl) reticleEl.classList.add('hidden');
      if (kineticFluxLine) kineticFluxLine.visible = false;
      return;
    }

    // We ARE Locked to State.KineticTarget
    const target = State.KineticTarget;
    if (!target || target.destroyed || target.mesh.position.x < State.CoMPosition.x - 1500) {
      unlockKineticTarget();
      return;
    }

    const toTarget = new THREE.Vector3().subVectors(target.mesh.position, playerPos);
    const distCm = toTarget.length();
    target.rangeCm = distCm;

    const relVel = new THREE.Vector3(
      target.velocity.x - State.CoMVelocity.x,
      target.velocity.y - (State.CoMVelocity.y + State.LateralVelocity.y),
      target.velocity.z - (State.CoMVelocity.z + State.LateralVelocity.z)
    );

    const unitToTarget = toTarget.clone().normalize();
    const closingSpeedCms = -relVel.dot(unitToTarget);
    target.closingSpeedCms = closingSpeedCms;

    const tti = closingSpeedCms > 10.0 ? (distCm / closingSpeedCms) : 99.9;
    target.ttiSeconds = tti;

    const bIsCritical = distCm <= Tuning.CriticalDangerDistanceCm;
    target.bIsCritical = bIsCritical;

    const screenPos = target.mesh.position.clone().project(camera);

    if (screenPos.z > 1.0) {
      if (reticleEl) reticleEl.classList.add('hidden');
    } else {
      const screenX = (screenPos.x * 0.5 + 0.5) * window.innerWidth;
      const screenY = (-(screenPos.y * 0.5) + 0.5) * window.innerHeight;

      if (reticleEl) {
        reticleEl.style.left = `${screenX}px`;
        reticleEl.style.top = `${screenY}px`;
        reticleEl.classList.remove('hidden');
      }

      if (desigEl) desigEl.textContent = `${target.name} [M=${Math.round(target.massKg / 1000)}T]`;
      if (rangeEl) rangeEl.textContent = `${(distCm * 0.01).toFixed(1)}m`;
      if (speedEl) speedEl.textContent = `${(closingSpeedCms * 0.036).toFixed(1)} km/h`;
      if (ttiEl) ttiEl.textContent = tti < 99 ? `${tti.toFixed(1)}s` : 'CLEAR';
      if (sizeEl) sizeEl.textContent = `Ø ${(target.radius * 0.02).toFixed(1)}m`;

      if (dilemmaEl) {
        dilemmaEl.classList.toggle('hidden', !bIsCritical);
      }
    }

    // Update 3D Kinetic Flux Beam
    if (kineticFluxLine) {
      kineticFluxLine.visible = true;
      const linePositions = kineticFluxLine.geometry.attributes.position.array;
      linePositions[0] = playerPos.x + 25;
      linePositions[1] = playerPos.y;
      linePositions[2] = playerPos.z;
      linePositions[3] = target.mesh.position.x;
      linePositions[4] = target.mesh.position.y;
      linePositions[5] = target.mesh.position.z;
      kineticFluxLine.geometry.attributes.position.needsUpdate = true;

      if (State.bIsChargingKineticPush) {
        kineticFluxLine.material.color.setHex(bIsCritical ? 0xffb703 : 0x00f0ff);
        kineticFluxLine.material.opacity = 0.5 + State.KineticPushCharge * 0.5;
      } else {
        kineticFluxLine.material.color.setHex(0x00f0ff);
        kineticFluxLine.material.opacity = 0.45;
      }
    }
  }

  function lockKineticTarget() {
    if (State.CandidateTarget) {
      State.KineticTarget = State.CandidateTarget;
      State.KineticLockStatus = 'Locked';
      playTargetLockBeep(false);
      updateHUD();
    }
  }

  function unlockKineticTarget() {
    State.KineticTarget = null;
    State.KineticLockStatus = 'NoTarget';
    const reticle = document.getElementById('kinetic-target-reticle');
    if (reticle) reticle.classList.add('hidden');
    if (kineticFluxLine) kineticFluxLine.visible = false;
    updateHUD();
  }

  function startChargingKineticPush() {
    if (State.RunPhase !== 'Running') return;
    State.bIsChargingKineticPush = true;
  }

  function releaseAndDeployKineticPush() {
    if (!State.bIsChargingKineticPush) return;
    State.bIsChargingKineticPush = false;

    const charge = Math.max(0.12, State.KineticPushCharge);
    State.KineticPushCharge = 0.0;

    if (State.KineticLockStatus !== 'Locked' || !State.KineticTarget) {
      return;
    }

    const target = State.KineticTarget;
    const playerPos = new THREE.Vector3(
      State.CoMPosition.x,
      State.CoMPosition.y + State.InternalDisplacement.y,
      State.CoMPosition.z + State.InternalDisplacement.z
    );

    const toTarget = new THREE.Vector3().subVectors(target.mesh.position, playerPos);
    const distCm = toTarget.length();
    const unitPush = toTarget.clone().normalize();

    const forceKN = charge * Tuning.MaxPushForceKiloNewtons;
    const impulseNs = forceKN * 1000.0 * 0.22;

    // --- PROXIMITY DILEMMA CHECK ---
    // If closer than critical distance (18m) and push charge < 70%:
    // "IT WILL PUSH IT" — asteroid's colossal inertia overpowers player repulsion!
    if (distCm <= Tuning.CriticalDangerDistanceCm && charge < 0.70) {
      playAsteroidOverpoweredSound();
      triggerAsteroidOverpowerFlash();

      State.CoMVelocity.x -= 820.0 * Tuning.OverpowerRatio;
      State.LateralVelocity.y += (Math.random() - 0.5) * 600.0;
      State.LateralVelocity.z += (Math.random() - 0.5) * 400.0;
      State.CameraTrauma = 1.0;

      spawnKineticSparks(playerPos, 40);
      unlockKineticTarget();
      return;
    }

    // --- SUCCESSFUL KINETIC PUSH DEFLECTION ---
    const deltaVTargetCms = (impulseNs / target.massKg) * 100.0;
    target.velocity.x += unitPush.x * deltaVTargetCms;
    target.velocity.y += unitPush.y * deltaVTargetCms;
    target.velocity.z += unitPush.z * deltaVTargetCms;

    target.rotSpeed.x += (Math.random() - 0.5) * 0.25;
    target.rotSpeed.y += (Math.random() - 0.5) * 0.25;

    const playerMassKg = Tuning.AstronautMassKg + Tuning.MaceMassKg;
    const deltaVPlayerCms = (impulseNs * Tuning.ImpulseEfficiencyAlpha / playerMassKg) * 100.0;

    State.CoMVelocity.x -= unitPush.x * deltaVPlayerCms * 0.65;
    State.LateralVelocity.y -= unitPush.y * deltaVPlayerCms;
    State.LateralVelocity.z -= unitPush.z * deltaVPlayerCms;

    playKineticPushBlastSound(forceKN);
    triggerPushDeflectionFlash(forceKN);
    spawnKineticSparks(target.mesh.position, 55);

    if (kineticShockwaveMesh) {
      kineticShockwaveMesh.position.copy(target.mesh.position);
      kineticShockwaveMesh.lookAt(playerPos);
      kineticShockwaveMesh.scale.set(1, 1, 1);
      kineticShockwaveMesh.material.opacity = 1.0;
      kineticShockwaveMesh.visible = true;
    }

    State.CameraTrauma = Math.min(1.0, 0.45 + charge * 0.4);
    State.StrikeCount++;
  }

  function triggerPushDeflectionFlash(forceKN) {
    const banner = document.getElementById('push-deflection-banner');
    const detail = document.getElementById('push-detail');
    if (banner && detail) {
      detail.textContent = `Delivered +${forceKN.toFixed(1)} kN kinetic repulsion! Asteroid deflected with reaction recoil.`;
      banner.classList.remove('hidden');
      setTimeout(() => { banner.classList.add('hidden'); }, 2200);
    }
    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = 'brightness(1.8) drop-shadow(0 0 45px rgba(0,240,255,0.9))';
      setTimeout(() => { container.style.filter = 'none'; }, 160);
    }
  }

  function triggerAsteroidOverpowerFlash() {
    const banner = document.getElementById('asteroid-overpower-banner');
    if (banner) {
      banner.classList.remove('hidden');
      setTimeout(() => { banner.classList.add('hidden'); }, 2600);
    }
    const container = document.getElementById('canvas-container');
    if (container) {
      container.style.filter = 'brightness(2.2) drop-shadow(0 0 60px rgba(239,68,68,0.95)) hue-rotate(300deg)';
      setTimeout(() => { container.style.filter = 'none'; }, 220);
    }
  }

  function onMouseMoveAim(e) {
    mouseNormalized.x = (e.clientX / window.innerWidth) * 2 - 1;
    mouseNormalized.y = -(e.clientY / window.innerHeight) * 2 + 1;

    const crosshair = document.getElementById('aim-crosshair-3d');
    if (crosshair && Tuning.ActiveScreenMode === 'ColossalBelt') {
      crosshair.style.left = `${e.clientX}px`;
      crosshair.style.top = `${e.clientY}px`;
      crosshair.classList.toggle('has-candidate', !!State.CandidateTarget);
    }
  }

  function setScreenMode(mode) {
    Tuning.ActiveScreenMode = mode;
    const btnGauntlet = document.getElementById('btn-screen-gauntlet');
    const btnColossal = document.getElementById('btn-screen-colossal');
    const colossalBanner = document.getElementById('colossal-mode-banner');
    const aimCrosshair = document.getElementById('aim-crosshair-3d');
    const kineticCard = document.getElementById('card-kinetic-target');
    const pushCard = document.getElementById('card-push-force');

    if (mode === 'ColossalBelt') {
      if (btnGauntlet) btnGauntlet.classList.remove('active');
      if (btnColossal) btnColossal.classList.add('active');
      if (colossalBanner) colossalBanner.classList.remove('hidden');
      if (aimCrosshair) aimCrosshair.classList.remove('hidden');
      if (kineticCard) kineticCard.classList.remove('hidden');
      if (pushCard) pushCard.classList.remove('hidden');

      const launchBanner = document.getElementById('launch-guidance-banner');
      if (launchBanner) launchBanner.classList.add('hidden');

      loadSector(State.CurrentSectorIndex);
    } else {
      if (btnGauntlet) btnGauntlet.classList.add('active');
      if (btnColossal) btnColossal.classList.remove('active');
      if (colossalBanner) colossalBanner.classList.add('hidden');
      if (aimCrosshair) aimCrosshair.classList.add('hidden');
      const reticle = document.getElementById('kinetic-target-reticle');
      if (reticle) reticle.classList.add('hidden');

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
    const container = document.getElementById('canvas-container');
    container.style.filter = 'brightness(1.4) drop-shadow(0 0 24px rgba(0,240,255,0.7))';
    setTimeout(() => { container.style.filter = 'none'; }, 130);
  }

  // --- Render Loop ---
  let lastTime = performance.now();

  function animate() {
    requestAnimationFrame(animate);

    const now = performance.now();
    let dt = (now - lastTime) / 1000.0;
    lastTime = now;
    if (dt > 0.05) dt = 0.05;

    // 120Hz Substepping
    const substeps = 4;
    const subDt = dt / substeps;
    for (let s = 0; s < substeps; s++) {
      physicsTick(subDt);
    }

    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      updateColossalAsteroidBelt(dt);
      updateKineticTargetingAndAim();
    }

    updateHyperspaceSpeedDust(dt);
    updateKineticSparks(dt);
    update3DVisuals();
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
    const latMag = Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z);
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
        const velY = State.CoMVelocity.y + State.LateralVelocity.y;
        const velZ = State.CoMVelocity.z + State.LateralVelocity.z;
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

    // 7. Debug Gizmo Visibility (Strict Rule 4 Blind Math)
    const bDrawGizmos = !State.bBlindMath && State.bShowGizmos;
    tangentArrow.visible = bDrawGizmos;
    kickArrow.visible = bDrawGizmos;
    trajectoryLine.visible = bDrawGizmos;

    if (bDrawGizmos) {
      const sign = State.AngularVelocityRadPerSec >= 0 ? 1.0 : -1.0;
      const tanY = -sinT * sign;
      const tanZ = cosT * sign;
      const tanSpeed = Math.abs(State.AngularVelocityRadPerSec) * Tuning.TetherLengthCm;

      tangentArrow.position.copy(maceWorldPos);
      tangentArrow.setDirection(new THREE.Vector3(0, tanY, tanZ).normalize());
      tangentArrow.setLength(Math.max(1, tanSpeed * 0.12), 15, 8);

      const modelMult = Tuning.KickModel === 'ReactionRecoil' ? -1.0 : 1.0;
      kickArrow.position.copy(astronautAvatar.position);
      kickArrow.setDirection(new THREE.Vector3(0, tanY * modelMult, tanZ * modelMult).normalize());
      kickArrow.setLength(Math.max(1, (tanSpeed * 0.15)), 18, 10);

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
    const fwdSpeed = State.CoMVelocity.x || 3000.0;
    const camSpeedRatio = Math.min(1.5, Math.max(0, (fwdSpeed - 2000.0) / 2500.0));
    const targetArmDist = 650.0 + camSpeedRatio * 120.0;
    State.CameraDistance += (targetArmDist - State.CameraDistance) * 0.12;

    const targetCamX = State.CoMPosition.x - State.CameraDistance;
    const targetCamY = State.CoMPosition.y * 0.45;
    const targetCamZ = State.CoMPosition.z * 0.45 + 120;
    camera.position.x += (targetCamX - camera.position.x) * 0.18;
    camera.position.y += (targetCamY - camera.position.y) * 0.18;
    camera.position.z += (targetCamZ - camera.position.z) * 0.18;
    camera.lookAt(State.CoMPosition.x + 800, State.CoMPosition.y * 0.8, State.CoMPosition.z * 0.8);

    // Dutch roll banking during lateral dodge/momentum kick
    const totalLatY = State.CoMVelocity.y + State.LateralVelocity.y;
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
      starsParticles.position.x = State.CoMPosition.x;
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

          const isLocked = (State.KineticTarget === ast);
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
      const velY = State.CoMVelocity.y + State.LateralVelocity.y;
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
    HUD.ready = true;
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
      State.CoMVelocity.y + State.LateralVelocity.y,
      State.CoMVelocity.z + State.LateralVelocity.z
    );
    setHudText(HUD.speedVal, String(Math.round(speedCms * 0.036)));

    // Power arc: launch charge / push charge (Colossal) / spin energy in flight
    let powerRatio;
    if (!isCorridor) {
      powerRatio = State.KineticPushCharge;
    } else if (State.RunPhase === 'LaunchPushOff') {
      powerRatio = State.LaunchPushCharge;
    } else {
      const tanSpeed = Math.abs(State.AngularVelocityRadPerSec) * (Tuning.TetherLengthCm * 0.01);
      powerRatio = (0.5 * Tuning.MaceMassKg * tanSpeed * tanSpeed) / 4500.0;
    }
    powerRatio = Math.max(0, Math.min(1, powerRatio));
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
      State.CoMVelocity.y + State.LateralVelocity.y,
      State.CoMVelocity.z + State.LateralVelocity.z
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
    const latMag = Math.hypot(State.CoMVelocity.y + State.LateralVelocity.y, State.CoMVelocity.z + State.LateralVelocity.z);
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

    // --- Launch Guidance Banner Updates ---
    const aimTextEl = document.getElementById('launch-aim-text');
    if (aimTextEl && State.RunPhase === 'LaunchPushOff') {
      const dirText = State.LaunchAimAngleDeg < -1.5 ? 'LEFT CORRIDOR' : State.LaunchAimAngleDeg > 1.5 ? 'RIGHT CORRIDOR' : 'CENTER CORRIDOR';
      aimTextEl.textContent = `DEPARTURE HEADING: ${State.LaunchAimAngleDeg >= 0 ? '+' : ''}${State.LaunchAimAngleDeg.toFixed(1)}° [${dirText}]`;
    }

    // Step 8: Kinetic Target & Push Force Card Updates
    const targetStatusEl = document.getElementById('stat-target-status');
    const targetMetricsEl = document.getElementById('stat-target-metrics');
    if (targetStatusEl && targetMetricsEl) {
      if (State.KineticLockStatus === 'Locked' && State.KineticTarget) {
        targetStatusEl.textContent = 'TARGET LOCKED';
        targetStatusEl.className = 'metric-value highlight-gold';
        const rangeM = (State.KineticTarget.rangeCm * 0.01).toFixed(1);
        const ttiS = State.KineticTarget.ttiSeconds < 99 ? State.KineticTarget.ttiSeconds.toFixed(1) + 's' : '---';
        targetMetricsEl.textContent = `Range: ${rangeM}m | TTI: ${ttiS} | Aim: ${State.KineticTarget.name}`;
      } else if (State.CandidateTarget) {
        targetStatusEl.textContent = 'CANDIDATE ACQUIRED';
        targetStatusEl.className = 'metric-value highlight-cyan';
        targetMetricsEl.textContent = `Press [F] or Click to Lock: ${State.CandidateTarget.name}`;
      } else {
        targetStatusEl.textContent = 'NO TARGET';
        targetStatusEl.className = 'metric-value';
        targetMetricsEl.textContent = 'Aim at asteroid to acquire lock';
      }
    }

    const pushForceEl = document.getElementById('stat-push-force');
    const pushStatusEl = document.getElementById('stat-push-status');
    if (pushForceEl && pushStatusEl) {
      const chargePct = Math.round(State.KineticPushCharge * 100);
      const forceKN = (State.KineticPushCharge * Tuning.MaxPushForceKiloNewtons).toFixed(1);
      pushForceEl.textContent = `${forceKN} kN`;
      if (State.bIsChargingKineticPush) {
        pushForceEl.className = 'metric-value highlight-gold';
        pushStatusEl.textContent = `Charging: ${chargePct}% | Release to Fire`;
      } else {
        pushForceEl.className = 'metric-value highlight-cyan';
        pushStatusEl.textContent = `Capacitor Ready (Hold SPACE)`;
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

  // --- Keyboard & Mouse Event Listeners ---
  window.addEventListener('keydown', (e) => {
    if (e.repeat) return;
    const key = e.key.toLowerCase();

    if (e.code === 'Backquote' || key === 'f3') {
      e.preventDefault();
      setDevOverlay(!UI.devOpen);
      return;
    }

    if (key === ' ' || e.code === 'Space') {
      e.preventDefault();
      Input.isSpacePressed = true;
      if (Tuning.ActiveScreenMode === 'ColossalBelt') {
        startChargingKineticPush();
      } else if (State.RunPhase === 'LaunchPushOff') {
        startChargingPush();
      }
    } else if (key === 'f') {
      if (Tuning.ActiveScreenMode === 'ColossalBelt') {
        if (State.KineticLockStatus === 'Locked') {
          unlockKineticTarget();
        } else {
          lockKineticTarget();
        }
      }
    } else if (key === 'a') {
      Input.isAPressed = true;
      Input.lastAPressTime = performance.now();
      evaluateActiveSpin();
    } else if (key === 'd') {
      Input.isDPressed = true;
      Input.lastDPressTime = performance.now();
      evaluateActiveSpin();
    } else if (key === 'm') {
      if (UI.devOpen) toggleKickModel();
    } else if (key === 'r') {
      restartRun();
    }
  });

  window.addEventListener('keyup', (e) => {
    const key = e.key.toLowerCase();

    if (key === ' ' || e.code === 'Space') {
      e.preventDefault();
      Input.isSpacePressed = false;
      if (Tuning.ActiveScreenMode === 'ColossalBelt') {
        releaseAndDeployKineticPush();
      } else if (State.RunPhase === 'LaunchPushOff') {
        fireLaunchPush();
      }
    } else if (key === 'a') {
      Input.isAPressed = false;
      evaluateActiveSpin();
    } else if (key === 'd') {
      Input.isDPressed = false;
      evaluateActiveSpin();
    }
  });

  window.addEventListener('mousemove', onMouseMoveAim);

  window.addEventListener('mousedown', (e) => {
    if (e.target.closest('#dev-overlay') || e.target.closest('#tuning-drawer') || e.target.closest('.hud-panel') || e.target.closest('.screen-switcher-tabs') || e.target.closest('.colossal-mode-actions') || e.target.closest('.modal-backdrop') || e.target.closest('button')) {
      return;
    }
    if (Tuning.ActiveScreenMode === 'ColossalBelt') {
      if (e.button === 0) {
        if (State.KineticLockStatus === 'Locked') {
          unlockKineticTarget();
        } else {
          lockKineticTarget();
        }
      }
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
    bindSlider('slider-alpha', Tuning, 'ImpulseEfficiencyAlpha', 'val-alpha', 2);
    bindSlider('slider-damping', Tuning, 'LateralDampingPerSec', 'val-damping', 2);
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
        Tuning.ImpulseEfficiencyAlpha = 0.65;
        Tuning.LateralDampingPerSec = 1.5;
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
        document.getElementById('slider-alpha').value = 0.65;
        document.getElementById('val-alpha').textContent = '0.65';
        document.getElementById('slider-damping').value = 1.5;
        document.getElementById('val-damping').textContent = '1.50';
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
        Tuning.MaxPushForceKiloNewtons = 50.0;
        Tuning.CriticalDangerDistanceCm = 1800.0;
        Tuning.OverpowerRatio = 1.35;

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

        const sPush = document.getElementById('slider-push-force');
        if (sPush) sPush.value = 50;
        const vPush = document.getElementById('val-push-force');
        if (vPush) vPush.textContent = '50';

        const sCrit = document.getElementById('slider-crit-dist');
        if (sCrit) sCrit.value = 18;
        const vCrit = document.getElementById('val-crit-dist');
        if (vCrit) vCrit.textContent = '18';

        const sOver = document.getElementById('slider-overpower-ratio');
        if (sOver) sOver.value = 1.35;
        const vOver = document.getElementById('val-overpower-ratio');
        if (vOver) vOver.textContent = '1.35';
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

    // Modal retry buttons
    const restartVoidBtn = document.getElementById('btn-restart-void');
    if (restartVoidBtn) restartVoidBtn.addEventListener('click', restartRun);

    const restartCrashBtn = document.getElementById('btn-restart-crash');
    if (restartCrashBtn) restartCrashBtn.addEventListener('click', restartRun);

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
        sectorModal.classList.add('hidden');
      });
    }

    const sectorCards = document.querySelectorAll('.sector-choice-card');
    sectorCards.forEach(card => {
      card.addEventListener('click', () => {
        const idx = parseInt(card.dataset.sector, 10);
        loadSector(idx);
        if (sectorModal) sectorModal.classList.add('hidden');
      });
    });

    // Step 6: Victory Modal Actions
    const nextSectorBtn = document.getElementById('btn-next-sector');
    if (nextSectorBtn) {
      nextSectorBtn.addEventListener('click', () => {
        const victoryModal = document.getElementById('sector-completed-modal');
        if (victoryModal) victoryModal.classList.add('hidden');
        const nextIdx = State.CurrentSectorIndex < 3 ? State.CurrentSectorIndex + 1 : 1;
        loadSector(nextIdx);
      });
    }

    const restartVictoryBtn = document.getElementById('btn-restart-victory');
    if (restartVictoryBtn) {
      restartVictoryBtn.addEventListener('click', () => {
        const victoryModal = document.getElementById('sector-completed-modal');
        if (victoryModal) victoryModal.classList.add('hidden');
        restartRun();
      });
    }

    // Step 8 Sliders & Buttons
    bindSlider('slider-colossal-scale', Tuning, 'ColossalAsteroidScale', 'val-colossal-scale', 1);
    bindSlider('slider-incoming-speed', Tuning, 'IncomingAsteroidSpeedCmPerSec', 'val-incoming-speed', 0);
    bindSlider('slider-push-force', Tuning, 'MaxPushForceKiloNewtons', 'val-push-force', 0);
    bindSlider('slider-overpower-ratio', Tuning, 'OverpowerRatio', 'val-overpower-ratio', 2);

    const sliderBeltRadius = document.getElementById('slider-belt-radius');
    const valBeltRadius = document.getElementById('val-belt-radius');
    if (sliderBeltRadius && valBeltRadius) {
      sliderBeltRadius.addEventListener('input', () => {
        const val = parseFloat(sliderBeltRadius.value);
        Tuning.OuterBeltRadiusCm = val * 100.0;
        valBeltRadius.textContent = val;
      });
    }

    const sliderCritDist = document.getElementById('slider-crit-dist');
    const valCritDist = document.getElementById('val-crit-dist');
    if (sliderCritDist && valCritDist) {
      sliderCritDist.addEventListener('input', () => {
        const val = parseFloat(sliderCritDist.value);
        Tuning.CriticalDangerDistanceCm = val * 100.0;
        valCritDist.textContent = val;
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

    const btnColossalLock = document.getElementById('btn-colossal-lock');
    if (btnColossalLock) {
      btnColossalLock.addEventListener('click', () => {
        if (State.KineticLockStatus === 'Locked') unlockKineticTarget();
        else lockKineticTarget();
      });
    }

    const btnColossalPush = document.getElementById('btn-colossal-push');
    if (btnColossalPush) {
      btnColossalPush.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        startChargingKineticPush();
      });
      window.addEventListener('pointerup', () => {
        if (State.bIsChargingKineticPush) {
          releaseAndDeployKineticPush();
        }
      });
    }

    const btnCloseColossalBanner = document.getElementById('btn-close-colossal-banner');
    if (btnCloseColossalBanner) {
      btnCloseColossalBanner.addEventListener('click', () => {
        const banner = document.getElementById('colossal-mode-banner');
        if (banner) banner.classList.add('hidden');
      });
    }

    // Interactive Launch Button (Touch/Mouse)
    const chargeBtn = document.getElementById('btn-charge-push');
    if (chargeBtn) {
      chargeBtn.addEventListener('pointerdown', (e) => {
        e.preventDefault();
        startChargingPush();
      });
      window.addEventListener('pointerup', () => {
        if (State.bIsChargingPush) {
          fireLaunchPush();
        }
      });
    }
  }

  // Initialize
  init3D();
  setupTuningDrawer();
  if (/[?&]dev=1/.test(window.location.search)) setDevOverlay(true);
  animate();
})();
