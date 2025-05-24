#!/bin/bash

# HarmonyScape Plugin Deployment Script
# This script copies the built plugins to standard locations and refreshes Ableton's plugin cache

echo "Deploying HarmonyScape plugin..."

# Define standard plugin directories
VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"

# Create directories if they don't exist
mkdir -p "$VST3_DIR"
mkdir -p "$AU_DIR"

# Copy plugins to standard locations
echo "Copying VST3 plugin..."
rm -rf "$VST3_DIR/HarmonyScape.vst3"
cp -R "HarmonyScape_artefacts/VST3/HarmonyScape.vst3" "$VST3_DIR/"

echo "Copying AU plugin..."
rm -rf "$AU_DIR/HarmonyScape.component"
cp -R "HarmonyScape_artefacts/AU/HarmonyScape.component" "$AU_DIR/"

# Set permissions
echo "Setting permissions..."
chmod -R 755 "$VST3_DIR/HarmonyScape.vst3"
chmod -R 755 "$AU_DIR/HarmonyScape.component"

# Reset Ableton's plugin cache (if Ableton is not running)
echo "Resetting plugin cache..."
ABLETON_DIR="$HOME/Library/Preferences/Ableton"

# Find the most recent Live version folder
LATEST_LIVE=$(ls -d "$ABLETON_DIR/Live"* 2>/dev/null | sort -r | head -n 1)

if [ -n "$LATEST_LIVE" ]; then
    echo "Found Ableton Live installation: $LATEST_LIVE"
    
    # Check if Ableton is running
    if pgrep -x "Ableton Live" > /dev/null; then
        echo "⚠️ Ableton Live is currently running. Please close it and run this script again to reset the cache."
    else
        # Remove the plugin database files (they will be recreated when Ableton starts)
        echo "Removing plugin database files..."
        rm -f "$LATEST_LIVE/Database/Plugin Database.xml"
        rm -f "$LATEST_LIVE/Database/Plugin Database_m.xml"
        
        echo "Plugin cache reset! Ableton will rescan plugins on next launch."
    fi
else
    echo "⚠️ Could not find Ableton Live installation folder."
fi

echo "Deployment complete! The plugin should appear in Ableton's browser under Plugins."
echo
echo "TESTING WORKFLOW:"
echo "1. Build the plugin: cd /Users/paulparker/projects/vstproject/HarmonyScape/build && cmake --build ."
echo "2. Deploy the plugin: ./deploy_plugin.sh"
echo "3. Launch Ableton Live (or restart if already running)"
echo "4. The plugin should appear in VST3 and Audio Units sections"
echo
echo "If the plugin doesn't appear, try:"
echo "- In Ableton's Preferences > Plugins, click 'Rescan Plugins'"
echo "- Check 'Use VST3 Plug-in Custom Folder' and point it to $VST3_DIR"
echo "- Enable both 'Use VST3 Plug-ins' and 'Use Audio Units' options"