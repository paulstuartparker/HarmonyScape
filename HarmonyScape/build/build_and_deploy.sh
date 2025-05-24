#!/bin/bash

# HarmonyScape - Streamlined Build and Deploy Script
# One command to rule them all: increment version, build, and deploy

set -e  # Exit on any error

echo "🚀 HarmonyScape Build & Deploy Pipeline"
echo "======================================"

# Step 1: Increment version and generate new build color
echo "📈 Step 1: Incrementing build version..."
./increment_version.sh

# Step 2: Clean previous build
echo "🧹 Step 2: Cleaning previous build..."
make clean > /dev/null 2>&1 || true

# Step 3: Build VST3 plugin
echo "🔨 Step 3: Building VST3 plugin..."
cmake --build . --target HarmonyScape_VST3

# Check if build was successful
if [ ! -f "HarmonyScape_artefacts/Release/VST3/HarmonyScape.vst3/Contents/MacOS/HarmonyScape" ]; then
    echo "❌ Build failed - VST3 plugin not found"
    exit 1
fi

# Build AU plugin
echo "🔨 Step 3b: Building AU plugin..."
cmake --build . --target HarmonyScape_AU

# Check if AU build was successful
if [ ! -f "HarmonyScape_artefacts/Release/AU/HarmonyScape.component/Contents/MacOS/HarmonyScape" ]; then
    echo "❌ Build failed - AU plugin not found"
    exit 1
fi

echo "✅ Build successful!"

# Step 4: Deploy plugins
echo "📦 Step 4: Deploying plugins..."

# Define directories
VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"

# Create directories if needed
mkdir -p "$VST3_DIR" "$AU_DIR"

# Remove old plugins completely
echo "   🗑️  Removing old plugins..."
rm -rf "$VST3_DIR/HarmonyScape.vst3"*
rm -rf "$AU_DIR/HarmonyScape.component"*

# Copy new plugins
echo "   📋 Copying VST3..."
cp -R "HarmonyScape_artefacts/Release/VST3/HarmonyScape.vst3" "$VST3_DIR/"

if [ -d "HarmonyScape_artefacts/Release/AU/HarmonyScape.component" ]; then
    echo "   📋 Copying AU..."
    cp -R "HarmonyScape_artefacts/Release/AU/HarmonyScape.component" "$AU_DIR/"
fi

# Set permissions
chmod -R 755 "$VST3_DIR/HarmonyScape.vst3"
[ -d "$AU_DIR/HarmonyScape.component" ] && chmod -R 755 "$AU_DIR/HarmonyScape.component"

# Step 5: Reset plugin cache
echo "🔄 Step 5: Resetting plugin cache..."
ABLETON_DIR="$HOME/Library/Preferences/Ableton"
LATEST_LIVE=$(ls -d "$ABLETON_DIR/Live"* 2>/dev/null | sort -r | head -n 1)

if [ -n "$LATEST_LIVE" ]; then
    echo "   📱 Found Ableton Live: $(basename "$LATEST_LIVE")"
    
    # Force cache reset regardless of whether Ableton is running
    rm -f "$LATEST_LIVE/Database/Plugin Database.xml"
    rm -f "$LATEST_LIVE/Database/Plugin Database_m.xml"
    echo "   ✅ Plugin cache reset"
else
    echo "   ⚠️  Ableton Live not found - manual rescan may be needed"
fi

# Step 6: Get version info for confirmation
VERSION_INFO=$(grep "HARMONYSCAPE_VERSION_STRING" ../Source/Version.h | sed 's/.*"\(.*\)".*/\1/')
BUILD_COLOR_R=$(grep "BUILD_COLOR_R" ../Source/Version.h | sed 's/.*R \([0-9.]*\)f.*/\1/')
BUILD_COLOR_G=$(grep "BUILD_COLOR_G" ../Source/Version.h | sed 's/.*G \([0-9.]*\)f.*/\1/')
BUILD_COLOR_B=$(grep "BUILD_COLOR_B" ../Source/Version.h | sed 's/.*B \([0-9.]*\)f.*/\1/')

echo
echo "🎉 DEPLOYMENT COMPLETE!"
echo "======================"
echo "Version: $VERSION_INFO"
echo "Build ID Color: R=$BUILD_COLOR_R G=$BUILD_COLOR_G B=$BUILD_COLOR_B"
echo
echo "🎯 NEXT STEPS:"
echo "1. Launch/restart Ableton Live"
echo "2. Load HarmonyScape plugin"
echo "3. Look for the colored square (top-right) and version number"
echo "4. If you see the new version/color, deployment worked!"
echo
echo "If plugin doesn't appear:"
echo "- In Ableton: Preferences > Plugins > Rescan Plugins"
echo "- Check VST3 is enabled in preferences" 