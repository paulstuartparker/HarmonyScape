#!/bin/bash

# HarmonyScape Build Version Incrementer
# This script automatically increments the build number and cycles through primary colors

VERSION_FILE="../Source/Version.h"

if [ ! -f "$VERSION_FILE" ]; then
    echo "Error: Version.h not found at $VERSION_FILE"
    exit 1
fi

# Read current build number
CURRENT_BUILD=$(grep "HARMONYSCAPE_BUILD_NUMBER" "$VERSION_FILE" | sed 's/.*BUILD_NUMBER \([0-9]*\).*/\1/')

# Increment build number
NEW_BUILD=$((CURRENT_BUILD + 1))

# Cycle through primary/vibrant colors based on build number
COLOR_INDEX=$((NEW_BUILD % 8))

case $COLOR_INDEX in
    0) R="1.0"; G="0.0"; B="0.0" ;;  # Pure Red
    1) R="0.0"; G="1.0"; B="0.0" ;;  # Pure Green
    2) R="0.0"; G="0.0"; B="1.0" ;;  # Pure Blue
    3) R="1.0"; G="1.0"; B="0.0" ;;  # Yellow
    4) R="1.0"; G="0.0"; B="1.0" ;;  # Magenta
    5) R="0.0"; G="1.0"; B="1.0" ;;  # Cyan
    6) R="1.0"; G="0.5"; B="0.0" ;;  # Orange
    7) R="0.5"; G="0.0"; B="1.0" ;;  # Purple
esac

# Create new version file
cat > "$VERSION_FILE" << EOF
#pragma once

// Auto-generated build info - DO NOT EDIT MANUALLY
#define HARMONYSCAPE_VERSION_MAJOR 1
#define HARMONYSCAPE_VERSION_MINOR 0
#define HARMONYSCAPE_BUILD_NUMBER $NEW_BUILD

// Build color for visual identification - changes each build
#define BUILD_COLOR_R ${R}f
#define BUILD_COLOR_G ${G}f
#define BUILD_COLOR_B ${B}f

// Version string
#define HARMONYSCAPE_VERSION_STRING "1.0.$NEW_BUILD"
EOF

echo "✅ Build incremented: $CURRENT_BUILD → $NEW_BUILD"
echo "🎨 New build color: R=$R G=$G B=$B"
echo "📄 Updated $VERSION_FILE" 