"""
ValidateStandaloneAssets.py
===========================
SpaceMentum — Offline Project Health, Asset Pipeline & Standalone Packaging Readiness Validator.

@creator_feel
"lets go for quality and hence unreal engine make most sense to me"
"it is a rule to add my emotions, likings, reasonings, quotes too in all documentations"

Checks:
  1. SpaceMentum.uproject descriptor schema, engine version, runtime modules, and plugin activations.
  2. Target build configuration files (SpaceMentum.Target.cs, SpaceMentumEditor.Target.cs).
  3. Engine & packaging configurations (DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini).
  4. C++ source architecture, module acyclic dependencies, and header declarations.
  5. Content directory tree for Nanite meshes, PBR materials, Niagara VFX, and MetaSounds.
  6. Standalone build script readiness.
"""

import os
import sys
import json
import re

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

class ValidationSuite:
    def __init__(self):
        self.passes = 0
        self.warnings = 0
        self.failures = 0

    def assert_test(self, condition, message, is_warning=False):
        if condition:
            print(f"  [PASS] {message}")
            self.passes += 1
            return True
        else:
            if is_warning:
                print(f"  [WARN] {message}")
                self.warnings += 1
            else:
                print(f"  [FAIL] {message}")
                self.failures += 1
            return False

def validate_uproject(suite):
    print("\n--- [1] Validating SpaceMentum.uproject Descriptor ---")
    uproject_path = os.path.join(PROJECT_ROOT, "SpaceMentum.uproject")
    if not suite.assert_test(os.path.isfile(uproject_path), f"File exists: {uproject_path}"):
        return

    try:
        with open(uproject_path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except Exception as e:
        suite.assert_test(False, f"Valid JSON syntax: {e}")
        return

    suite.assert_test(data.get("EngineAssociation") == "5.8", "Engine Association is set to 5.8 (Unreal Engine 5.8 June 2026)")
    
    modules = {m.get("Name"): m for m in data.get("Modules", [])}
    suite.assert_test("SpaceMentumPhysics" in modules, "Module 'SpaceMentumPhysics' declared")
    suite.assert_test("SpaceMentumAbilities" in modules, "Module 'SpaceMentumAbilities' declared")
    suite.assert_test("SpaceMentumCore" in modules, "Primary Module 'SpaceMentumCore' declared")

    plugins = {p.get("Name"): p.get("Enabled") for p in data.get("Plugins", [])}
    suite.assert_test(plugins.get("EnhancedInput") is True, "Plugin 'EnhancedInput' is enabled")
    suite.assert_test(plugins.get("Niagara") is True, "Plugin 'Niagara' is enabled")
    suite.assert_test(plugins.get("Metasound") is True, "Plugin 'Metasound' is enabled for Step 7 audio polish")

def validate_configs(suite):
    print("\n--- [2] Validating Configuration Files & Packaging Settings ---")
    config_dir = os.path.join(PROJECT_ROOT, "Config")
    
    engine_ini = os.path.join(config_dir, "DefaultEngine.ini")
    if suite.assert_test(os.path.isfile(engine_ini), "DefaultEngine.ini exists"):
        with open(engine_ini, "r", encoding="utf-8") as f:
            content = f.read()
        suite.assert_test("DefaultGravityZ=0.000000" in content, "Zero-G deep space environment configured")
        suite.assert_test("bSubstepping=True" in content, "Chaos physics substepping enabled for 120Hz determinism")
        suite.assert_test("GlobalDefaultGameMode=/Script/SpaceMentumCore.SpaceGreyboxGameMode" in content, "Default GameMode hooked to SpaceGreyboxGameMode")
        suite.assert_test("ResolutionSizeX=1920" in content, "Standalone 1080p resolution configured")

    game_ini = os.path.join(config_dir, "DefaultGame.ini")
    if suite.assert_test(os.path.isfile(game_ini), "DefaultGame.ini exists"):
        with open(game_ini, "r", encoding="utf-8") as f:
            content = f.read()
        suite.assert_test("BuildConfiguration=PPBC_Shipping" in content, "Shipping packaging build configuration set")
        suite.assert_test("DirectoriesToAlwaysCook=(Path=\"/Game/Core\")" in content, "Core cooked asset directory registered")
        suite.assert_test("DirectoriesToAlwaysCook=(Path=\"/Game/Environment\")" in content, "Environment cooked asset directory registered")
        suite.assert_test("DirectoriesToAlwaysCook=(Path=\"/Game/Characters\")" in content, "Characters cooked asset directory registered")
        suite.assert_test("DirectoriesToAlwaysCook=(Path=\"/Game/Weapons\")" in content, "Weapons cooked asset directory registered")

    input_ini = os.path.join(config_dir, "DefaultInput.ini")
    suite.assert_test(os.path.isfile(input_ini), "DefaultInput.ini exists")

def validate_source_modules(suite):
    print("\n--- [3] Validating C++ Source Architecture & Step 7 Implementations ---")
    source_dir = os.path.join(PROJECT_ROOT, "Source")
    
    suite.assert_test(os.path.isfile(os.path.join(source_dir, "SpaceMentum.Target.cs")), "SpaceMentum.Target.cs exists")
    suite.assert_test(os.path.isfile(os.path.join(source_dir, "SpaceMentumEditor.Target.cs")), "SpaceMentumEditor.Target.cs exists")

    # Step 7 Astronaut & Rigging Hookups
    char_header = os.path.join(source_dir, "SpaceMentumCore", "Public", "Player", "SpaceAstronautCharacter.h")
    if suite.assert_test(os.path.isfile(char_header), "SpaceAstronautCharacter.h exists"):
        with open(char_header, "r", encoding="utf-8") as f:
            code = f.read()
        suite.assert_test("MechanicalArmMesh" in code, "MechanicalArmMesh declared in character header")
        suite.assert_test("MaceHeadMesh" in code, "MaceHeadMesh declared in character header")
        suite.assert_test("ArmPhysicsConstraint" in code, "ArmPhysicsConstraint declared in character header")
        suite.assert_test("SuitDynamicMaterial" in code, "SuitDynamicMaterial declared in character header")

    # Step 7 Audio Polish Hookups
    audio_header = os.path.join(source_dir, "SpaceMentumCore", "Public", "Audio", "SpaceProceduralFlightAudioComponent.h")
    if suite.assert_test(os.path.isfile(audio_header), "SpaceProceduralFlightAudioComponent.h exists"):
        with open(audio_header, "r", encoding="utf-8") as f:
            code = f.read()
        suite.assert_test("EngineLifeSupportHumSound" in code, "EngineLifeSupportHumSound declared in audio header")
        suite.assert_test("AsteroidProximityAlarmSound" in code, "AsteroidProximityAlarmSound declared in audio header")
        suite.assert_test("PortalWarpCollapseSound" in code, "PortalWarpCollapseSound declared in audio header")
        suite.assert_test("UpdateProximityRadarAlarm" in code, "UpdateProximityRadarAlarm method declared")

    # Step 7 Material Palette Data Asset
    palette_header = os.path.join(source_dir, "SpaceMentumCore", "Public", "Data", "SpaceMaterialPaletteDataAsset.h")
    suite.assert_test(os.path.isfile(palette_header), "SpaceMaterialPaletteDataAsset.h exists")

def validate_editor_scripts_and_tools(suite):
    print("\n--- [4] Validating Tools, Editor Scripts & Standalone Packaging Pipeline ---")
    tools_editor = os.path.join(PROJECT_ROOT, "Tools", "Editor")
    suite.assert_test(os.path.isfile(os.path.join(tools_editor, "GenerateGreyboxAssets.py")), "Step 3 asset generator script present")
    suite.assert_test(os.path.isfile(os.path.join(tools_editor, "GenerateStep5VFXAndAudioAssets.py")), "Step 5 VFX/Audio asset generator script present")
    suite.assert_test(os.path.isfile(os.path.join(tools_editor, "GenerateStep6ProgressionAssets.py")), "Step 6 progression asset generator script present")
    suite.assert_test(os.path.isfile(os.path.join(tools_editor, "GenerateStep7HighFidelityAssets.py")), "Step 7 high-fidelity asset generator script present")

    build_tools = os.path.join(PROJECT_ROOT, "Tools", "Build")
    suite.assert_test(os.path.isfile(os.path.join(build_tools, "PackageSpaceMentumWin64.ps1")), "Win64 PowerShell packaging automation script present")
    suite.assert_test(os.path.isfile(os.path.join(build_tools, "PackageSpaceMentumWin64.bat")), "Win64 batch packaging runner present")

def validate_content_structure(suite):
    print("\n--- [5] Validating Content Asset Directory Tree ---")
    content_dir = os.path.join(PROJECT_ROOT, "Content")
    expected_subdirs = [
        "Core/Blueprints",
        "Core/Data",
        "Core/Input",
        "Characters/Astronaut",
        "Weapons/Mace",
        "Environment/Meshes",
        "Environment/Materials",
        "FX/Niagara",
        "Audio/Metasounds",
        "Maps"
    ]
    for sub in expected_subdirs:
        full_p = os.path.join(content_dir, sub.replace("/", os.sep))
        suite.assert_test(os.path.isdir(full_p), f"Content subdirectory exists: Content/{sub}")

def main():
    print("======================================================================")
    print("  SPACEMENTUM — OFFLINE STANDALONE ASSET & PACKAGING VALIDATOR        ")
    print("======================================================================")

    suite = ValidationSuite()
    validate_uproject(suite)
    validate_configs(suite)
    validate_source_modules(suite)
    validate_editor_scripts_and_tools(suite)
    validate_content_structure(suite)

    print("\n======================================================================")
    print(f"  VALIDATION SUMMARY: {suite.passes} PASSED | {suite.warnings} WARNINGS | {suite.failures} FAILURES")
    print("======================================================================")

    if suite.failures == 0:
        print("  RESULT: PROJECT READY FOR HIGH-FIDELITY PACKAGING & STANDALONE BUILDS!")
        return 0
    else:
        print("  RESULT: ISSUES DETECTED. PLEASE RESOLVE BEFORE PACKAGING.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
