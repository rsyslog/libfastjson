/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 * Copyright (c) 2015 Rainer Gerhards
 * Copyright (c) 2016 Copernica BV
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>

#include "debug.h"
#include "atomic.h"
#include "printbuf.h"
#include "arraylist.h"
#include "json_object.h"
#include "json_object_private.h"
#include "json_object_iterator.h"
#include "json_util.h"

#if !defined(HAVE_STRDUP)
# error You do not have strdup on your system.
#endif /* HAVE_STRDUP */

#if !defined(HAVE_SNPRINTF)
# error You do not have snprintf on your system.
#endif /* HAVE_SNPRINTF */

const char *fjson_number_chars = "0123456789.+-eE";
const char *fjson_hex_chars = "0123456789abcdefABCDEF";



/** 
 *  helper for accessing the optimized string data component in fjson_object
 *  @param  jso
 *  @return 
 */
static const char *get_string_component(struct fjson_object *jso)
{
	return (jso->o.c_string.len < LEN_DIRECT_STRING_DATA) ?
		   jso->o.c_string.str.data : jso->o.c_string.str.ptr;
}

/** 
 *  string escaping
 *
 *  String escaping is a surprisingly performance intense operation.
 *  I spent many hours in the profiler, and the root problem seems
 *  to be that there is no easy way to detect the character classes
 *  that need to be escaped, where the root cause is that these
 *  characters are spread all over the ascii table. I tried
 *  several approaches, including call tables, re-structuring
 *  the case condition, different types of if conditions and
 *  reordering the if conditions. What worked out best is this:
 *  The regular case is that a character must not be escaped. So
 *  we want to process that as fast as possible. In order to
 *  detect this as quickly as possible, we have a lookup table
 *  that tells us if a char needs escaping ("needsEscape", below).
 *  This table has a spot for each ascii code. Note that it uses
 *  chars, because anything larger causes worse cache operation
 *  and anything smaller requires bit indexing and masking
 *  operations, which are also comparatively costly. So plain
 *  chars work best. What we then do is a single lookup into the
 *  table to detect if we need to escape a character. If we need,
 *  we go into the depth of actual escape detection. But if we
 *  do NOT need to escape, we just quickly advance the index
 *  and are done with that char. Note that it may look like the
 *  extra table lookup costs performance, but right the contrary
 *  is the case. We get amore than 30% performance increase due
 *  to it (compared to the latest version of the code that did not
 *  do the lookups.
 *  rgerhards@adiscon.com, 2015-11-18
 */
static char needsEscape[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, /* ascii codes 0 .. 7 */
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 0, /* ascii codes 32 .. 39 */
	0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/**
 *  Function to escape a string
 *  @param  str     the string to be escaped
 *  @param  func    user supplied write function
 *  @param  ptr     user supplied pointer
 *  @return size_t  number of bytes written
 */
static size_t escape(const char *str, fjson_print_fn *func, void *ptr)
{
    size_t result = 0;
	const char *start_offset = str;
	while(1) { /* broken below on 0-byte */
		if(needsEscape[*((unsigned char*)str)]) {
			if(*str == '\0') break;
			if(str != start_offset) result += func(ptr, start_offset, str - start_offset);
			switch(*str) {
			case '\b':  result += func(ptr, "\\b", 2); break;
			case '\n':  result += func(ptr, "\\n", 2); break;
			case '\r':  result += func(ptr, "\\r", 2); break;
			case '\t':  result += func(ptr, "\\t", 2); break;
			case '\f':  result += func(ptr, "\\f", 2); break;
			case '"':   result += func(ptr, "\\\"", 2); break;
			case '\\':  result += func(ptr, "\\\\", 2); break;
			case '/':   result += func(ptr, "\\/", 2); break;
			default: 
                char tempbuf[6];
                auto bytes = sprintf(tempbuf, "\\u00%c%c", fjson_hex_chars[*str >> 4], fjson_hex_chars[*str & 0xf]);
                result += func(ptr, tempbuf, bytes);
				break;
			}
			start_offset = ++str;
		} else
			++str;
	}
	if(str != start_offset) result += func(ptr, start_offset, str - start_offset);
    return result;
}

/* add indentation */

static void indent(int level, int flags, fjson_write_fn *func, void *ptr)
{
    // skip if pretty-printing is not needed
	if (!(flags & FJSON_TO_STRING_PRETTY)) return;
    
    // result variable
    size_t result = 0;
    
    // iterate to add the spaces
    for (size_t i = 0; i < level; ++i)
    {
        // write a tab or two spaces
        if (flags & FJSON_TO_STRING_PRETTY_TAB) result += func(ptr, "\t", 1);
        else result += func(ptr, "  ", 2);
    }
    
    // done
    return result;
}

/* write a json object */

static size_t write_object(struct fjson_object* jso, int level, int flags, fjson_write_fn *func, void *ptr)
{
	struct fjson_object *val;
	int had_children = 0;
    size_t result = 0;

	result += func(ptr, "{" /*}*/, 1);
	if (flags & FJSON_TO_STRING_PRETTY) result += func(ptr, "\n", 1);
	struct fjson_object_iterator it = fjson_object_iter_begin(jso);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(jso);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		if (had_children)
		{
			result += func(ptr, ",", 1);
			if (flags & FJSON_TO_STRING_PRETTY) result += func(ptr, "\n", 1);
		}
		had_children = 1;
		if (flags & FJSON_TO_STRING_SPACED) result += func(ptr, " ", 1);
		result += indent(level+1, flags, func, ptr);
		result += func(ptr, "\"", 1);
		result += escape(fjson_object_iter_peek_name(&it), func, ptr);
		if (flags & FJSON_TO_STRING_SPACED) result += func(ptr, "\": ", 3);
		else result += func(ptr, "\":", 2);
		result += write(fjson_object_iter_peek_value(&it), level+1, flags, func, ptr);
		fjson_object_iter_next(&it);
	}
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (had_children) result += func(ptr, "\n", 1);
		result += indent(level, flags, func, ptr);
	}
	if (flags & FJSON_TO_STRING_SPACED) result += func(ptr, /*{*/ " }", 2);
	else result += func(ptr, /*{*/ "}", 1);
	return result;
}

