#!/bin/sh
set -e

echo "Starting tevo..."

PWD=$(dirname "$(readlink -f "$0")")

export LD_LIBRARY_PATH="$PWD"

export QT_PLUGIN_PATH="$PWD"

#export QT_DEBUG_PLUGINS=1

"$PWD/tevo"
