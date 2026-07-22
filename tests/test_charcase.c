#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../json.h"
#include "../json_tokener.h"
#include "../debug.h"

#define CHK(x) if (!(x)) { \
	printf("%s:%d: unexpected result with '%s'\n", \
		__FILE__, __LINE__, #x); \
	exit(1); \
}

static void test_case_parse(void);
static void test_object_lookup_case(void);

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	MC_SET_DEBUG(1);

	test_case_parse();
	test_object_lookup_case();
	return 0;
}

/* make sure only lowercase forms are parsed in strict mode */
static void test_case_parse(void)
{
	struct fjson_tokener *tok;
	fjson_object *new_obj;

	tok = fjson_tokener_new();
	fjson_tokener_set_flags(tok, FJSON_TOKENER_STRICT);

	new_obj = fjson_tokener_parse_ex(tok, "True", 4);
	CHK(new_obj == NULL);

	new_obj = fjson_tokener_parse_ex(tok, "False", 5);
	CHK(new_obj == NULL);

	new_obj = fjson_tokener_parse_ex(tok, "Null", 4);
	CHK(new_obj == NULL);

	printf("OK\n");

	fjson_tokener_free(tok);
}

/* Exercise object lookup across pages and deleted slots in both comparison modes. */
static void test_object_lookup_case(void)
{
	char key[16];
	fjson_object *obj = fjson_object_new_object();
	fjson_object *value;
	int i;

	CHK(obj != NULL);
	for (i = 0; i < 10; ++i) {
		snprintf(key, sizeof(key), "key-%02d", i);
		fjson_object_object_add(obj, key, fjson_object_new_int(i));
	}
	fjson_object_object_del(obj, "key-02");
	fjson_object_object_del(obj, "key-08");
	CHK(fjson_object_object_length(obj) == 8);
	CHK(fjson_object_object_get_ex(obj, "key-09", &value));
	CHK(fjson_object_get_int(value) == 9);

	fjson_global_do_case_sensitive_comparison(0);
	CHK(fjson_object_object_get_ex(obj, "KEY-09", &value));
	CHK(fjson_object_get_int(value) == 9);
	fjson_object_object_add(obj, "KEY-09", fjson_object_new_int(90));
	CHK(fjson_object_object_length(obj) == 8);
	CHK(fjson_object_object_get_ex(obj, "key-09", &value));
	CHK(fjson_object_get_int(value) == 90);

	fjson_global_do_case_sensitive_comparison(1);
	CHK(!fjson_object_object_get_ex(obj, "KEY-09", &value));
	CHK(value == NULL);
	CHK(fjson_object_object_get_ex(obj, "key-09", &value));
	fjson_object_put(obj);
}
