#!/bin/bash

set -e

PLUGIN_NAME="KV-reTuner"
PATTERN="${PLUGIN_NAME}.*"

# Plugin locations
PLUGIN_DIRS=(
    "$HOME/Library/Audio/Plug-Ins"
    "$HOME/Applications"
    "/Library/Audio/Plug-Ins"
    "/Applications"
)

for dir in "${PLUGIN_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        find "$dir" -maxdepth 3 -type d -name "$PATTERN" -print -exec rm -rf {} \; 2>/dev/null || true
    fi
done

rm -rf "/Applications/reTuner.app"
rm -rf "$HOME/Applications/reTuner.app"
