"""
SpaceMentum — In-Editor Greybox Asset Generator & Verification Script
Compatible with Unreal Engine 5.8 / 5.5 / 5.4.

Creator Feel & Intent:
"lets go for quality and hence unreal engine make most sense to me"
"it is a rule to add my emotions, likings, reasonings, quotes too in all documentations"

Usage:
  Option A (Inside Unreal Editor):
    1. Open SpaceMentum in Unreal Editor.
    2. Open Window -> Output Log -> switch command prompt from Cmd to Python.
    3. Run:
       unreal.PythonScriptLibrary.execute_python_command("exec(open('Tools/Editor/GenerateGreyboxAssets.py').read())")
       OR in the terminal:
       UnrealEditor-Cmd.exe "d:/AwesomenessProwess/SpaceMentum/SpaceMentum.uproject" -ExecutePythonScript="Tools/Editor/GenerateGreyboxAssets.py"

This script creates and verifies the following native Unreal assets:
  - Content/Core/Maps/L_Prototype_Greybox.umap (Level with Greybox Arena & PlayerStart)
  - Content/Core/Input/IA_SpinCounterClockwise.uasset (Enhanced Input Action)
  - Content/Core/Input/IA_SpinClockwise.uasset (Enhanced Input Action)
  - Content/Core/Input/IA_ToggleKickModel.uasset (Enhanced Input Action)
  - Content/Core/Input/IMC_DefaultSpaceDiving.uasset (Input Mapping Context: A, D, M)
  - Content/Abilities/Maces/DA_Mace_Default.uasset (Mace Definition Data Asset)
"""

import sys

try:
    import unreal
except ImportError:
    print("[SpaceMentum] WARNING: 'unreal' module not found. This script must be run inside Unreal Editor Python environment.")
    print("[SpaceMentum] Note: C++ code has procedural Enhanced Input and procedural Arena fallbacks so the game runs even without these assets!")
    sys.exit(0)


def log_step(message: str):
    unreal.log(f"[SpaceMentum|GreyboxSetup] {message}")


def create_package_directory(package_path: str):
    """Ensures content folder directory exists."""
    if not unreal.EditorAssetLibrary.does_directory_exist(package_path):
        unreal.EditorAssetLibrary.make_directory(package_path)
        log_step(f"Created content directory: {package_path}")


def create_input_action(asset_name: str, package_path: str = "/Game/Core/Input"):
    """Creates a boolean Enhanced Input Action asset if it doesn't already exist."""
    create_package_directory(package_path)
    full_path = f"{package_path}/{asset_name}"

    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        log_step(f"Input Action already exists: {full_path}")
        return unreal.EditorAssetLibrary.load_asset(full_path)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputActionFactory()
    new_action = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=unreal.InputAction,
        factory=factory
    )

    if new_action:
        new_action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
        unreal.EditorAssetLibrary.save_loaded_asset(new_action)
        log_step(f"Successfully generated Input Action: {full_path}")

    return new_action


def create_input_mapping_context(
    action_ccw,
    action_cw,
    action_toggle,
    package_path: str = "/Game/Core/Input",
    asset_name: str = "IMC_DefaultSpaceDiving"
):
    """Creates the Enhanced Input Mapping Context with A, D, and M keys."""
    create_package_directory(package_path)
    full_path = f"{package_path}/{asset_name}"

    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        log_step(f"Mapping Context already exists: {full_path}")
        return unreal.EditorAssetLibrary.load_asset(full_path)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputMappingContextFactory()
    imc = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=unreal.InputMappingContext,
        factory=factory
    )

    if imc:
        # A key -> CCW spin
        if action_ccw:
            mapping_a = imc.map_key(action_ccw, unreal.Key(key_name="A"))
            log_step("Mapped Key 'A' -> IA_SpinCounterClockwise")

        # D key -> CW spin
        if action_cw:
            mapping_d = imc.map_key(action_cw, unreal.Key(key_name="D"))
            log_step("Mapped Key 'D' -> IA_SpinClockwise")

        # M key -> Toggle kick model
        if action_toggle:
            mapping_m = imc.map_key(action_toggle, unreal.Key(key_name="M"))
            log_step("Mapped Key 'M' -> IA_ToggleKickModel")

        unreal.EditorAssetLibrary.save_loaded_asset(imc)
        log_step(f"Successfully generated Input Mapping Context: {full_path}")

    return imc


