/* Copyright (C) 2026 by Rainer Gerhards
 * Released under ASL 2.0 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include "../json_object.h"

struct output {
	char data[32];
	size_t filled;
};

static size_t append(void *ptr, const char *buffer, size_t size)
{
	struct output *output = ptr;

	memcpy(output->data + output->filled, buffer, size);
	output->filled += size;
	return size;
}

static void dump(const char *name, char *temp, size_t size, struct fjson_object *json)
{
	struct output output = {{0}, 0};

	fjson_object_dump_buffered(json, FJSON_TO_STRING_PLAIN, temp, size, append, &output);
	printf("%s:%.*s\n", name, (int)output.filled, output.data);
}

int main(void)
{
	struct fjson_object *json = fjson_object_new_int64(123456789012345);
	char small[2];

	dump("zero", NULL, 0, json);
	dump("small", small, sizeof(small), json);
	fjson_object_put(json);
	return 0;
}
