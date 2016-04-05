/*
 * Copyright (c) 2012 Eric Haszlakiewicz
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 */
#include "config.h"

#include "json_c_version.h"

const char *fjson_c_version(void)
{
	return FJSON_VERSION;
}

int fjson_c_version_num(void)
{
	return FJSON_VERSION_NUM;
}

