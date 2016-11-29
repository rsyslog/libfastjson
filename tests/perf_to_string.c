/* libfastjson testbench tool
 * This program can be used to test the performance of the _to_string()
 * subsystem. It is meant to be used together with a profiler.
 *
 * Copyright (c) 2016 Adiscon GmbH
 * Rainer Gerhards <rgerhards@adiscon.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "config.h"

#include "../json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_ITERATIONS 100000

int
main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	int i;
	struct fjson_object *json;

	json = fjson_object_new_object();
	fjson_object_object_add(json, "string1", fjson_object_new_string("This is a test"));
	fjson_object_object_add(json, "string2", fjson_object_new_string("This is a "
		"loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
		"ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
		"ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong"
		"test"));
	fjson_object_object_add(json, "string3", fjson_object_new_string("This is\n a test öäü"));
	fjson_object_object_add(json, "int1", fjson_object_new_int(4711));


	for(i = 0 ; i < NUM_ITERATIONS ; ++i) {
		const char *dummy = fjson_object_to_json_string(json);
		if(dummy == NULL) {
			fprintf(stderr, "dummy has received no output!");
			exit(1);
		}
		if(i == 0)
			printf("%s\n", dummy);
	}

	fjson_object_put(json);
	return 0;
}
