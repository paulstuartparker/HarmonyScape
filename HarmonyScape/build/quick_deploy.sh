#!/bin/bash

# Quick deployment script for HarmonyScape plugin
# This script builds and deploys the plugin in one step

echo "🚀 Quick deploying HarmonyScape plugin..."

# Build the plugin
echo "📦 Building plugin..."
cmake --build .

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "❌ Build failed! Please check the errors above."
    exit 1
fi

# Run the deployment script
echo "📤 Deploying plugin..."
./deploy_plugin.sh

# Check if Ableton is running
if pgrep -x "Ableton Live" > /dev/null; then
    echo "⚠️  Ableton Live is currently running."
    echo "To see your changes:"
    echo "1. Save your work in Ableton"
    echo "2. Close Ableton Live"
    echo "3. Run this script again"
    echo "4. Reopen Ableton Live"
else
    echo "✅ Plugin deployed successfully!"
    echo "You can now open Ableton Live to see your changes."
fi 