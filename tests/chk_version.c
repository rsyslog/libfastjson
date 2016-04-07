#include "config.h"

#include "../json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	if(strcmp(fjson_version(), VERSION)) {
		fprintf(stderr, "ERROR: fjson_version reports '%s', VERSION is '%s'.\n",
			fjson_version(), VERSION);
		exit(1);
	}
	return 0;
}