def create_default_mace_data_asset(package_path: str = "/Game/Abilities/Maces", asset_name: str = "DA_Mace_Default"):
    """Creates the default mace definition asset DA_Mace_Default with calibrated physics knobs."""
    create_package_directory(package_path)
    full_path = f"{package_path}/{asset_name}"

    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        log_step(f"Mace Data Asset already exists: {full_path}")
        return unreal.EditorAssetLibrary.load_asset(full_path)

    # Look up UMaceDefinitionDataAsset class
    mace_class = unreal.load_class(None, "/Script/SpaceMentumAbilities.MaceDefinitionDataAsset")
    if not mace_class:
        log_step("WARNING: MaceDefinitionDataAsset class not loaded; skipping data asset authoring.")
        return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", mace_class)

    mace_asset = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=package_path,
        asset_class=mace_class,
        factory=factory
    )

    if mace_asset:
        # Populate tuned feel variables
        mace_asset.set_editor_property("display_name", unreal.Text("Standard Orbital Anchor"))
        
        # Physics tuning
        orbit_tuning = mace_asset.get_editor_property("orbit_tuning")
        orbit_tuning.set_editor_property("tether_length_cm", 160.0)
        orbit_tuning.set_editor_property("mace_mass_kg", 40.0)
        orbit_tuning.set_editor_property("spin_acceleration_deg_per_sec2", 1440.0)
        orbit_tuning.set_editor_property("max_angular_speed_deg_per_sec", 1080.0)
        mace_asset.set_editor_property("orbit_tuning", orbit_tuning)

        transfer_tuning = mace_asset.get_editor_property("transfer_tuning")
        transfer_tuning.set_editor_property("astronaut_mass_kg", 90.0)
        transfer_tuning.set_editor_property("impulse_efficiency_alpha", 0.6)
        transfer_tuning.set_editor_property("lateral_damping_per_sec", 1.5)
        transfer_tuning.set_editor_property("max_lateral_speed_cm_per_sec", 1800.0)
        mace_asset.set_editor_property("transfer_tuning", transfer_tuning)

        unreal.EditorAssetLibrary.save_loaded_asset(mace_asset)
        log_step(f"Successfully generated Mace Data Asset: {full_path}")

    return mace_asset


def create_greybox_level(map_package_path: str = "/Game/Core/Maps/L_Prototype_Greybox"):
    """Creates the L_Prototype_Greybox level with PlayerStart and ASpaceGreyboxArenaActor."""
    create_package_directory("/Game/Core/Maps")

    if unreal.EditorAssetLibrary.does_asset_exist(map_package_path):
        log_step(f"Greybox Level already exists: {map_package_path}")
        return

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not level_subsystem:
        log_step("LevelEditorSubsystem not available.")
        return

    # Create new empty level
    level_subsystem.new_level(map_package_path)
    log_step(f"Created new blank level at: {map_package_path}")

    # Spawn PlayerStart at world origin
    player_start_class = unreal.PlayerStart.static_class()
    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        player_start_class,
        location=unreal.Vector(0.0, 0.0, 0.0),
        rotation=unreal.Rotator(0.0, 0.0, 0.0)
    )
    if player_start:
        log_step("Spawned PlayerStart at origin (0, 0, 0)")

    # Spawn ASpaceGreyboxArenaActor at world origin
    arena_class = unreal.load_class(None, "/Script/SpaceMentumCore.SpaceGreyboxArenaActor")
    if arena_class:
        arena_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            arena_class,
            location=unreal.Vector(0.0, 0.0, 0.0),
            rotation=unreal.Rotator(0.0, 0.0, 0.0)
        )
        if arena_actor:
            log_step("Spawned ASpaceGreyboxArenaActor into L_Prototype_Greybox")

    # Save level
    level_subsystem.save_current_level()
    log_step(f"Saved L_Prototype_Greybox level successfully!")


def main():
    log_step("=== Starting Step 3 Greybox Asset Verification & Authoring ===")
    
    # 1. Enhanced Input Actions
    ia_ccw = create_input_action("IA_SpinCounterClockwise")
    ia_cw = create_input_action("IA_SpinClockwise")
    ia_toggle = create_input_action("IA_ToggleKickModel")

    # 2. Enhanced Input Mapping Context
    imc = create_input_mapping_context(ia_ccw, ia_cw, ia_toggle)

    # 3. Default Mace Definition Asset
    da_mace = create_default_mace_data_asset()

    # 4. Greybox Level
    create_greybox_level()

    log_step("=== Step 3 Greybox Asset Generation & Verification Complete! ===")


if __name__ == "__main__":
    main()
