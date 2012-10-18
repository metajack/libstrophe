/* list.c
 * libcouplet - fork of libstrophe, XMPP client library
 * General list routines
 *
 * Copyright (C) 2012 Dmitry Podgorny
 *
 * This software is provided AS-IS with no warranty
 *
 * You can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either version
 * 3 of the License, or any later version.
 */

/** @file
 *  General list routines
 */

#include "list.h"

list_head_t *list_init(const xmpp_ctx_t * const ctx)
{
}

void list_destroy(list_head_t *list)
{
}

list_t *list_init_item(const xmpp_ctx_t * const ctx)
{
}

list_t *list_get_first(const list_head_t * const list)
{
}

list_t *list_get_next(const list_head_t * const list, const list_t * const item)
{
}

list_t *list_get_last(const list_head_t * const list)
{
}

list_t *list_pull_first(list_head_t * const list)
{
}

list_t *list_pull_next(list_head_t * const list, list_t * const item)
{
}

list_t *list_pull_last(list_head_t * const list)
{
}

void list_insert(list_head_t * const list, list_t * const item)
{
}

void list_insert_after(list_head_t * const list, list_t * const item, list_t * const after)
{
}

void list_append(list_head_t * const list, list_t * const item)
{
}

void list_update(list_head_t * const list, list_t * const item, const void * const data)
{
}
