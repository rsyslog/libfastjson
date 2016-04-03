#include "config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../json.h"
#include "../printbuf.h"

struct myinfo {
	int value;
};

static int freeit_was_called = 0;
static void freeit(fjson_object *jso, void *userdata)
{
	struct myinfo *info = userdata;
	printf("freeit, value=%d\n", info->value);
	// Don't actually free anything here, the userdata is stack allocated.
	freeit_was_called = 1;
}
static int custom_serializer(struct fjson_object *o,
					struct printbuf *pb,
					int level,
					int flags)
{
	sprintbuf(pb, "Custom Output");
	return 0;
}

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *my_object;

	MC_SET_DEBUG(1);

	printf("Test setting, then resetting a custom serializer:\n");
	my_object = fjson_object_new_object();
	fjson_object_object_add(my_object, "abc", fjson_object_new_int(12));
	fjson_object_object_add(my_object, "foo", fjson_object_new_string("bar"));

	printf("my_object.to_string(standard)=%s\n", fjson_object_to_json_string(my_object));

	struct myinfo userdata = { .value = 123 };
	fjson_object_set_serializer(my_object, custom_serializer, &userdata, freeit);

	printf("my_object.to_string(custom serializer)=%s\n", fjson_object_to_json_string(my_object));

	printf("Next line of output should be from the custom freeit function:\n");
	freeit_was_called = 0;
	fjson_object_set_serializer(my_object, NULL, NULL, NULL);
	assert(freeit_was_called);

	printf("my_object.to_string(standard)=%s\n", fjson_object_to_json_string(my_object));

	fjson_object_put(my_object);

	// ============================================

	my_object = fjson_object_new_object();
	printf("Check that the custom serializer isn't free'd until the last fjson_object_put:\n");
	fjson_object_set_serializer(my_object, custom_serializer, &userdata, freeit);
	fjson_object_get(my_object);
	fjson_object_put(my_object);
	printf("my_object.to_string(custom serializer)=%s\n", fjson_object_to_json_string(my_object));
	printf("Next line of output should be from the custom freeit function:\n");

	freeit_was_called = 0;
	fjson_object_put(my_object);
	assert(freeit_was_called);

	return 0;
}
