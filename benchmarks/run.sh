#!/bin/sh
# Stable entry point for the libfastjson benchmark runner.
set -eu

SCRIPT_DIR=$(dirname -- "$0")
SCRIPT_DIR=$(CDPATH='' cd -- "$SCRIPT_DIR" && pwd)
exec python3 "$SCRIPT_DIR/runner.py" "$@"
