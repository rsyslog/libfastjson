#!/bin/bash
# Run a command in an rsyslog development container.
#
# LIBFASTJSON_CONTAINER_UID controls the container user in uid:gid form.
# Set it to an empty string to use the container default.
set -e

if [ "$1" = "--rm" ]; then
	optrm="--rm"
	shift
fi
if [ "$1" = "-ti" ]; then
	ti="-ti"
	shift
fi
if [ "$1" = "--rm" ]; then
	optrm="--rm"
	shift
fi

if [ -z "$LIBFASTJSON_HOME" ]; then
	LIBFASTJSON_HOME=$(pwd)
	export LIBFASTJSON_HOME
	printf 'info: LIBFASTJSON_HOME not set, using %s\n' "$LIBFASTJSON_HOME"
fi

if [ -z "$LIBFASTJSON_DEV_CONTAINER" ]; then
	LIBFASTJSON_DEV_CONTAINER=$(cat "$LIBFASTJSON_HOME/devtools/default_dev_container")
	export LIBFASTJSON_DEV_CONTAINER
fi

printf '/rsyslog is mapped to %s\n' "$LIBFASTJSON_HOME"
printf 'using container %s\n' "$LIBFASTJSON_DEV_CONTAINER"
printf 'pulling container...\n'
printf 'user ids: %s:%s\n' "$(id -u)" "$(id -g)"
printf 'container_uid: %s\n' "${LIBFASTJSON_CONTAINER_UID--u $(id -u):$(id -g)}"
printf 'container cmd: %s\n' "$*"
printf '\nThe rsyslog development containers use /rsyslog as the project home.\n\n'

docker pull "$LIBFASTJSON_DEV_CONTAINER"
# Deliberately allow word splitting for caller-supplied Docker options and command.
# shellcheck disable=SC2086
docker run $ti $optrm $DOCKER_RUN_EXTRA_OPTS \
	-e LIBFASTJSON_DEV_CONTAINER \
	-e LIBFASTJSON_CONFIGURE_OPTIONS_EXTRA \
	-e LIBFASTJSON_CONFIGURE_OPTIONS_OVERRIDE \
	-e CC \
	-e CFLAGS \
	-e LDFLAGS \
	-e ASAN_OPTIONS \
	-e LSAN_OPTIONS \
	-e TSAN_OPTIONS \
	-e UBSAN_OPTIONS \
	-e CI_MAKE_OPT \
	-e CI_MAKE_CHECK_OPT \
	-e CI_MAKE_CHECK_EXTRA \
	-e CI_CHECK_CMD \
	--cap-add SYS_ADMIN \
	--cap-add SYS_PTRACE \
	${LIBFASTJSON_CONTAINER_UID--u $(id -u):$(id -g)} \
	$DOCKER_RUN_EXTRA_FLAGS \
	-v "$LIBFASTJSON_HOME":/rsyslog \
	"$LIBFASTJSON_DEV_CONTAINER" "$@"
