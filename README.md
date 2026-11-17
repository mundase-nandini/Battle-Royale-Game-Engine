# RBG Engine Pro — Royal Battle Game Tactical Runtime

A C++17 battle simulation runtime with a live tactical command interface. This is an RBG-specific engine prototype, not a general-purpose replacement for Unreal Engine and not a networked game client.

## What is included
- 20-player match capacity and four selectable specialist archetypes.
- **Vanguard / Assault:** 110 HP, balanced shield/armour, Royal Assault Rifle, Overdrive damage burst.
- **Phantom / Recon:** 85 HP, high mobility, Longshot Sniper, Phase Dash.
- **Titan / Sentinel:** 145 HP, heavy armour, Titan LMG, Fortify armour boost.
- **Arcweaver / Medic:** 100 HP, expanded shield, Violet Repeater, Nova Pulse shield restoration.
- Add-player workflow with class selection; class selection assigns the weapon and stat profile automatically.
- Autonomous target-seeking, range-aware movement, strafing, kiting, class-specific mobility and visible movement trails.
- Autonomous fire cadence, range checks, shield-first damage, armour mitigation, damage statistics and eliminations.
- Shrinking safe zone, storm damage, event/combat feed, roster, inspector and match controls.
- Responsive tactical command layout with a navy / blue / electric cyan / violet / magenta gradient theme from the supplied palette: `#042142`, `#153C6A`, `#2475AC`, `#3DE0FC`, `#733E85`, `#E977F5`.

## Windows build
Requirements: Windows 10/11, CMake 3.20+, Ninja, MinGW-w64 GCC 13+ and internet access for the first configure (CMake downloads GLFW and Dear ImGui).

1. Add your MinGW `bin` folder and Ninja folder to PATH.
2. Open PowerShell in this folder.
3. Run:

```powershell
.\build_windows.ps1
.\build\RBGEngine.exe
```

Or configure manually:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER="C:/path/to/g++.exe"
cmake --build build --parallel
.\build\RBGEngine.exe
```

## Runtime controls
- **START / RESUME**, **PAUSE**, **STEP**, **RESET** control the simulation.
- **AUTO COMBAT** toggles autonomous firing; movement continues while the match runs.
- **TRAILS** toggles movement trails; **SPEED** adjusts simulation speed.
- Choose a class, enter a name and select **ADD PLAYER**. The class configures the player's starting weapon and attributes.
- Select a roster entry to inspect stats and adjust the weapon, position and armour.

## Architecture
`Engine` owns `Match`; `Match` coordinates `Player`, `Weapon`, `SafeZone` and `EventLog`. The simulation core is independent of the ImGui/GLFW presentation layer and can be embedded in another application.

## Scope
This is a reusable RBG simulation/backend with an Unreal-inspired tactical command UI. It is not a full 3D game editor, rendering engine, multiplayer server or production anti-cheat/network stack.