/* write a json boolean */

static size_t write_boolean(struct fjson_object* jso, fjson_write_fn *func, void *ptr)
{
	if (jso->o.c_boolean) return func(ptr, "true", 4);
    else return func(ptr, "false", 5);
}

/* write a json int */

static size_t write_int(struct fjson_object* jso, fjson_write_fn *func, void *ptr)
{
    // temporary buffer
    char tempbuffer[32];
    size_t bytes = sprintf(tempbuffer, "%" PRId64, jso->o.c_int64);
	return func(ptr, tempbuffer, bytes);
}

/* write a json floating point */

static size_t write_double(struct fjson_object* jso, fjson_write_fn *func, void *ptr)
{
	char buf[128], *p, *q;
	int size;
	/* Although JSON RFC does not support
	 * NaN or Infinity as numeric values
	 * ECMA 262 section 9.8.1 defines
	 * how to handle these cases as strings
	 */
	if(isnan(jso->o.c_double)) 
		size = snprintf(buf, sizeof(buf), "NaN");
	else if(isinf(jso->o.c_double))
		if(jso->o.c_double > 0)
			size = snprintf(buf, sizeof(buf), "Infinity");
		else
			size = snprintf(buf, sizeof(buf), "-Infinity");
	else
		size = snprintf(buf, sizeof(buf), "%.17g", jso->o.c_double);

	p = strchr(buf, ',');
	if (p) {
		*p = '.';
	} else {
		p = strchr(buf, '.');
	}
	if (p && (flags & FJSON_TO_STRING_NOZERO)) {
		/* last useful digit, always keep 1 zero */
		p++;
		for (q=p ; *q ; q++) {
			if (*q!='0') p=q;
		}
		/* drop trailing zeroes */
		*(++p) = 0;
		size = p-buf;
	}
	return func(ptr, buf, size);
}

/* write a json string */

static size_t write_string(struct fjson_object* jso, fjson_write_fn *func, void *ptr)
{
	return func(ptr, "\"", 1) + escape(get_string_component(jso), func, ptr) + func(ptr, "\"", 1);
}

/* write a json array */

static size_t write_array(struct fjson_object* jso, int level, int flags, fjson_write_fn *func, void *ptr)
{
	int had_children = 0;
	int ii;
    size_t result = 0;
	result += func(ptr, "[", 1);
	if (flags & FJSON_TO_STRING_PRETTY) result += func(ptr, "\n", 1);
	for(ii=0; ii < fjson_object_array_length(jso); ii++)
	{
		struct fjson_object *val;
		if (had_children)
		{
			result += func(ptr, ",", 1);
			if (flags & FJSON_TO_STRING_PRETTY) result += func(ptr, "\n", 1);
		}
		had_children = 1;
		if (flags & FJSON_TO_STRING_SPACED) result += func(ptr, " ", 1);
		result += indent(level + 1, flags, func, ptr);
		result += write(fjson_object_array_get_idx(jso, ii), level+1, flags, func, ptr);
	}
	if (flags & FJSON_TO_STRING_PRETTY)
	{
		if (had_children) result += func(ptr, "\n", 1);
		result += indent(level, flags, func, ptr);
	}

	if (flags & FJSON_TO_STRING_SPACED) result += func(ptr, " ]", 2);
	else result += func(ptr, "]", 1);
	return result;
}

/* write a json value */

static size_t write(struct fjson_object *jso, int level, int flags, fjson_write_fn *func, void *ptr)
{
    // if object is not set
	if (!jso) return func(ptr, "null", 4);

    // @todo implementation
    return 0;
}

/* extended write function to string */

size_t fjson_object_write_ext(struct fjson_object *jso, int flags, fjson_write_fn *func, void *ptr)
{
    // write the value
	return write(jso, 0, flags, func, ptr);
}

/* more simple write function */

size_t fjson_object_write_ext(struct fjson_object *jso, fjson_write_fn *func, void *ptr)
{
    // write the value
	return write(jso, 0, FJSON_TO_STRING_SPACED, func, ptr);
}

