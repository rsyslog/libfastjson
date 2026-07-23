/* Parser fuzz target and deterministic regression-corpus replay. */
#include "config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../json.h"

#define FUZZ_MAX_INPUT (64 * 1024)

static void
check_canonical(struct fjson_object *object)
{
	const char *const encoded = fjson_object_to_json_string_ext(
		object, FJSON_TO_STRING_PLAIN);
	struct fjson_object *reparsed;

	if (encoded == NULL)
		return;
	reparsed = fjson_tokener_parse(encoded);

	if (reparsed != NULL) {
		const char *const reencoded = fjson_object_to_json_string_ext(
			reparsed, FJSON_TO_STRING_PLAIN);
		if (reencoded == NULL || strcmp(encoded, reencoded) != 0)
			abort();
		fjson_object_put(reparsed);
	}
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct fjson_tokener *tokener;
	struct fjson_object *object;
	int chunk_size;
	size_t offset;

	if (size > FUZZ_MAX_INPUT)
		return 0;

	tokener = fjson_tokener_new_ex(64);
	if (tokener == NULL)
		return 0;
	if (size > 0 && (data[0] & 1) != 0)
		fjson_tokener_set_flags(tokener, FJSON_TOKENER_STRICT);

	object = fjson_tokener_parse_ex(tokener, (const char *) data, (int) size);
	if (object != NULL) {
		check_canonical(object);
		fjson_object_put(object);
	}
	fjson_tokener_free(tokener);

	tokener = fjson_tokener_new_ex(64);
	if (tokener == NULL)
		return 0;
	if (size > 0 && (data[0] & 1) != 0)
		fjson_tokener_set_flags(tokener, FJSON_TOKENER_STRICT);
	chunk_size = size == 0 ? 1 : (int) (data[0] % 17) + 1;
	for (offset = 0 ; offset < size ; ) {
		const size_t remaining = size - offset;
		const int length = (int) (remaining < (size_t) chunk_size ?
			remaining : (size_t) chunk_size);

		object = fjson_tokener_parse_ex(
			tokener, (const char *) data + offset, length);
		offset += (size_t) length;
		if (object != NULL) {
			if (fjson_tokener_get_error(tokener) == fjson_tokener_success)
				check_canonical(object);
			fjson_object_put(object);
			break;
		}
		if (fjson_tokener_get_error(tokener) != fjson_tokener_continue)
			break;
	}
	fjson_tokener_free(tokener);
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
