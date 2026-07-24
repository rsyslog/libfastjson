/* Serializer fuzz target and deterministic regression-corpus replay. */
#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../json.h"

#define FUZZ_MAX_INPUT (64 * 1024)
#define FUZZ_MAX_NODES 256

struct cursor {
	const uint8_t *data;
	size_t size;
	size_t offset;
	unsigned int nodes;
};

struct output {
	char data[16384];
	size_t filled;
	size_t limit;
};

static uint8_t
next_byte(struct cursor *cursor)
{
	if (cursor->offset < cursor->size)
		return cursor->data[cursor->offset++];
	return 0;
}

static size_t
append(void *ptr, const char *buffer, size_t size)
{
	struct output *output = ptr;
	size_t accepted = size;

	if (output->limit > 0 && accepted > output->limit)
		accepted = output->limit;
	if (accepted > sizeof(output->data) - output->filled)
		abort();
	memcpy(output->data + output->filled, buffer, accepted);
	output->filled += accepted;
	return accepted;
}

static struct fjson_object *build_value(struct cursor *cursor, unsigned int depth);

static struct fjson_object *
build_string(struct cursor *cursor)
{
	static const char alphabet[] = "ab\\\"/\n";
	char value[17];
	const size_t length = next_byte(cursor) % (sizeof(value) - 1);

	for (size_t i = 0 ; i < length ; ++i)
		value[i] = alphabet[next_byte(cursor) % (sizeof(alphabet) - 1)];
	value[length] = '\0';
	return fjson_object_new_string_len(value, (int) length);
}

static struct fjson_object *
build_value(struct cursor *cursor, unsigned int depth)
{
	const uint8_t kind = next_byte(cursor) % 5;
	struct fjson_object *object;

	if (++cursor->nodes > FUZZ_MAX_NODES || depth >= 6)
		return fjson_object_new_int((int) next_byte(cursor));
	switch (kind) {
	case 0:
		return fjson_object_new_boolean(next_byte(cursor) & 1);
	case 1:
		return fjson_object_new_int64((int64_t) next_byte(cursor) - 128);
	case 2:
		return build_string(cursor);
	case 3:
		object = fjson_object_new_array();
		if (object == NULL)
			return NULL;
		for (unsigned int i = 0 ; i < next_byte(cursor) % 4 ; ++i)
			fjson_object_array_add(object, build_value(cursor, depth + 1));
		return object;
	default:
		object = fjson_object_new_object();
		if (object == NULL)
			return NULL;
		for (unsigned int i = 0 ; i < next_byte(cursor) % 4 ; ++i) {
			char key[16];
			snprintf(key, sizeof(key), "key-%u-%u", depth, i);
			fjson_object_object_add(object, key, build_value(cursor, depth + 1));
		}
		return object;
	}
}

static void
check_serialization(struct fjson_object *object, int flags)
{
	const char *const encoded = fjson_object_to_json_string_ext(object, flags);
	size_t length;
	char *expected;
	const size_t sizes[] = {0, 1, 2, 7, 31, 128};

	if (encoded == NULL)
		return;
	length = strlen(encoded);
	expected = malloc(length + 1);
	if (expected == NULL)
		abort();
	memcpy(expected, encoded, length + 1);
	if (fjson_object_size_ext(object, flags) != length)
		abort();
	for (size_t i = 0 ; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
		char temp[128];
		struct output output = {{0}, 0, 0};
		const size_t written = fjson_object_dump_buffered(
			object, flags, sizes[i] == 0 ? NULL : temp, sizes[i], append, &output);

		if (written != length || output.filled != length ||
			memcmp(output.data, expected, length) != 0)
			abort();
	}
	{
		char temp[7];
		struct output output = {{0}, 0, 1};
		const size_t written = fjson_object_dump_buffered(
			object, flags, temp, sizeof(temp), append, &output);

		if (written != output.filled || written > length)
			abort();
	}
	if (flags == FJSON_TO_STRING_PLAIN) {
		struct fjson_object *const reparsed = fjson_tokener_parse(expected);
		const char *reencoded;

		if (reparsed == NULL)
			abort();
		reencoded = fjson_object_to_json_string_ext(
			reparsed, FJSON_TO_STRING_PLAIN);
		if (reencoded == NULL || strcmp(reencoded, expected) != 0)
			abort();
		fjson_object_put(reparsed);
	}
	free(expected);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct cursor cursor = {data, size, 0, 0};
	struct fjson_object *object;

	if (size > FUZZ_MAX_INPUT)
		return 0;
	object = build_value(&cursor, 0);
	if (object == NULL)
		return 0;
	check_serialization(object, FJSON_TO_STRING_PLAIN);
	check_serialization(object, FJSON_TO_STRING_SPACED);
	fjson_object_put(object);
	return 0;
}

#ifdef FUZZ_REPLAY
static uint8_t *
read_input(const char *path, size_t *size)
{
	FILE *file = fopen(path, "rb");
	long file_size;
	uint8_t *data;

	if (file == NULL)
		return NULL;
	if (fseek(file, 0, SEEK_END) != 0 ||
		(file_size = ftell(file)) < 0 ||
		fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		return NULL;
	}
	data = malloc((size_t) file_size + 1);
	if (data == NULL || fread(data, 1, (size_t) file_size, file) !=
		(size_t) file_size) {
		free(data);
		fclose(file);
		return NULL;
	}
	fclose(file);
	*size = (size_t) file_size;
	return data;
}

int
main(int argc, char **argv)
{
	uint8_t *data;
	size_t size;
	int result;

	if (argc != 2)
		return 2;
	data = read_input(argv[1], &size);
	if (data == NULL)
		return 2;
	result = LLVMFuzzerTestOneInput(data, size);
	free(data);
	return result;
}
#endif
