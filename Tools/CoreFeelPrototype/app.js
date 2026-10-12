/**
 * SpaceMentum — Interactive Core Feel Prototype & Step 3 Greybox Level Testbed
 * Direct JavaScript mirror of Unreal Engine 5.8 C++ components:
 *   - FTrajectoryPredictor (Solvers/TrajectoryPredictor.cpp)
 *   - UMaceOrbitComponent (Components/MaceOrbitComponent.cpp)
 *   - UMomentumTransferComponent (Components/MomentumTransferComponent.cpp)
 *   - UForwardDriftComponent (Components/ForwardDriftComponent.cpp)
 *   - ASpaceGreyboxArenaActor (Environment/SpaceGreyboxArenaActor.cpp)
 */

(function () {
  'use strict';

  // --- Physics Tuning Knobs (Mirror of C++ Defaults) ---
  const Tuning = {
    // FMaceOrbitTuning
    TetherLengthCm: 160.0,
    MaceMassKg: 40.0,
    SpinAccelerationDegPerSec2: 1440.0,
    MaxAngularSpeedDegPerSec: 1080.0,

    // FMomentumTransferTuning
    AstronautMassKg: 90.0,
    ImpulseEfficiencyAlpha: 0.6,
    KickModel: 'ReactionRecoil', // 'ReactionRecoil' or 'CarryAlong'
    MinAngularSpeedForKickDegPerSec: 90.0,
    LateralDampingPerSec: 1.5,
    MaxLateralSpeedCmPerSec: 1800.0,

    // FForwardDriftTuning
    ForwardSpeedCmPerSec: 3000.0, // 108 km/h sacred forward drift (30 m/s)
  };

  // --- Curated Slalom Hazard Targets (Exact Mirror of ASpaceGreyboxArenaActor) ---
  const SlalomTargets = [
    {
      name: 'Gate 1: Lateral Right (+250cm)',
      forwardX: 6000.0,   // 60m (2.0s mark)
      targetY: 250.0,
      targetZ: 0.0,
      radius: 120.0,
      color: 0xff4747,
      evaluated: false,
      cleared: false,
      meshGroup: null,
    },
    {
      name: 'Gate 2: Counter Whiplash Left (-300cm)',
      forwardX: 12000.0,  // 120m (4.0s mark)
      targetY: -300.0,
      targetZ: 0.0,
      radius: 120.0,
      color: 0xff7828,
      evaluated: false,
      cleared: false,
      meshGroup: null,
    },
    {
      name: 'Gate 3: High Apex Evade (+200cm Up)',
      forwardX: 18000.0,  // 180m (6.0s mark)
      targetY: 0.0,
      targetZ: 200.0,
      radius: 120.0,
      color: 0x28dcff,
      evaluated: false,
      cleared: false,
      meshGroup: null,
    },
    {
      name: 'Gate 4: Diagonal Vector (+220cm R, -180cm Down)',
      forwardX: 24000.0,  // 240m (8.0s mark)
      targetY: 220.0,
      targetZ: -180.0,
      radius: 140.0,
      color: 0xc828ff,
      evaluated: false,
      cleared: false,
      meshGroup: null,
    },
    {
      name: 'Gate 5: Double S-Curve (-280cm L, +150cm Up)',
      forwardX: 30000.0,  // 300m (10.0s mark)
      targetY: -280.0,
      targetZ: 150.0,
      radius: 130.0,
      color: 0xffe628,
      evaluated: false,
      cleared: false,
      meshGroup: null,
    },
  ];

  // --- Live Physics State ---
  const State = {
    // FMaceOrbitState
    OrbitAngleRad: 0.0,
    AngularVelocityRadPerSec: 0.0,
    HeldSpinDirection: 'None', // 'CounterClockwise', 'Clockwise', 'None'

    // Lateral Dodging Velocity (strictly Y and Z)
    LateralVelocity: { y: 0.0, z: 0.0 },

    // Astronaut Position in World (cm)
    Position: { x: 0.0, y: 0.0, z: 0.0 },

    // Dynamic Camera FOV
    CurrentFOV: 80.0,
    TargetFOV: 80.0,

    // Gizmo Visibility
    bShowGizmos: true,
  };

  // --- SOCD Input Tracker ---
  const Input = {
    isAPressed: false,
    isDPressed: false,
    lastAPressTime: 0,
    lastDPressTime: 0,
  };

  // --- Web Audio API Procedural Sound Synthesizer ---
  let audioCtx = null;
  function getAudioContext() {
    if (!audioCtx) {
      const AudioContextClass = window.AudioContext || window.webkitAudioContext;
      if (AudioContextClass) {
        audioCtx = new AudioContextClass();
      }
    }
    if (audioCtx && audioCtx.state === 'suspended') {
      audioCtx.resume();
    }
    return audioCtx;
  }

  function playKickSound() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();

    osc.type = 'triangle';
    osc.frequency.setValueAtTime(180, now);
    osc.frequency.exponentialRampToValueAtTime(35, now + 0.18);

    gain.gain.setValueAtTime(0.6, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.22);

    osc.connect(gain);
    gain.connect(ctx.destination);

    osc.start(now);
    osc.stop(now + 0.22);
  }

  function playClearChime() {
    const ctx = getAudioContext();
    if (!ctx) return;
    const now = ctx.currentTime;

    // Harmonic chime: 880Hz -> 1320Hz
    [880, 1320].forEach((freq, idx) => {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();

      osc.type = 'sine';
      osc.frequency.setValueAtTime(freq, now + idx * 0.06);

      gain.gain.setValueAtTime(0.3, now + idx * 0.06);
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

    osc.type = 'sawtooth';
    osc.frequency.setValueAtTime(75, now);
    osc.frequency.linearRampToValueAtTime(50, now + 0.2);

    gain.gain.setValueAtTime(0.5, now);
    gain.gain.exponentialRampToValueAtTime(0.001, now + 0.25);

    osc.connect(gain);
    gain.connect(ctx.destination);

    osc.start(now);
    osc.stop(now + 0.25);
  }

  function evaluateActiveSpin() {
    let targetDirection = 'None';

    if (Input.isAPressed && Input.isDPressed) {
      // SOCD: Last-Input-Priority
      targetDirection = Input.lastAPressTime > Input.lastDPressTime ? 'CounterClockwise' : 'Clockwise';
    } else if (Input.isAPressed) {
      targetDirection = 'CounterClockwise';
    } else if (Input.isDPressed) {
      targetDirection = 'Clockwise';
    } else {
      targetDirection = 'None';
    }

    setHeldSpinDirection(targetDirection);
  }

  function setHeldSpinDirection(newDirection) {
    if (State.HeldSpinDirection === newDirection) return;

    const previousDirection = State.HeldSpinDirection;
    State.HeldSpinDirection = newDirection;

    // Transition from spinning to None: MOMENTUM KICK FIRES!
    if (previousDirection !== 'None' && newDirection === 'None') {
      executeMomentumTransfer(
        State.OrbitAngleRad,
        State.AngularVelocityRadPerSec,
        Tuning.TetherLengthCm,
        Tuning.MaceMassKg
      );
      // Arrest rotation
      State.AngularVelocityRadPerSec = 0.0;
    }
  }

  // --- Momentum Transfer Solver (C++ Mirror) ---
  function executeMomentumTransfer(angleRad, omegaRadPerSec, tetherCm, maceMassKg) {
    const angularSpeedDeg = Math.abs(omegaRadPerSec) * (180 / Math.PI);
    if (angularSpeedDeg < Tuning.MinAngularSpeedForKickDegPerSec) {
      return; // Ignore micro-taps
    }

    // Tangential direction in YZ plane
    // CCW tangent at angle theta: (-sin(theta), cos(theta))
    const sign = omegaRadPerSec >= 0 ? 1.0 : -1.0;
    const sinT = Math.sin(angleRad);
    const cosT = Math.cos(angleRad);
    const tangentY = -sinT * sign;
    const tangentZ = cosT * sign;

    const tangentialSpeed = Math.abs(omegaRadPerSec) * tetherCm;

    // J = (+/-) alpha * m_m * v_t
    const modelMultiplier = Tuning.KickModel === 'ReactionRecoil' ? -1.0 : 1.0;
    const impulseMag = Tuning.ImpulseEfficiencyAlpha * maceMassKg * tangentialSpeed;

    const impulseY = tangentY * impulseMag * modelMultiplier;
    const impulseZ = tangentZ * impulseMag * modelMultiplier;

    // Delta V = J / m_astronaut
    const deltaVY = impulseY / Tuning.AstronautMassKg;
    const deltaVZ = impulseZ / Tuning.AstronautMassKg;

    State.LateralVelocity.y += deltaVY;
    State.LateralVelocity.z += deltaVZ;

    // Hard clamp to max lateral speed
    const currentSpeed = Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z);
    if (currentSpeed > Tuning.MaxLateralSpeedCmPerSec) {
      const scale = Tuning.MaxLateralSpeedCmPerSec / currentSpeed;
      State.LateralVelocity.y *= scale;
      State.LateralVelocity.z *= scale;
    }

    // Camera FOV recoil kick pulse
    const kickRatio = Math.min(1.0, currentSpeed / 1800.0);
    State.TargetFOV = 80.0 + kickRatio * 18.0;

    // Screen kick flash & Audio punch
    triggerKickFlash();
    playKickSound();
  }

  function toggleKickModel() {
    Tuning.KickModel = Tuning.KickModel === 'ReactionRecoil' ? 'CarryAlong' : 'ReactionRecoil';
    updateHUD();
  }

  function restartRun() {
    State.Position.x = 0.0;
    State.Position.y = 0.0;
    State.Position.z = 0.0;
    State.LateralVelocity.y = 0.0;
    State.LateralVelocity.z = 0.0;
    State.AngularVelocityRadPerSec = 0.0;
    State.OrbitAngleRad = 0.0;
    State.HeldSpinDirection = 'None';
    Input.isAPressed = false;
    Input.isDPressed = false;

    // Reset slalom targets
    for (const target of SlalomTargets) {
      target.evaluated = false;
      target.cleared = false;
      if (target.meshGroup) {
        target.meshGroup.children[0].material.color.setHex(target.color);
        target.meshGroup.children[1].material.color.setHex(target.color);
      }
    }

    const banner = document.getElementById('slalom-banner');
    if (banner) {
      banner.classList.add('hidden');
    }

    updateHUD();
  }

  // --- Fixed 120Hz Physics Tick ---
  function physicsTick(dt) {
    if (dt <= 0) return;

    const accelRad = Tuning.SpinAccelerationDegPerSec2 * (Math.PI / 180);
    const maxSpeedRad = Tuning.MaxAngularSpeedDegPerSec * (Math.PI / 180);

    // 1. Accelerate angular velocity
    if (State.HeldSpinDirection === 'CounterClockwise') {
      if (State.AngularVelocityRadPerSec < 0) {
        State.AngularVelocityRadPerSec += accelRad * 2.0 * dt; // Counter-spin braking
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

    // 2. Integrate orbit angle theta
    if (Math.abs(State.AngularVelocityRadPerSec) > 0.0001) {
      State.OrbitAngleRad += State.AngularVelocityRadPerSec * dt;
      State.OrbitAngleRad = State.OrbitAngleRad % (2 * Math.PI);
      if (State.OrbitAngleRad < 0) State.OrbitAngleRad += 2 * Math.PI;
    }

    // 3. Exact exponential decay on lateral dodging velocity
    if (Tuning.LateralDampingPerSec > 0) {
      const decay = Math.exp(-Tuning.LateralDampingPerSec * dt);
      State.LateralVelocity.y *= decay;
      State.LateralVelocity.z *= decay;
      if (Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z) < 0.1) {
        State.LateralVelocity.y = 0;
        State.LateralVelocity.z = 0;
      }
    }

    // 4. Update astronaut position (Forward +X is continuous 30 m/s)
    State.Position.x += Tuning.ForwardSpeedCmPerSec * dt;
    State.Position.y += State.LateralVelocity.y * dt;
    State.Position.z += State.LateralVelocity.z * dt;

    // 5. Evaluate Slalom Gate Evasions
    evaluateSlalomCrossings();

    // 6. Relax camera FOV
    State.TargetFOV += (80.0 - State.TargetFOV) * Math.min(1.0, dt * 2.5);
    State.CurrentFOV += (State.TargetFOV - State.CurrentFOV) * Math.min(1.0, dt * 10.0);
  }

  function evaluateSlalomCrossings() {
    for (const target of SlalomTargets) {
      if (!target.evaluated && State.Position.x >= target.forwardX) {
        target.evaluated = true;
        const deltaY = State.Position.y - target.targetY;
        const deltaZ = State.Position.z - target.targetZ;
        const clearance = Math.hypot(deltaY, deltaZ);
        target.cleared = clearance > target.radius;

        // Visual feedback on 3D target mesh
        if (target.meshGroup) {
          const evalColor = target.cleared ? 0x10b981 : 0xf43f5e;
          target.meshGroup.children[0].material.color.setHex(evalColor);
          target.meshGroup.children[1].material.color.setHex(evalColor);
        }

        // Show banner and play sound
        showSlalomBanner(target, clearance);
      }
    }
  }

  let bannerTimeout = null;
  function showSlalomBanner(target, clearance) {
    const banner = document.getElementById('slalom-banner');
    const titleEl = document.getElementById('slalom-title');
    const detailEl = document.getElementById('slalom-detail');
    const iconEl = document.getElementById('slalom-icon');
    if (!banner || !titleEl || !detailEl || !iconEl) return;

    if (target.cleared) {
      banner.className = 'slalom-banner';
      iconEl.textContent = '✓';
      titleEl.textContent = 'CLEAN EVASION! PASS';
      detailEl.textContent = `${target.name} | Clearance: ${clearance.toFixed(1)} cm (Req: >${target.radius.toFixed(0)} cm)`;
      playClearChime();
    } else {
      banner.className = 'slalom-banner collision-hazard';
      iconEl.textContent = '✕';
      titleEl.textContent = 'COLLISION HAZARD! GRAZED';
      detailEl.textContent = `${target.name} | Clearance: ${clearance.toFixed(1)} cm | Needs stronger kick!`;
      playGrazeWarning();
    }

    if (bannerTimeout) clearTimeout(bannerTimeout);
    bannerTimeout = setTimeout(() => {
      banner.classList.add('hidden');
    }, 3800);
  }

  // --- Three.js 3D Viewport & Visuals ---
  let scene, camera, renderer;
  let astronautGroup, maceHeadMesh, tetherLine, orbitRingMesh;
  let tangentArrow, kickArrow, trajectoryLine;
  let starsParticles, tunnelRings = [];

  function init3D() {
    const container = document.getElementById('canvas-container');

    // Scene
    scene = new THREE.Scene();
    scene.fog = new THREE.FogExp2(0x05070d, 0.00032);

    // Camera (TPP Chase Cam)
    camera = new THREE.PerspectiveCamera(80, window.innerWidth / window.innerHeight, 10, 60000);
    camera.position.set(-650, 0, 120);

    // Renderer
    renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: 'high-performance' });
    renderer.setSize(window.innerWidth, window.innerHeight);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.toneMapping = THREE.ACESFilmicToneMapping;
    renderer.toneMappingExposure = 1.25;
    container.appendChild(renderer.domElement);

    // High-Contrast Deep Space Lighting Rig (Matching ASpaceGreyboxArenaActor)
    const ambientLight = new THREE.AmbientLight(0x0a1424, 1.4);
    scene.add(ambientLight);

    const keyCosmicLight = new THREE.DirectionalLight(0x00f0ff, 2.8);
    keyCosmicLight.position.set(-400, -300, 600);
    scene.add(keyCosmicLight);

    const rimCosmicLight = new THREE.DirectionalLight(0xbd00ff, 2.2);
    rimCosmicLight.position.set(700, 500, -300);
    scene.add(rimCosmicLight);

    // Astronaut Avatar Group
    astronautGroup = new THREE.Group();
    scene.add(astronautGroup);

    // Astronaut Capsule Body
    const bodyGeo = new THREE.CylinderGeometry(25, 25, 90, 16);
    const bodyMat = new THREE.MeshStandardMaterial({
      color: 0xe2e8f0,
      metalness: 0.85,
      roughness: 0.2,
    });
    const bodyMesh = new THREE.Mesh(bodyGeo, bodyMat);
    bodyMesh.rotation.x = Math.PI / 2; // Lie along +X
    astronautGroup.add(bodyMesh);

    // Visor
    const visorGeo = new THREE.SphereGeometry(20, 16, 16, 0, Math.PI);
    const visorMat = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00a8b8,
      emissiveIntensity: 0.7,
      metalness: 0.9,
      roughness: 0.1,
    });
    const visorMesh = new THREE.Mesh(visorGeo, visorMat);
    visorMesh.position.set(20, 0, 15);
    visorMesh.rotation.y = Math.PI / 2;
    astronautGroup.add(visorMesh);

    // Mace Orbit Ring (YZ plane)
    const ringGeo = new THREE.BufferGeometry();
    const ringPoints = [];
    for (let i = 0; i <= 64; i++) {
      const theta = (i / 64) * Math.PI * 2;
      ringPoints.push(new THREE.Vector3(0, Math.cos(theta) * Tuning.TetherLengthCm, Math.sin(theta) * Tuning.TetherLengthCm));
    }
    ringGeo.setFromPoints(ringPoints);
    const ringMat = new THREE.LineBasicMaterial({ color: 0x38bdf8, transparent: true, opacity: 0.4 });
    orbitRingMesh = new THREE.Line(ringGeo, ringMat);
    astronautGroup.add(orbitRingMesh);

    // Tether Line
    const tetherGeo = new THREE.BufferGeometry().setFromPoints([new THREE.Vector3(), new THREE.Vector3()]);
    const tetherMat = new THREE.LineBasicMaterial({ color: 0x94a3b8, linewidth: 2 });
    tetherLine = new THREE.Line(tetherGeo, tetherMat);
    astronautGroup.add(tetherLine);

    // Mace Head Sphere
    const maceGeo = new THREE.SphereGeometry(18, 24, 24);
    const maceMat = new THREE.MeshStandardMaterial({
      color: 0x00f0ff,
      emissive: 0x00f0ff,
      emissiveIntensity: 0.9,
      roughness: 0.2,
      metalness: 0.9,
    });
    maceHeadMesh = new THREE.Mesh(maceGeo, maceMat);
    astronautGroup.add(maceHeadMesh);

    // Tangential Velocity Arrow (Yellow)
    tangentArrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 1), new THREE.Vector3(), 50, 0xffe600, 15, 8);
    astronautGroup.add(tangentArrow);

    // Instantaneous Kick Preview Arrow (Green)
    kickArrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, 1), new THREE.Vector3(), 60, 0x10b981, 18, 10);
    astronautGroup.add(kickArrow);

    // Trajectory Curve Line (Cyan)
    const trajGeo = new THREE.BufferGeometry();
    const trajMat = new THREE.LineBasicMaterial({ color: 0x00f0ff, linewidth: 2.5, transparent: true, opacity: 0.85 });
    trajectoryLine = new THREE.Line(trajGeo, trajMat);
    scene.add(trajectoryLine);

    // Cosmic Space Dust Particles
    createStarfield();

    // Procedural Greybox Corridor & Slalom Hazard Gates
    createGreyboxCorridor();

    window.addEventListener('resize', onWindowResize);
  }

  function createStarfield() {
    const starCount = 4000;
    const starGeo = new THREE.BufferGeometry();
    const positions = new Float32Array(starCount * 3);

    for (let i = 0; i < starCount * 3; i += 3) {
      positions[i] = (Math.random() - 0.5) * 12000;    // X
      positions[i + 1] = (Math.random() - 0.5) * 6000; // Y
      positions[i + 2] = (Math.random() - 0.5) * 6000; // Z
    }

    starGeo.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    const starMat = new THREE.PointsMaterial({
      color: 0xffffff,
      size: 4,
      transparent: true,
      opacity: 0.75,
    });

    starsParticles = new THREE.Points(starGeo, starMat);
    scene.add(starsParticles);
  }

  function createGreyboxCorridor() {
    const corridorRadius = 800; // cm (8m radius)
    const totalLength = 60000;  // 600 meters
    const ringInterval = 3000;  // 30 meters = 1 second of travel
    const milestoneInterval = 15000; // 150 meters

    const standardRingGeo = new THREE.TorusGeometry(corridorRadius, 6, 8, 36);
    const standardRingMat = new THREE.MeshBasicMaterial({ color: 0x1e293b, transparent: true, opacity: 0.35 });

    const milestoneRingGeo = new THREE.TorusGeometry(corridorRadius, 14, 8, 36);
    const milestoneRingMat = new THREE.MeshBasicMaterial({ color: 0x00f0ff, transparent: true, opacity: 0.85 });

    // 1. Runway Rings
    const ringCount = Math.floor(totalLength / ringInterval);
    for (let i = 0; i <= ringCount; i++) {
      const ringX = i * ringInterval;
      const isMilestone = (ringX % milestoneInterval === 0);

      const ring = new THREE.Mesh(
        isMilestone ? milestoneRingGeo : standardRingGeo,
        isMilestone ? milestoneRingMat : standardRingMat
      );
      ring.rotation.y = Math.PI / 2;
      ring.position.set(ringX, 0, 0);
      scene.add(ring);
      tunnelRings.push(ring);

      // Milestone crosshair alignment tick
      if (isMilestone) {
        const tickGeo = new THREE.BufferGeometry().setFromPoints([
          new THREE.Vector3(ringX, -150, 0), new THREE.Vector3(ringX, 150, 0),
          new THREE.Vector3(ringX, 0, -150), new THREE.Vector3(ringX, 0, 150)
        ]);
        const tickMat = new THREE.LineBasicMaterial({ color: 0x00f0ff, transparent: true, opacity: 0.5 });
        const tickLine = new THREE.LineSegments(tickGeo, tickMat);
        scene.add(tickLine);
      }
    }

    // 2. Slalom Hazard Gates (Matching ASpaceGreyboxArenaActor)
    for (const target of SlalomTargets) {
      const group = new THREE.Group();
      group.position.set(target.forwardX, target.targetY, target.targetZ);

      // Outer Danger Ring
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

      // Floating Hazard Core (Asteroid / Beacon Sphere)
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

      // Tether anchor to outer tunnel wall for depth grounding
      const normY = target.targetY;
      const normZ = target.targetZ;
      const normMag = Math.hypot(normY, normZ) || 1;
      const wallY = (normY / normMag) * corridorRadius;
      const wallZ = (normZ / normMag) * corridorRadius;

      const tetherStemGeo = new THREE.BufferGeometry().setFromPoints([
        new THREE.Vector3(0, 0, 0),
        new THREE.Vector3(0, wallY - target.targetY, wallZ - target.targetZ)
      ]);
      const tetherStemMat = new THREE.LineBasicMaterial({ color: 0x334155, transparent: true, opacity: 0.6 });
      const tetherStem = new THREE.Line(tetherStemGeo, tetherStemMat);
      group.add(tetherStem);

      scene.add(group);
      target.meshGroup = group;
    }
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

  // --- Real-Time Render Loop ---
  let lastTime = performance.now();

  function animate() {
    requestAnimationFrame(animate);

    const now = performance.now();
    let dt = (now - lastTime) / 1000.0;
    lastTime = now;
    if (dt > 0.05) dt = 0.05; // Prevent spiral of death on tab switch

    // 120Hz Substepping
    const substeps = 4;
    const subDt = dt / substeps;
    for (let s = 0; s < substeps; s++) {
      physicsTick(subDt);
    }

    // Update 3D Positions
    update3DVisuals();

    // Update HUD
    updateHUD();

    renderer.render(scene, camera);
  }

  function update3DVisuals() {
    // 1. Astronaut position
    astronautGroup.position.set(State.Position.x, State.Position.y, State.Position.z);

    // 2. Mace Head position on YZ orbit circle: (0, r * cos(theta), r * sin(theta))
    const maceY = Tuning.TetherLengthCm * Math.cos(State.OrbitAngleRad);
    const maceZ = Tuning.TetherLengthCm * Math.sin(State.OrbitAngleRad);
    maceHeadMesh.position.set(0, maceY, maceZ);

    // 3. Dynamic speed-reactive mace head coloration (Cyan -> Magenta)
    const maxSpeedRad = Tuning.MaxAngularSpeedDegPerSec * (Math.PI / 180);
    const speedRatio = Math.min(1.0, Math.abs(State.AngularVelocityRadPerSec) / maxSpeedRad);
    const color = new THREE.Color().lerpColors(new THREE.Color(0x00f0ff), new THREE.Color(0xbd00ff), speedRatio);
    maceHeadMesh.material.color = color;
    maceHeadMesh.material.emissive = color;
    maceHeadMesh.material.emissiveIntensity = 0.5 + speedRatio * 1.5;

    // 4. Tether Line
    const tetherPos = tetherLine.geometry.attributes.position;
    tetherPos.setXYZ(0, 0, 0, 0);
    tetherPos.setXYZ(1, 0, maceY, maceZ);
    tetherPos.needsUpdate = true;

    // 5. Orbit Ring scale if tether length changed
    orbitRingMesh.scale.set(1, Tuning.TetherLengthCm / 160.0, Tuning.TetherLengthCm / 160.0);

    // 6. Tangential Velocity Arrow
    const sign = State.AngularVelocityRadPerSec >= 0 ? 1.0 : -1.0;
    const tanY = -Math.sin(State.OrbitAngleRad) * sign;
    const tanZ = Math.cos(State.OrbitAngleRad) * sign;
    const tanSpeed = Math.abs(State.AngularVelocityRadPerSec) * Tuning.TetherLengthCm;
    tangentArrow.position.set(0, maceY, maceZ);
    tangentArrow.setDirection(new THREE.Vector3(0, tanY, tanZ).normalize());
    tangentArrow.setLength(Math.max(1, tanSpeed * 0.12), 15, 8);
    tangentArrow.visible = State.bShowGizmos && tanSpeed > 10;

    // 7. Instantaneous Kick Prediction Arrow
    const modelMult = Tuning.KickModel === 'ReactionRecoil' ? -1.0 : 1.0;
    const kickDirY = tanY * modelMult;
    const kickDirZ = tanZ * modelMult;
    const estKickDeltaV = (Tuning.ImpulseEfficiencyAlpha * Tuning.MaceMassKg * tanSpeed) / Tuning.AstronautMassKg;
    kickArrow.position.set(0, 0, 0);
    kickArrow.setDirection(new THREE.Vector3(0, kickDirY, kickDirZ).normalize());
    kickArrow.setLength(Math.max(1, estKickDeltaV * 0.25), 18, 10);
    kickArrow.visible = State.bShowGizmos && tanSpeed > 10;

    // 8. Projected Corridor Flight Path (1.5-second analytical trajectory)
    if (State.bShowGizmos) {
      const trajPoints = [];
      const steps = 30;
      const horizonSec = 1.5;
      const stepDt = horizonSec / steps;

      for (let i = 0; i <= steps; i++) {
        const t = i * stepDt;
        const fwdDisp = Tuning.ForwardSpeedCmPerSec * t;
        let latFactor = t;
        if (Tuning.LateralDampingPerSec > 0.001) {
          latFactor = (1.0 - Math.exp(-Tuning.LateralDampingPerSec * t)) / Tuning.LateralDampingPerSec;
        }
        trajPoints.push(new THREE.Vector3(
          State.Position.x + fwdDisp,
          State.Position.y + State.LateralVelocity.y * latFactor,
          State.Position.z + State.LateralVelocity.z * latFactor
        ));
      }
      trajectoryLine.geometry.setFromPoints(trajPoints);
      trajectoryLine.visible = true;
    } else {
      trajectoryLine.visible = false;
    }

    // 9. Camera Chase Rig (elastic boom with lag)
    const targetCamX = State.Position.x - 650;
    const targetCamY = State.Position.y * 0.45; // Subtle lag
    const targetCamZ = State.Position.z * 0.45 + 120;
    camera.position.x += (targetCamX - camera.position.x) * 0.18;
    camera.position.y += (targetCamY - camera.position.y) * 0.18;
    camera.position.z += (targetCamZ - camera.position.z) * 0.18;
    camera.lookAt(State.Position.x + 800, State.Position.y * 0.8, State.Position.z * 0.8);
    camera.fov = State.CurrentFOV;
    camera.updateProjectionMatrix();

    // 10. Starfield follows astronaut
    if (starsParticles) {
      starsParticles.position.x = State.Position.x;
    }

    // 11. Subtle spin on slalom hazard cores
    for (const target of SlalomTargets) {
      if (target.meshGroup && target.meshGroup.children[1]) {
        target.meshGroup.children[1].rotation.x += 0.015;
        target.meshGroup.children[1].rotation.y += 0.02;
      }
    }
  }

  // --- HUD Updates ---
  function updateHUD() {
    // Input state
    const inputEl = document.getElementById('stat-input');
    if (State.HeldSpinDirection === 'CounterClockwise') {
      inputEl.textContent = 'HOLDING A [CCW]';
      inputEl.className = 'metric-value highlight-cyan';
    } else if (State.HeldSpinDirection === 'Clockwise') {
      inputEl.textContent = 'HOLDING D [CW]';
      inputEl.className = 'metric-value highlight-cyan';
    } else {
      inputEl.textContent = 'RELEASE ARMED';
      inputEl.className = 'metric-value highlight-gold';
    }

    // RPM
    const rpm = (Math.abs(State.AngularVelocityRadPerSec) * 60) / (2 * Math.PI);
    const degPerSec = Math.abs(State.AngularVelocityRadPerSec) * (180 / Math.PI);
    document.getElementById('stat-rpm').textContent = `${rpm.toFixed(1)} RPM`;
    document.getElementById('stat-deg-per-sec').textContent = `${degPerSec.toFixed(0)}°/s`;

    // Angle
    const angleDeg = State.OrbitAngleRad * (180 / Math.PI);
    document.getElementById('stat-angle').textContent = `${angleDeg.toFixed(1)}°`;
    let quadrant = 'RIGHT (+Y)';
    if (angleDeg >= 45 && angleDeg < 135) quadrant = 'UP (+Z)';
    else if (angleDeg >= 135 && angleDeg < 225) quadrant = 'LEFT (-Y)';
    else if (angleDeg >= 225 && angleDeg < 315) quadrant = 'DOWN (-Z)';
    document.getElementById('stat-quadrant').textContent = quadrant;

    // Lateral Velocity
    const latMag = Math.hypot(State.LateralVelocity.y, State.LateralVelocity.z);
    document.getElementById('stat-lat-vel').textContent = `${latMag.toFixed(1)} cm/s`;
    document.getElementById('stat-lat-mag').textContent = `Y: ${State.LateralVelocity.y >= 0 ? '+' : ''}${State.LateralVelocity.y.toFixed(1)} | Z: ${State.LateralVelocity.z >= 0 ? '+' : ''}${State.LateralVelocity.z.toFixed(1)}`;

    // Kick Model
    document.getElementById('stat-kick-model').textContent = Tuning.KickModel === 'ReactionRecoil'
      ? 'Reaction Recoil'
      : 'Carry Along';

    // Track Distance & Slalom Cleared
    const distMeters = (State.Position.x / 100.0).toFixed(1);
    const clearedCount = SlalomTargets.filter(t => t.cleared).length;
    const distEl = document.getElementById('stat-track-dist');
    if (distEl) {
      distEl.textContent = `${distMeters} m / 600 m`;
    }
    const clearedEl = document.getElementById('stat-slalom-cleared');
    if (clearedEl) {
      clearedEl.textContent = `${clearedCount} / ${SlalomTargets.length} GATES CLEARED`;
    }

    // Key Pills Highlighting
    document.getElementById('key-a').classList.toggle('active', Input.isAPressed);
    document.getElementById('key-d').classList.toggle('active', Input.isDPressed);
  }

  // --- Keyboard Event Listeners ---
  window.addEventListener('keydown', (e) => {
    if (e.repeat) return;
    const key = e.key.toLowerCase();

    if (key === 'a') {
      Input.isAPressed = true;
      Input.lastAPressTime = performance.now();
      evaluateActiveSpin();
    } else if (key === 'd') {
      Input.isDPressed = true;
      Input.lastDPressTime = performance.now();
      evaluateActiveSpin();
    } else if (key === 'm') {
      toggleKickModel();
    } else if (key === 'r') {
      restartRun();
    }
  });

  window.addEventListener('keyup', (e) => {
    const key = e.key.toLowerCase();

    if (key === 'a') {
      Input.isAPressed = false;
      evaluateActiveSpin();
    } else if (key === 'd') {
      Input.isDPressed = false;
      evaluateActiveSpin();
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
      slider.addEventListener('input', () => {
        const val = parseFloat(slider.value);
        targetObj[prop] = val;
        valDisplay.textContent = stepDecimals ? val.toFixed(stepDecimals) : val;
      });
    }

    bindSlider('slider-tether', Tuning, 'TetherLengthCm', 'val-tether', 0);
    bindSlider('slider-mace-mass', Tuning, 'MaceMassKg', 'val-mace-mass', 0);
    bindSlider('slider-accel', Tuning, 'SpinAccelerationDegPerSec2', 'val-accel', 0);
    bindSlider('slider-max-speed', Tuning, 'MaxAngularSpeedDegPerSec', 'val-max-speed', 0);
    bindSlider('slider-alpha', Tuning, 'ImpulseEfficiencyAlpha', 'val-alpha', 2);
    bindSlider('slider-damping', Tuning, 'LateralDampingPerSec', 'val-damping', 2);

    document.getElementById('btn-reset-feel').addEventListener('click', () => {
      Tuning.TetherLengthCm = 160.0;
      Tuning.MaceMassKg = 40.0;
      Tuning.SpinAccelerationDegPerSec2 = 1440.0;
      Tuning.MaxAngularSpeedDegPerSec = 1080.0;
      Tuning.ImpulseEfficiencyAlpha = 0.6;
      Tuning.LateralDampingPerSec = 1.5;

      document.getElementById('slider-tether').value = 160;
      document.getElementById('val-tether').textContent = '160';
      document.getElementById('slider-mace-mass').value = 40;
      document.getElementById('val-mace-mass').textContent = '40';
      document.getElementById('slider-accel').value = 1440;
      document.getElementById('val-accel').textContent = '1440';
      document.getElementById('slider-max-speed').value = 1080;
      document.getElementById('val-max-speed').textContent = '1080';
      document.getElementById('slider-alpha').value = 0.6;
      document.getElementById('val-alpha').textContent = '0.60';
      document.getElementById('slider-damping').value = 1.5;
      document.getElementById('val-damping').textContent = '1.50';
    });

    const gizmoBtn = document.getElementById('btn-toggle-gizmos');
    gizmoBtn.addEventListener('click', () => {
      State.bShowGizmos = !State.bShowGizmos;
      gizmoBtn.textContent = `GIZMOS: ${State.bShowGizmos ? 'ON' : 'OFF'}`;
      gizmoBtn.className = State.bShowGizmos ? 'btn-primary' : 'btn-secondary';
    });

    const resetPill = document.getElementById('key-r');
    if (resetPill) {
      resetPill.addEventListener('click', () => {
        restartRun();
      });
    }

    const togglePill = document.getElementById('key-m');
    if (togglePill) {
      togglePill.addEventListener('click', () => {
        toggleKickModel();
      });
    }
  }

  // Initialize
  init3D();
  setupTuningDrawer();
  animate();
})();
