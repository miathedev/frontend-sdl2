# projectM VST3 Plugin

This directory contains a VST3/AU plugin version of projectM that can be used directly within Digital Audio Workstations (DAWs) such as Ableton Live, FL Studio, Reaper, Logic Pro, and others.

## Features

- Real-time visualization of audio from your DAW
- OpenGL-accelerated rendering
- Preset navigation and management
- Audio pass-through (plugin does not modify audio)
- Supports VST3 and AU (Audio Units) formats
- Cross-platform: Windows, macOS, and Linux

## Building the VST Plugin

### Prerequisites

1. **libprojectM** - Must be installed. Follow the [main projectM build instructions](https://github.com/projectM-visualizer/projectm/wiki/Building-libprojectM).

2. **CMake 3.22+** - Required for JUCE integration.

3. **C++17 compiler** - GCC 7+, Clang 5+, or MSVC 2019+.

4. **OpenGL development libraries** - Usually included with your graphics drivers.

### Build Steps

From the main repository root:

```bash
# Create build directory
mkdir cmake-build-vst
cd cmake-build-vst

# Configure with VST plugin enabled
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DENABLE_VST_PLUGIN=ON

# Build
cmake --build . --config Release
```

JUCE framework will be automatically downloaded during the CMake configuration step.

### Platform-Specific Notes

#### Windows

```bash
cmake -G "Visual Studio 17 2022" -A x64 -S .. -B . -DENABLE_VST_PLUGIN=ON
cmake --build . --config Release
```

The VST3 plugin will be in: `cmake-build-vst/src/vst-plugin/projectM-VST_artefacts/Release/VST3/`

#### macOS

```bash
cmake -G Xcode -S .. -B . -DENABLE_VST_PLUGIN=ON
cmake --build . --config Release
```

Both VST3 and AU plugins will be built. Find them in:
- VST3: `cmake-build-vst/src/vst-plugin/projectM-VST_artefacts/Release/VST3/`
- AU: `cmake-build-vst/src/vst-plugin/projectM-VST_artefacts/Release/AU/`

#### Linux

```bash
cmake -G "Unix Makefiles" -S .. -B . -DCMAKE_BUILD_TYPE=Release -DENABLE_VST_PLUGIN=ON
cmake --build .
```

## Installation

### VST3 Installation Paths

Copy the built `.vst3` bundle to your DAW's VST3 plugin directory:

- **Windows**: `C:\Program Files\Common Files\VST3\`
- **macOS**: `/Library/Audio/Plug-Ins/VST3/` or `~/Library/Audio/Plug-Ins/VST3/`
- **Linux**: `~/.vst3/` or `/usr/lib/vst3/`

### AU Installation (macOS only)

Copy the built `.component` bundle to:
- `/Library/Audio/Plug-Ins/Components/` or `~/Library/Audio/Plug-Ins/Components/`

After installation, rescan plugins in your DAW.

## Configuration

### Preset Paths

The plugin looks for presets in the following default locations:

- **Windows**: `%APPDATA%\projectM\presets\`
- **macOS**: `~/Library/Application Support/projectM/presets/`
- **Linux**: `~/.config/projectM/presets/`

You can also configure custom preset paths via the Settings button in the plugin UI.

### Texture Paths

Textures are searched in:

- **Windows**: `%APPDATA%\projectM\textures\`
- **macOS**: `~/Library/Application Support/projectM/textures/`
- **Linux**: `~/.config/projectM/textures/`

You can download the standard texture pack from [projectM texture pack repository](https://github.com/projectM-visualizer/presets-milkdrop-texture-pack).

### Presets

Download presets from:
- [Cream of the Crop preset collection](https://github.com/projectM-visualizer/presets-cream-of-the-crop)
- [En D Presets](https://github.com/projectM-visualizer/presets-en-d)

## Usage

1. Insert "projectM Visualizer" as an effect/analyzer on any audio track
2. The visualization window will appear showing real-time audio visualization
3. Use the control panel at the bottom:
   - **<<** / **>>**: Navigate to previous/next preset
   - **Random**: Jump to a random preset
   - **Lock**: Lock the current preset (prevents automatic switching)
   - **Settings**: Configure preset and texture paths

### Resizing

The plugin window can be resized by dragging its edges. The visualization will scale accordingly.

## Tested DAWs

The plugin has been designed to work with:
- Reaper
- Ableton Live
- FL Studio
- Logic Pro (macOS, AU format)
- Cubase
- Bitwig Studio

## Known Limitations

- Some complex presets may impact performance
- Very high frame rates may increase CPU usage
- Audio latency is not visualized (visualization shows current audio, not played audio)

## Troubleshooting

### Plugin not loading

1. Ensure libprojectM is installed and accessible
2. Check that OpenGL 3.0+ is available
3. Verify the plugin is in the correct directory
4. Rescan plugins in your DAW

### No visualization

1. Make sure audio is playing through the track
2. Check that presets are in the correct directory
3. Try clicking "Random" to load a preset

### Performance issues

1. Reduce the plugin window size
2. Use simpler presets
3. Lower your DAW's display refresh rate

## License

This VST plugin is part of the projectM-SDL frontend and is licensed under the GNU General Public License v3.0. See the main LICENSE.md file for details.

The plugin uses:
- [JUCE Framework](https://juce.com/) - GPLv3 compatible
- [libprojectM](https://github.com/projectM-visualizer/projectm) - LGPL v2.1+

## Contributing

Contributions are welcome! Please submit pull requests to the main projectM-SDL repository.
