# Warp Space Time (Reality Warper SKSE)

A high-performance C++ SKSE plugin built with **CommonLibSSE-NG** that replicates the iconic **Amenotejikara** ability used by Sasuke Uchiha. It allows the player to seamlessly warp reality and instantly swap physical locations with target characters over long distances using precision vector line-of-sight math. It includes dynamic load-order alignment to project custom spell visual effects from an external mod file during execution.

## Features

*   **Amenotejikara Reality Warp:** Directly inspired by Sasuke's Rinnegan ability, allowing you to instantly displace space and trade places with any entity caught in your field of vision.
*   **Sniper-Range Location Swapping:** Transcends the game's default engine limitations on crosshair distance, allowing spatial execution from thousands of units away.
*   **Vector Engine Math Integration:** Employs physical trigonometry (`pitch`, `yaw`, and tracking matrices) alongside a cellular pointer lookup array (`ForEachReference`) to manually intercept entity frames.
*   **Payload Interpreter Processing:** Utilizes low-level bitmask manipulation (`& 0x00000FFF`) to dynamically calculate ESL-flagged plugin data records under shifting load orders.
*   **Dynamic `.ini` Inversion Engine:** Exposes targeting controls, dependency targets, and payload identification codes to a custom configuration file.

## Configuration (`warpspacetime.ini`)

The mod creates and evaluates configurations through a `warpspacetime.ini` file located alongside the compiled binary inside the `Data/SKSE/Plugins/` directory.

```ini
[Settings]
# The exact file target name containing the visual payload
ModName = Judgement Cut End - ap05's Remake.esp

# The 8-digit hexadecimal target identifier code (FormID) from xEdit
SpellFormID = 0xFE00084c

# The hexadecimal keycode code mapping for execution (Default: 0x2E is the 'C' Key)
Hotkey = 0x2E
```

### Quick Keyboard Hex Reference
*   `0x2E` = 'C' Key
*   `0x21` = 'F' Key
*   `0x14` = 'T' Key
*   `0x58` = 'X' Key

## Dependencies

*   [Skyrim Special Edition (1.5.97 / 1.6.x+)](https://steampowered.com)
*   [SKSE64](https://silverlock.org)
*   [Address Library for SKSE Plugins]([https://nexusmods.com](https://www.nexusmods.com/skyrimspecialedition/mods/32444))
*   

## Building from Source

This project uses CMake and vcpkg manifests to automate dependencies.

1. Clone the repository down to your localized build sector.
2. Initialize and set up your vcpkg development configurations:
   ```bash
   vcpkg integrate install
   ```
3. Generate the project caching layouts using your IDE or terminal:
   ```bash
   cmake -B build -S .
   ```
4. Build the compilation solution to generate `warpspacetime.dll`.

## How It Works (Systems Engineering Level)

1. **Input Interception:** The plugin registers a customized `BSTEventSink` into Skyrim's active `InputDeviceManager` to listen for button triggers before the engine evaluates standard UI states.
2. **Cell Reference Array Scanner:** Upon hotkey registration, the code looks up the player cell address space and executes a lambda-wrapped loop (`ForEachReference`) on the local entity coordinates matrix.
3. **Matrix Alignment Calculation:** The engine computes the dot product intersecting the player's normalized look vector and target positions. If an actor aligns within a 180-unit tracking vector path, they are flagged as the active target block.
4. **Memory Address Override:** The script issues safe low-level physical coordinate overrides (`SetPosition`) and updates spatial boundaries (`UpdateActor3DPosition`) to trick the physical collision bounds layer into executing an instant location shift.
