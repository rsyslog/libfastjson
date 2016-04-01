/*
* Tests if binary strings are supported.
*/

#include "config.h"
#include <stdio.h>
#include <string.h>

#include "json_inttypes.h"
#include "json_object.h"
#include "json_tokener.h"

int main()
{
	// this test has a space after the null character. check that it's still included
	const char *input = " \0 ";
	const char *expected = "\" \\u0000 \"";
	struct fjson_object *string = fjson_object_new_string_len(input, 3);
	const char *json = fjson_object_to_json_string(string);

	int strings_match =  !strcmp( expected, json);
	int retval = 0;
	if (strings_match)
	{
		printf("JSON write result is correct: %s\n", json);
		printf("PASS\n");
	} else {
		printf("JSON write result doesn't match expected string\n");
		printf("expected string: ");
		printf("%s\n", expected);
		printf("parsed string:   ");
		printf("%s\n", json);
		printf("FAIL\n");
		retval=1;
	}
	fjson_object_put(string);

	struct fjson_object *parsed_str = fjson_tokener_parse(expected);
	if (parsed_str)
	{
		int parsed_len = fjson_object_get_string_len(parsed_str);
		const char *parsed_cstr = fjson_object_get_string(parsed_str);
		int ii;
		printf("Re-parsed object string len=%d, chars=[", parsed_len);
		for (ii = 0; ii < parsed_len ; ii++)
		{
			printf("%s%d", (ii ? ", " : ""), (int)parsed_cstr[ii]);
		}
		printf("]\n");
		fjson_object_put(parsed_str);
	}
	else
	{
		printf("ERROR: failed to parse\n");
	}
	return retval;
}
