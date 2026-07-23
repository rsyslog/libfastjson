#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "../json.h"

#define CHK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", \
			__FILE__, __LINE__, #condition); \
		exit(1); \
	} \
} while (0)

static int
compare_ints(const void *left, const void *right)
{
	const struct fjson_object *const left_object =
		*(const struct fjson_object *const *) left;
	const struct fjson_object *const right_object =
		*(const struct fjson_object *const *) right;
	const int left_value = fjson_object_get_int((struct fjson_object *) left_object);
	const int right_value = fjson_object_get_int((struct fjson_object *) right_object);

	return (left_value > right_value) - (left_value < right_value);
}

int
main(void)
{
	struct fjson_object *array = fjson_object_new_array();
	struct fjson_object *non_array = fjson_object_new_int(1);
	struct fjson_object *key;
	struct fjson_object *found;

	CHK(array != NULL);
	CHK(non_array != NULL);
	CHK(fjson_object_get_array(array) != NULL);
	CHK(fjson_object_get_array(non_array) == NULL);
	fjson_object_put(non_array);

	CHK(fjson_object_array_add(array, fjson_object_new_int(30)) == 0);
	CHK(fjson_object_array_add(array, fjson_object_new_int(10)) == 0);
	CHK(fjson_object_array_add(array, fjson_object_new_int(20)) == 0);
	CHK(fjson_object_array_length(array) == 3);

	fjson_object_array_sort(array, compare_ints);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 0)) == 10);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 1)) == 20);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 2)) == 30);

	key = fjson_object_new_int(20);
	found = fjson_object_array_bsearch(key, array, compare_ints);
	CHK(found != NULL);
	CHK(fjson_object_get_int(found) == 20);
	fjson_object_put(key);

	key = fjson_object_new_int(25);
	CHK(fjson_object_array_bsearch(key, array, compare_ints) == NULL);
	fjson_object_put(key);

	CHK(fjson_object_array_put_idx(array, 1, fjson_object_new_int(25)) == 0);
	CHK(fjson_object_array_length(array) == 3);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 1)) == 25);
	fjson_object_array_del_idx(array, 1);
	CHK(fjson_object_array_length(array) == 2);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 1)) == 30);
	fjson_object_array_del_idx(array, -1);
	fjson_object_array_del_idx(array, 99);
	CHK(fjson_object_array_length(array) == 2);

	CHK(fjson_object_array_put_idx(array, 4, fjson_object_new_int(50)) == 0);
	CHK(fjson_object_array_length(array) == 5);
	CHK(fjson_object_array_get_idx(array, 2) == NULL);
	CHK(fjson_object_array_get_idx(array, 3) == NULL);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 4)) == 50);
	CHK(fjson_object_array_get_idx(array, 5) == NULL);

	fjson_object_array_del_idx(array, 0);
	CHK(fjson_object_array_length(array) == 4);
	CHK(fjson_object_get_int(fjson_object_array_get_idx(array, 0)) == 30);
	fjson_object_array_del_idx(array, 3);
	CHK(fjson_object_array_length(array) == 3);

	fjson_object_put(array);
	puts("OK");
	return 0;
}
