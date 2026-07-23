#include "config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../json.h"

#define CHK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", \
			__FILE__, __LINE__, #condition); \
		exit(1); \
	} \
} while (0)

static struct fjson_object *
new_document(void)
{
	struct fjson_object *document = fjson_object_new_object();
	struct fjson_object *items = fjson_object_new_array();

	CHK(document != NULL);
	CHK(items != NULL);
	CHK(fjson_object_array_add(items, fjson_object_new_int(3)) == 0);
	CHK(fjson_object_array_add(items, fjson_object_new_int(1)) == 0);
	CHK(fjson_object_array_add(items, fjson_object_new_int(2)) == 0);
	fjson_object_object_add(document, "name", fjson_object_new_string("value"));
	fjson_object_object_add(document, "items", items);
	return document;
}

static void
check_document(struct fjson_object *document)
{
	const char *const expected = "{\"name\":\"value\",\"items\":[3,1,2]}";

	CHK(document != NULL);
	CHK(strcmp(fjson_object_to_json_string_ext(
		document, FJSON_TO_STRING_PLAIN), expected) == 0);
}

int
main(void)
{
	const char *const plain_path = "test_json_util-plain.json";
	const char *const pretty_path = "test_json_util-pretty.json";
	const char *const fd_path = "test_json_util-fd.json";
	const char *const serialized = "{\"name\":\"value\",\"items\":[3,1,2]}";
	struct fjson_object *document = new_document();
	struct fjson_object *parsed;
	int fd;

	CHK(fjson_object_to_file(plain_path, document) == 0);
	parsed = fjson_object_from_file(plain_path);
	check_document(parsed);
	fjson_object_put(parsed);

	CHK(fjson_object_to_file_ext(
		pretty_path, document, FJSON_TO_STRING_PRETTY) == 0);
	parsed = fjson_object_from_file(pretty_path);
	check_document(parsed);
	fjson_object_put(parsed);

	fd = open(fd_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	CHK(fd >= 0);
	CHK(write(fd, "xxx", 3) == 3);
	CHK(write(fd, serialized, strlen(serialized)) == (ssize_t) strlen(serialized));
	CHK(close(fd) == 0);

	fd = open(fd_path, O_RDONLY);
	CHK(fd >= 0);
	CHK(lseek(fd, 3, SEEK_SET) == 3);
	parsed = fjson_object_from_fd(fd);
	CHK(close(fd) == 0);
	check_document(parsed);
	fjson_object_put(parsed);

	CHK(unlink(plain_path) == 0);
	CHK(unlink(pretty_path) == 0);
	CHK(unlink(fd_path) == 0);
	fjson_object_put(document);
	puts("OK");
	return 0;
}
