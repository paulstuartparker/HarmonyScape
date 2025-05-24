#!/bin/bash

# HarmonyScape Build Version Incrementer
# This script automatically increments the build number and randomizes the color

VERSION_FILE="../Source/Version.h"

if [ ! -f "$VERSION_FILE" ]; then
    echo "Error: Version.h not found at $VERSION_FILE"
    exit 1
fi

# Read current build number
CURRENT_BUILD=$(grep "HARMONYSCAPE_BUILD_NUMBER" "$VERSION_FILE" | sed 's/.*BUILD_NUMBER \([0-9]*\).*/\1/')

# Increment build number
NEW_BUILD=$((CURRENT_BUILD + 1))

# Generate random color (avoiding too dark colors)
R=$(echo "scale=1; ($(od -An -N1 -tu1 < /dev/urandom) % 80 + 20) / 100" | bc -l)
G=$(echo "scale=1; ($(od -An -N1 -tu1 < /dev/urandom) % 80 + 20) / 100" | bc -l)
B=$(echo "scale=1; ($(od -An -N1 -tu1 < /dev/urandom) % 80 + 20) / 100" | bc -l)

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