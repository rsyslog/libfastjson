#!/bin/bash
# Generic CI entry point for development containers.
set -e

printf 'running CI with\n'
printf 'container: %s\n' "$LIBFASTJSON_DEV_CONTAINER"
printf 'CC:\t%s\n' "$CC"
printf 'CFLAGS:\t%s\n' "$CFLAGS"
printf 'LDFLAGS:\t%s\n' "$LDFLAGS"
printf 'CI_MAKE_CHECK_EXTRA:\t%s\n' "$CI_MAKE_CHECK_EXTRA"
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

if [ "${CI_CHECK_CMD:-check}" != "distcheck" ]; then
	printf 'STEP: make =================================================================\n'
	# shellcheck disable=SC2086
	make ${CI_MAKE_OPT:-}
fi

printf 'STEP: make %s ==============================================================\n' \
	"${CI_CHECK_CMD:-check}"
set +e
# shellcheck disable=SC2086
make ${CI_MAKE_CHECK_OPT:-} ${CI_MAKE_CHECK_EXTRA:-} "${CI_CHECK_CMD:-check}"
rc=$?
set -e

printf 'STEP: find failing tests ====================================================\n'
devtools/gather-check-logs.sh
exit "$rc"
