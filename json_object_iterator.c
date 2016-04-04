/**
*******************************************************************************
* @file fjson_object_iterator.c
*
* Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
* Copyright (c) 2016 Adiscon GmbH
* Rainer Gerhards <rgerhards@adiscon.com>
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the MIT license. See COPYING for details.
*
* @brief  json-c forces clients to use its private data
*         structures for JSON Object iteration.  This API
*         implementation corrects that by abstracting the
*         private json-c details.
*
*******************************************************************************
*/

#include <stddef.h>

#include "json.h"
#include "json_object_private.h"

#include "json_object_iterator.h"

/**
 * How It Works
 *
 * For each JSON Object, json-c maintains a linked list of zero
 * or more lh_entry (link-hash entry) structures inside the
 * Object's link-hash table (lh_table).
 *
 * Each lh_entry structure on the JSON Object's linked list
 * represents a single name/value pair.  The "next" field of the
 * last lh_entry in the list is set to NULL, which terminates
 * the list.
 *
 * We represent a valid iterator that refers to an actual
 * name/value pair via a pointer to the pair's lh_entry
 * structure set as the iterator's opaque_ field.
 *
 * We follow json-c's current pair list representation by
 * representing a valid "end" iterator (one that refers past the
 * last pair) with a NULL value in the iterator's opaque_ field.
 *
 * A JSON Object without any pairs in it will have the "head"
 * field of its lh_table structure set to NULL.  For such an
 * object, fjson_object_iter_begin will return an iterator with
 * the opaque_ field set to NULL, which is equivalent to the
 * "end" iterator.
 *
 * When iterating, we simply update the iterator's opaque_ field
 * to point to the next lh_entry structure in the linked list.
 * opaque_ will become NULL once we iterate past the last pair
 * in the list, which makes the iterator equivalent to the "end"
 * iterator.
 */

/// Our current representation of the "end" iterator;
///
/// @note May not always be NULL
static const void* kObjectEndIterValue = NULL;

/**
 * ****************************************************************************
 */
struct fjson_object_iterator
fjson_object_iter_begin(struct fjson_object* obj)
{
    struct fjson_object_iterator iter;
    struct lh_table* pTable;

    /// @note fjson_object_get_object will return NULL if passed NULL
    ///       or a non-fjson_type_object instance
    pTable = fjson_object_get_object(obj);
    JASSERT(NULL != pTable);

    /// @note For a pair-less Object, head is NULL, which matches our
    ///       definition of the "end" iterator
    iter.opaque_ = pTable->head;
    return iter;
}

/**
 * ****************************************************************************
 */
struct fjson_object_iterator
fjson_object_iter_end(const struct fjson_object* __attribute__((unused)) obj)
{
    struct fjson_object_iterator iter;

    JASSERT(NULL != obj);
    JASSERT(fjson_object_is_type(obj, fjson_type_object));

    iter.opaque_ = kObjectEndIterValue;

    return iter;
}

/**
 * ****************************************************************************
 */
void
fjson_object_iter_next(struct fjson_object_iterator* iter)
{
    JASSERT(NULL != iter);
    JASSERT(kObjectEndIterValue != iter->opaque_);

    iter->opaque_ = ((struct lh_entry *)iter->opaque_)->next;
}


/**
 * ****************************************************************************
 */
const char*
fjson_object_iter_peek_name(const struct fjson_object_iterator* iter)
{
    JASSERT(NULL != iter);
    JASSERT(kObjectEndIterValue != iter->opaque_);

    return (const char*)(((struct lh_entry *)iter->opaque_)->k);
}


/**
 * ****************************************************************************
 */
struct fjson_object*
fjson_object_iter_peek_value(const struct fjson_object_iterator* iter)
{
    JASSERT(NULL != iter);
    JASSERT(kObjectEndIterValue != iter->opaque_);

    return (struct fjson_object*)(((struct lh_entry *)iter->opaque_)->v);
}


/**
 * ****************************************************************************
 */
fjson_bool
fjson_object_iter_equal(const struct fjson_object_iterator* iter1,
                       const struct fjson_object_iterator* iter2)
{
    JASSERT(NULL != iter1);
    JASSERT(NULL != iter2);

    return (iter1->opaque_ == iter2->opaque_);
}


/**
 * ****************************************************************************
 */
struct fjson_object_iterator
fjson_object_iter_init_default(void)
{
    struct fjson_object_iterator iter;

    /**
     * @note Make this a negative, invalid value, such that
     *       accidental access to it would likely be trapped by the
     *       hardware as an invalid address.
     */
    iter.opaque_ = NULL;

    return iter;
}
