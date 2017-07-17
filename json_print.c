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
#include <string.h>
#include <math.h>

#include "json_object.h"
#include "json_object_private.h"
#include "json_object_iterator.h"


#if !defined(HAVE_SNPRINTF)
# error You do not have snprintf on your system.
#endif /* HAVE_SNPRINTF */


/* Forward declaration of the write function */
static size_t write(struct fjson_object *jso, int level, int flags, fjson_write_fn *func, void *ptr);

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
 *  do the lookups).
 *  rgerhards@adiscon.com, 2015-11-18
 *  using now external char_needsEscape array. -- rgerhards, 2016-11-30
 */
extern const char char_needsEscape[256];

/**
 *  Function to escape a string
 *  @param  str     the string to be escaped
 *  @param  func    user supplied write function
 *  @param  ptr     user supplied pointer
 *  @return size_t  number of bytes written
 */
static size_t escape(const char *str, fjson_write_fn *func, void *ptr)
{
	int size;
	char tempbuf[6];
	size_t result = 0;
	const char *start_offset = str;
	while(1) { /* broken below on 0-byte */
		if(char_needsEscape[*((unsigned char*)str)]) {
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
				size = snprintf(tempbuf, sizeof(tempbuf), "\\u00%c%c", fjson_hex_chars[*str >> 4], fjson_hex_chars[*str & 0xf]);
				result += func(ptr, tempbuf, size);
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

static size_t indent(int level, int flags, fjson_write_fn *func, void *ptr)
{
	// result variable, and loop counter
	size_t result = 0;
	int i;

	// skip if pretty-printing is not needed
	if (!(flags & FJSON_TO_STRING_PRETTY)) return 0;

	// iterate to add the spaces
	for (i = 0; i < level; ++i)
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
	size_t bytes = snprintf(tempbuffer, sizeof(tempbuffer), "%" PRId64, jso->o.c_int64);
	return func(ptr, tempbuffer, bytes);
}

/* write a json floating point */

static size_t write_double(struct fjson_object* jso, int flags, fjson_write_fn *func, void *ptr)
{
	char buf[128], *p, *q;
	int size;
	double dummy;  /* needed for modf() */
	
	if (jso->o.c_double.source) return func(ptr, jso->o.c_double.source, strlen(jso->o.c_double.source));
	
	/* Although JSON RFC does not support
	 * NaN or Infinity as numeric values
	 * ECMA 262 section 9.8.1 defines
	 * how to handle these cases as strings
	 */
	if(isnan(jso->o.c_double.value))
		size = snprintf(buf, sizeof(buf), "NaN");
	else if(isinf(jso->o.c_double.value))
		if(jso->o.c_double.value > 0)
			size = snprintf(buf, sizeof(buf), "Infinity");
		else
			size = snprintf(buf, sizeof(buf), "-Infinity");
	else
		size = snprintf(buf, sizeof(buf),
			(modf(jso->o.c_double.value, &dummy)==0)?"%.17g.0":"%.17g",
			jso->o.c_double.value);

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

	// check type
	switch(jso->o_type) {
	case fjson_type_null:       return func(ptr, "null", 4);
	case fjson_type_boolean:    return write_boolean(jso, func, ptr);
	case fjson_type_double:     return write_double(jso, flags, func, ptr);
	case fjson_type_int:        return write_int(jso, func, ptr);
	case fjson_type_object:     return write_object(jso, level, flags, func, ptr);
	case fjson_type_array:      return write_array(jso, level, flags, func, ptr);
	case fjson_type_string:     return write_string(jso, func, ptr);
	default:                    return 0;
	}
}

/* wrapper around fwrite() that has the same signature as fjson_write_fn */

static size_t fwrite_wrapper(void *ptr, const char *buffer, size_t size)
{
	return fwrite(buffer, 1, size, ptr);
}

/* extended dump function to string */

size_t fjson_object_dump_ext(struct fjson_object *jso, int flags, fjson_write_fn *func, void *ptr)
{
	// write the value
	return write(jso, 0, flags, func, ptr);
}

/* more simple write function */

size_t fjson_object_dump(struct fjson_object *jso, fjson_write_fn *func, void *ptr)
{
	// write the value
	return write(jso, 0, FJSON_TO_STRING_SPACED, func, ptr);
}

/* write to a file* */

size_t fjson_object_write(struct fjson_object *obj, FILE *fp)
{
	return fjson_object_dump_ext(obj, FJSON_TO_STRING_SPACED, fwrite_wrapper, fp);
}

/* write to a file with custom output flags */

size_t fjson_object_write_ext(struct fjson_object *obj, int flags, FILE *fp)
{
	return fjson_object_dump_ext(obj, flags, fwrite_wrapper, fp);
}

