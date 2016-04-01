/*
 * $Id: json.h,v 1.6 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _fj_json_h_
#define _fj_json_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"
#include "linkhash.h"
#include "arraylist.h"
#include "json_util.h"
#include "json_object.h"
#include "json_tokener.h"
#include "json_object_iterator.h"
#include "json_c_version.h"

/**
 * Set initial size allocation for memory when creating strings,
 * as is done for example in fjson_object_to_json_string(). The
 * default size is 32, which is very conservative. If an app
 * knows it typically deals with larger strings, performance
 * can be improved by setting the initial size to a different
 * number, e.g. 1k. Note that this also means that memory
 * consumption can increase. How far entriely depens on the
 * application and its use of json-c.
 *
 * Note: each time this function is called, the initial size is
 * changed to the given value. Already existing elements are not
 * affected. This function is usually meant to be called just once
 * at start of an application, but there is no harm calling it more
 * than once. Note that the function is NOT thread-safe and must not
 * be called on different threads concurrently.
 *
 * @param size new initial size for printbuf (formatting buffer)
 */
extern void
fjson_global_set_printbuf_initial_size(int size);
#ifdef __cplusplus
}
#endif

#endif
