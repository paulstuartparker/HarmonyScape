# HarmonyScape Build & Deploy Guide

## Quick Reference for AI Assistants

When a user asks to "build and deploy" or "deploy" HarmonyScape, use the existing automated script.

## Script Location

**Primary Script**: `/Users/paulparker/projects/vstproject/HarmonyScape/build/build_and_deploy.sh`

## Usage Commands

### Full Build & Deploy (Most Common)
```bash
cd /Users/paulparker/projects/vstproject/HarmonyScape/build
./build_and_deploy.sh
```

### Just Deploy (if already built)
```bash
cd /Users/paulparker/projects/vstproject/HarmonyScape/build
./deploy_plugin.sh
```

### Quick Deploy (fast copy only)
```bash
cd /Users/paulparker/projects/vstproject/HarmonyScape/build
./quick_deploy.sh
```

## What the Main Script Does

The `build_and_deploy.sh` script is a comprehensive pipeline that:

1. **Increments Version** - Auto-bumps build number and generates new color ID
2. **Cleans Build** - Removes previous build artifacts
3. **Builds VST3** - Compiles the VST3 plugin format
4. **Builds AU** - Compiles the Audio Units plugin format
5. **Deploys Plugins** - Copies to system plugin directories
6. **Resets Cache** - Clears Ableton Live plugin cache for immediate recognition

## Expected Output

Successful completion shows:
```
🎉 DEPLOYMENT COMPLETE!
Version: 1.0.XXXX
Build ID Color: R=X.X G=X.X B=X.X
```

## Build Directory Structure

```
/Users/paulparker/projects/vstproject/HarmonyScape/build/
├── build_and_deploy.sh       ← MAIN SCRIPT - Use this for "build and deploy"
├── deploy_plugin.sh          ← Deploy only (no build)
├── quick_deploy.sh           ← Fast copy only
├── increment_version.sh      ← Version management
├── HarmonyScape_artefacts/   ← Build outputs
└── CMakeFiles/               ← Build system files
```

## Plugin Deployment Locations

After successful deployment, plugins are installed to:
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/HarmonyScape.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/HarmonyScape.component`

## Version Identification

Each build has a unique identifier:
- **Version number** in top-right corner (e.g., "1.0.1025")
- **Colored square** next to version (changes each build for visual confirmation)

## Common User Requests → Actions

| User Request | Command | Notes |
|--------------|---------|-------|
| "build and deploy" | `./build_and_deploy.sh` | Full pipeline |
| "deploy" | `./build_and_deploy.sh` | Same as above |
| "rebuild" | `./build_and_deploy.sh` | Same as above |
| "just deploy" | `./deploy_plugin.sh` | Assumes already built |
| "quick deploy" | `./quick_deploy.sh` | Fast copy, no version bump |
| "update version" | `./increment_version.sh` | Version only |

## Troubleshooting

### Script Not Found
If script appears missing:
```bash
find /Users/paulparker/projects/vstproject -name "*build*deploy*" -type f
```

### Permission Issues
Make script executable:
```bash
chmod +x build_and_deploy.sh
```

### Build Failures
Check CMake is working:
```bash
cd /Users/paulparker/projects/vstproject/HarmonyScape/build
make clean
cmake ..
make -j8
```

### Plugin Not Loading
1. Check Ableton Live preferences → Plugins
2. Manually rescan plugins
3. Verify VST3 format is enabled
4. Check plugin directories exist

## Architecture Notes

The HarmonyScape plugin consists of:
- **Core Audio Processor** (`PluginProcessor.cpp`)
- **UI Editor** (`PluginEditor.cpp`)
- **ChordEngine** (harmony generation)
- **SpatialEngine** (audio rendering and positioning)
- **RibbonEngine** (rhythmic arpeggiation patterns)

## Development Workflow

1. **Make Code Changes** in `Source/` directory
2. **Run Build Script** with `./build_and_deploy.sh`
3. **Test in DAW** (Ableton Live recommended)
4. **Verify Version** using colored square and version number
5. **Iterate** as needed

## Important Files

- **Version Management**: `Source/Version.h` (auto-generated)
- **Build Configuration**: `CMakeLists.txt`
- **Main Plugin Logic**: `Source/PluginProcessor.cpp`
- **UI Implementation**: `Source/PluginEditor.cpp`
- **Documentation**: This file and `HarmonyScape_Ribbons_Implementation.md`

---

## For Future AI Assistants

**When user says "build and deploy":**
1. Navigate to build directory: `cd /Users/paulparker/projects/vstproject/HarmonyScape/build`
2. Run the script: `./build_and_deploy.sh`
3. Report the version number and build color from the output
4. Remind user to restart their DAW if needed

**The script handles everything automatically** - no need to manually copy files, reset caches, or increment versions. 