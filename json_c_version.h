/*
 * Copyright (c) 2012 Eric Haszlakiewicz
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 */

#ifndef _fj_json_c_version_h_
#define _fj_json_c_version_h_

#define FJSON_C_MAJOR_VERSION 0
#define FJSON_C_MINOR_VERSION 12
#define FJSON_C_MICRO_VERSION 99
#define FJSON_C_VERSION_NUM ((FJSON_C_MAJOR_VERSION << 16) | \
                            (FJSON_C_MINOR_VERSION << 8) | \
                            FJSON_C_MICRO_VERSION)
#define FJSON_C_VERSION "0.12.99"

const char *fjson_c_version(void); /* Returns FJSON_C_VERSION */
int fjson_c_version_num(void);     /* Returns FJSON_C_VERSION_NUM */

#endif
