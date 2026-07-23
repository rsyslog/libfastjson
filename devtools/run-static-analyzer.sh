#!/bin/bash
set -e
cd /rsyslog

printf 'SCAN_BUILD_CC: %s\n' "$SCAN_BUILD_CC"
printf 'SCAN_BUILD: %s\n' "$SCAN_BUILD"

if [ -n "$SCAN_BUILD_REPORT_DIR" ]; then
	CURRENT_REPORT=$(date +%y-%m-%d_%H-%M-%S)
	REPORT_DIR="$SCAN_BUILD_REPORT_DIR/$CURRENT_REPORT"
	export CURRENT_REPORT REPORT_DIR
fi

autoreconf -fvi
export CC="${SCAN_BUILD_CC:-clang}"
# shellcheck disable=SC2086
./configure \
	${LIBFASTJSON_CONFIGURE_OPTIONS_OVERRIDE:-} \
	${LIBFASTJSON_CONFIGURE_OPTIONS_EXTRA:-}

set +e
if [ -n "$REPORT_DIR" ]; then
	"${SCAN_BUILD:-scan-build}" -o "$REPORT_DIR" --use-cc "$CC" --status-bugs make
else
	"${SCAN_BUILD:-scan-build}" --use-cc "$CC" --status-bugs make
fi
result=$?
set -e

printf 'static analyzer result: %s\n' "$result"
exit "$result"
