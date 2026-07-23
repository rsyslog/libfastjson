/*
 * Microbenchmark for libfastjson operations used heavily by rsyslog.
 *
 * Setup is outside the timed interval for lookup and replacement workloads.
 * Every workload verifies its final value, member count, and accumulated
 * result so an optimizing compiler or a broken library cannot silently turn
 * the timed loop into a no-op.
 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "json.h"

static volatile uint64_t result_sink;

static void
fail(const char *const message)
{
	fprintf(stderr, "benchmark error: %s\n", message);
	exit(1);
}

static uint64_t
parse_u64(const char *const text, const char *const name)
{
	char *end = NULL;
	errno = 0;
	const unsigned long long value = strtoull(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0') {
		fprintf(stderr, "benchmark error: invalid %s: %s\n", name, text);
		exit(2);
	}
	return (uint64_t) value;
}

static uint64_t
now_ns(void)
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) != 0)
		fail("clock_gettime failed");
	return (uint64_t) now.tv_sec * UINT64_C(1000000000) + (uint64_t) now.tv_nsec;
}

static char **
make_keys(const size_t width)
{
	char **const keys = calloc(width == 0 ? 1 : width, sizeof(*keys));
	if (keys == NULL)
		fail("key table allocation failed");
	for (size_t i = 0 ; i < width ; ++i) {
		keys[i] = malloc(32);
		if (keys[i] == NULL)
			fail("key allocation failed");
		snprintf(keys[i], 32, "key-%03zu", i);
	}
	return keys;
}

static void
free_keys(char **const keys, const size_t width)
{
	for (size_t i = 0 ; i < width ; ++i)
		free(keys[i]);
	free(keys);
}

static size_t
position_index(const char *const position, const size_t width)
{
	if (!strcmp(position, "first"))
		return 0;
	if (!strcmp(position, "middle"))
		return width / 2;
	if (!strcmp(position, "last"))
		return width - 1;
	if (!strcmp(position, "miss"))
		return width;
	fail("unknown position");
	return 0;
}

static struct fjson_object *
make_object(char **const keys, const size_t width, const int assume_new)
{
	struct fjson_object *const object = fjson_object_new_object();
	if (object == NULL)
		fail("object allocation failed");
	for (size_t i = 0 ; i < width ; ++i) {
		struct fjson_object *const value = fjson_object_new_int64((int64_t) i);
		if (value == NULL)
			fail("integer allocation failed");
		if (assume_new)
			fjson_object_object_add_ex(object, keys[i], value, FJSON_OBJECT_ADD_KEY_IS_NEW);
		else
			fjson_object_object_add(object, keys[i], value);
	}
	if ((size_t) fjson_object_object_length(object) != width)
		fail("object setup member count mismatch");
	return object;
}

static uint64_t
run_lookup(const size_t width, const char *const position, const uint64_t iterations)
{
	if (width == 0)
		fail("lookup width must be positive");
	char **const keys = make_keys(width + 1);
	struct fjson_object *const object = make_object(keys, width, 0);
	const size_t index = position_index(position, width);
	uint64_t checksum = 0;
	const uint64_t start = now_ns();
	for (uint64_t i = 0 ; i < iterations ; ++i) {
		struct fjson_object *value = NULL;
		const fjson_bool found = fjson_object_object_get_ex(object, keys[index], &value);
		if (index == width) {
			checksum += found ? UINT64_C(1000003) : 1;
		} else {
			if (!found || value == NULL)
				fail("lookup unexpectedly missed");
			checksum += (uint64_t) fjson_object_get_int64(value) + 1;
		}
	}
	const uint64_t elapsed = now_ns() - start;
	const uint64_t expected = iterations * (index == width ? 1 : index + 1);
	if (checksum != expected)
		fail("lookup checksum mismatch");
	result_sink = checksum;
	fjson_object_put(object);
	free_keys(keys, width + 1);
	return elapsed;
}

static uint64_t
run_replace(const size_t width, const char *const position, const uint64_t iterations)
{
	if (width == 0 || !strcmp(position, "miss"))
		fail("replacement requires a present key");
	char **const keys = make_keys(width);
	struct fjson_object *const object = make_object(keys, width, 0);
	const size_t index = position_index(position, width);
	uint64_t checksum = 0;
	const uint64_t start = now_ns();
	for (uint64_t i = 0 ; i < iterations ; ++i) {
		struct fjson_object *const value = fjson_object_new_int64((int64_t) i);
		if (value == NULL)
			fail("replacement value allocation failed");
		fjson_object_object_add(object, keys[index], value);
		checksum += i & 1;
	}
	const uint64_t elapsed = now_ns() - start;
	struct fjson_object *final_value = NULL;
	if (!fjson_object_object_get_ex(object, keys[index], &final_value) || final_value == NULL)
		fail("replacement final lookup failed");
	if (fjson_object_get_int64(final_value) != (int64_t) (iterations - 1))
		fail("replacement final value mismatch");
	if ((size_t) fjson_object_object_length(object) != width)
		fail("replacement changed member count");
	if (checksum != iterations / 2)
		fail("replacement checksum mismatch");
	result_sink = checksum;
	fjson_object_put(object);
	free_keys(keys, width);
	return elapsed;
}

static uint64_t
run_string(const size_t value_bytes, const uint64_t iterations)
{
	if (value_bytes == SIZE_MAX)
		fail("string value bytes too large");
	char *const value = malloc(value_bytes + 1);
	if (value == NULL)
		fail("string source allocation failed");
	for (size_t i = 0 ; i < value_bytes ; ++i)
		value[i] = (char) ('a' + (i % 23));
	value[value_bytes] = '\0';
	uint64_t checksum = 0;
	const uint64_t start = now_ns();
	for (uint64_t i = 0 ; i < iterations ; ++i) {
		struct fjson_object *const object = fjson_object_new_string(value);
		if (object == NULL)
			fail("string object allocation failed");
		if ((size_t) fjson_object_get_string_len(object) != value_bytes)
			fail("string length mismatch");
		const char *const copy = fjson_object_get_string(object);
		if (copy == NULL || (value_bytes > 0 && copy[value_bytes - 1] != value[value_bytes - 1]))
			fail("string content mismatch");
		checksum += value_bytes + (value_bytes == 0 ? 0 : (unsigned char) copy[0]);
		fjson_object_put(object);
	}
	const uint64_t elapsed = now_ns() - start;
	const uint64_t expected = iterations * (value_bytes + (value_bytes == 0 ? 0 : (unsigned char) value[0]));
	if (checksum != expected)
		fail("string checksum mismatch");
	result_sink = checksum;
	free(value);
	return elapsed;
}

static uint64_t
run_object(const size_t width, const uint64_t iterations, const int assume_new)
{
	char **const keys = make_keys(width);
	uint64_t checksum = 0;
	const uint64_t start = now_ns();
	for (uint64_t i = 0 ; i < iterations ; ++i) {
		struct fjson_object *const object = make_object(keys, width, assume_new);
		checksum += (uint64_t) fjson_object_object_length(object) + 1;
		fjson_object_put(object);
	}
	const uint64_t elapsed = now_ns() - start;
	if (checksum != iterations * (width + 1))
		fail("object lifecycle checksum mismatch");
	result_sink = checksum;
	free_keys(keys, width);
	return elapsed;
}

int
main(const int argc, char **const argv)
{
	if (argc != 7) {
		fprintf(stderr,
			"usage: %s OPERATION WIDTH POSITION VALUE_BYTES ITERATIONS CASE_SENSITIVE\n",
			argv[0]);
		return 2;
	}
	const char *const operation = argv[1];
	const size_t width = (size_t) parse_u64(argv[2], "width");
	const char *const position = argv[3];
	const size_t value_bytes = (size_t) parse_u64(argv[4], "value bytes");
	const uint64_t iterations = parse_u64(argv[5], "iterations");
	const uint64_t case_mode = parse_u64(argv[6], "case mode");
	if (iterations == 0 || case_mode > 1)
		fail("iterations and case mode are out of range");
	const int case_sensitive = (int) case_mode;
	fjson_global_do_case_sensitive_comparison(case_sensitive);

	uint64_t elapsed;
	if (!strcmp(operation, "lookup"))
		elapsed = run_lookup(width, position, iterations);
	else if (!strcmp(operation, "replace"))
		elapsed = run_replace(width, position, iterations);
	else if (!strcmp(operation, "string"))
		elapsed = run_string(value_bytes, iterations);
	else if (!strcmp(operation, "object"))
		elapsed = run_object(width, iterations, 0);
	else if (!strcmp(operation, "object-new"))
		elapsed = run_object(width, iterations, 1);
	else
		fail("unknown operation");

	printf("{\"operation\":\"%s\",\"width\":%zu,\"position\":\"%s\","
	       "\"value_bytes\":%zu,\"iterations\":%" PRIu64 ",\"case_sensitive\":%d,"
	       "\"elapsed_ns\":%" PRIu64 ",\"ns_per_operation\":%.9f,"
	       "\"operations_per_second\":%.6f,\"oracle\":true}\n",
	       operation, width, position, value_bytes, iterations, case_sensitive, elapsed,
	       (double) elapsed / (double) iterations,
	       (double) iterations * 1000000000.0 / (double) elapsed);
	return result_sink == UINT64_MAX ? 3 : 0;
}
