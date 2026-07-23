#!/bin/bash
# Build-only CI entry point for development containers.
set -e

printf 'running build with\n'
printf 'container: %s\n' "$LIBFASTJSON_DEV_CONTAINER"
printf 'CC:\t%s\n' "$CC"
printf 'CFLAGS:\t%s\n' "$CFLAGS"
printf 'LDFLAGS:\t%s\n' "$LDFLAGS"
printf 'working directory: %s\n' "$(pwd)"
printf 'user ids: %s:%s\n' "$(id -u)" "$(id -g)"

if [ -n "$LIBFASTJSON_CONFIGURE_OPTIONS_OVERRIDE" ]; then
	CONFIGURE_OPTS="$LIBFASTJSON_CONFIGURE_OPTIONS_OVERRIDE"
else
	CONFIGURE_OPTS="$LIBFASTJSON_CONFIGURE_OPTIONS_EXTRA"
fi
printf 'CONFIGURE_OPTS:\t%s\n' "$CONFIGURE_OPTS"

printf 'STEP: autoreconf / configure ===============================================\n'
autoreconf -fvi
# shellcheck disable=SC2086
./configure $CONFIGURE_OPTS

printf 'STEP: make =================================================================\n'
# shellcheck disable=SC2086
make ${CI_MAKE_OPT:-}
