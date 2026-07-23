/* Copyright (C) 2026 by Rainer Gerhards
 * Released under ASL 2.0 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../json_object.h"

#define CHK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", \
			__FILE__, __LINE__, #condition); \
		exit(1); \
	} \
} while (0)

struct output {
	char data[256];
	size_t filled;
	size_t calls;
	size_t max_write;
	int reject;
	int overflow;
};

static size_t append(void *ptr, const char *buffer, size_t size)
{
	struct output *output = ptr;
	size_t accepted = size;

	++output->calls;
	if (output->reject)
		return 0;
	if (output->max_write > 0 && accepted > output->max_write)
		accepted = output->max_write;
	if (accepted > sizeof(output->data) - output->filled) {
		output->overflow = 1;
		return 0;
	}
	memcpy(output->data + output->filled, buffer, accepted);
	output->filled += accepted;
	return accepted;
}

static struct fjson_object *
new_document(void)
{
	struct fjson_object *document = fjson_object_new_object();
	struct fjson_object *values = fjson_object_new_array();

	CHK(document != NULL);
	CHK(values != NULL);
	CHK(fjson_object_array_add(values, fjson_object_new_int(1)) == 0);
	CHK(fjson_object_array_add(values, fjson_object_new_boolean(1)) == 0);
	CHK(fjson_object_array_add(values, NULL) == 0);
	fjson_object_object_add(
		document, "name", fjson_object_new_string("a\"b\n"));
	fjson_object_object_add(document, "values", values);
	return document;
}

static void
check_buffer_size(struct fjson_object *document, const char *expected, size_t size)
{
	char temp[256];
	struct output output = {{0}, 0, 0, 0, 0, 0};
	const size_t written = fjson_object_dump_buffered(
		document, FJSON_TO_STRING_PLAIN,
		size == 0 ? NULL : temp, size, append, &output);

	CHK(!output.overflow);
	CHK(written == strlen(expected));
	CHK(output.filled == strlen(expected));
	CHK(memcmp(output.data, expected, output.filled) == 0);
	CHK(output.calls > 0);
}

static void
check_file_write(struct fjson_object *document, int flags, const char *expected)
{
	char actual[256];
	FILE *file = tmpfile();
	size_t written;

	CHK(file != NULL);
	if (flags == FJSON_TO_STRING_SPACED)
		written = fjson_object_write(document, file);
	else
		written = fjson_object_write_ext(document, flags, file);
	CHK(written == strlen(expected));
	CHK(fflush(file) == 0);
	CHK(fseek(file, 0, SEEK_SET) == 0);
	CHK(fread(actual, 1, sizeof(actual), file) == strlen(expected));
	CHK(memcmp(actual, expected, strlen(expected)) == 0);
	CHK(fclose(file) == 0);
}

int main(void)
{
	static const char plain[] =
		"{\"name\":\"a\\\"b\\n\",\"values\":[1,true,null]}";
	static const char spaced[] =
		"{ \"name\": \"a\\\"b\\n\", \"values\": [ 1, true, null ] }";
	const size_t sizes[] = {
		0, 1, 2, 3, 4, 7, 8,
		sizeof(plain) - 2, sizeof(plain) - 1, sizeof(plain)
	};
	struct fjson_object *document = new_document();
	struct output output = {{0}, 0, 0, 0, 0, 0};
	char temp[4];
	size_t written;

	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i)
		check_buffer_size(document, plain, sizes[i]);

	written = fjson_object_dump_ext(
		document, FJSON_TO_STRING_PLAIN, append, &output);
	CHK(written == strlen(plain));
	CHK(output.filled == strlen(plain));
	CHK(memcmp(output.data, plain, output.filled) == 0);

	memset(&output, 0, sizeof(output));
	written = fjson_object_dump(document, append, &output);
	CHK(written == strlen(spaced));
	CHK(output.filled == strlen(spaced));
	CHK(memcmp(output.data, spaced, output.filled) == 0);
	CHK(fjson_object_size_ext(
		document, FJSON_TO_STRING_PLAIN) == strlen(plain));
	CHK(fjson_object_size(document) == strlen(spaced));

	check_file_write(document, FJSON_TO_STRING_PLAIN, plain);
	check_file_write(document, FJSON_TO_STRING_SPACED, spaced);

	memset(&output, 0, sizeof(output));
	output.max_write = 1;
	written = fjson_object_dump_buffered(
		document, FJSON_TO_STRING_PLAIN,
		temp, sizeof(temp), append, &output);
	CHK(!output.overflow);
	CHK(output.calls > 1);
	CHK(written == output.filled);
	CHK(written < strlen(plain));

	memset(&output, 0, sizeof(output));
	output.reject = 1;
	written = fjson_object_dump_buffered(
		document, FJSON_TO_STRING_PLAIN,
		temp, sizeof(temp), append, &output);
	CHK(written == 0);
	CHK(output.filled == 0);
	CHK(output.calls > 0);

	fjson_object_put(document);
	puts("OK");
	return 0;
}
