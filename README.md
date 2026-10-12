# SpaceMentum

> *"the concept is a space person zooming in the forward direction in a uniform velocity and the person has a mace in his hand. by spinning and stopping the spin in a pertiicular direction, one can stir their direction to move away from obsticles."*: the Creator

A physics-driven space odyssey built in **Unreal Engine 5.8**. You can't brake and you can't turn. The only way to steer is to spin a heavy mace and let it go at the right moment.

## Controls (prototype)
| Key | Action |
| :--- | :--- |
| **A** (hold) | Spin mace counter-clockwise |
| **D** (hold) | Spin mace clockwise |
| *(let go)* | Mace stops and its momentum kicks you sideways |

## Getting Started
1. Install **Unreal Engine 5.8** (Epic Games Launcher) and **Visual Studio 2022** with the *Game development with C++* workload.
2. Run `git lfs install` once. Binary assets are stored through Git LFS.
3. Right-click `SpaceMentum.uproject` → **Generate Visual Studio project files**.
4. Open `SpaceMentum.sln` and build the **Development Editor | Win64** configuration.
5. Launch the editor from Visual Studio, or double-click `SpaceMentum.uproject`.

## Documentation
| Doc | What's inside |
| :--- | :--- |
| [GAME_DESIGN_DOCUMENT.md](GAME_DESIGN_DOCUMENT.md) | Vision, the Creator's emotional anchors, requirements |
| [Docs/TECHNICAL_SPECIFICATIONS.md](Docs/TECHNICAL_SPECIFICATIONS.md) | Modules, dependency rules, math conventions, coding standard |
| [PROGRESS.md](PROGRESS.md) | **Live hand-off log.** Start here to see what's done and what's next |

## Source Layout
```
Source/
├── SpaceMentumPhysics/    # orbit math, momentum kick, forward drift (lowest layer)
├── SpaceMentumAbilities/  # data assets: maces, abilities, obstacles
└── SpaceMentumCore/       # primary module: game mode, controller, astronaut, camera
```
