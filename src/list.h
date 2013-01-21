/* list.h
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
 *  General list routines header
 */

#ifndef __LIBCOUPLET_LIST_H__
#define __LIBCOUPLET_LIST_H__

#include "couplet.h"
#include "thread.h"

struct _list_t {
	void *data;
	struct _list_t *next;
};

typedef struct _list_t list_t;

struct _list_head_t {
	xmpp_ctx_t *ctx;
	mutex_t *mutex;
	list_t *head;
	list_t *tail;
};

typedef struct _list_head_t list_head_t;

list_head_t *list_init(xmpp_ctx_t * const ctx);
void list_destroy(list_head_t *list);
list_t *list_init_item(const xmpp_ctx_t * const ctx);
list_t *list_get_first(const list_head_t * const list);
list_t *list_get_next(const list_head_t * const list, const list_t * const item);
list_t *list_get_last(const list_head_t * const list);
list_t *list_shift(list_head_t * const list);
list_t *list_pop_by_data(list_head_t * const list, const void * const data);
list_t *list_pop_next(list_head_t * const list, list_t * const item);
void list_insert(list_head_t * const list, list_t * const item);
void list_push(list_head_t * const list, list_t * const item);

static inline void list_lock_mutex(const list_head_t * const list)
{
	mutex_lock(list->mutex);
}

static inline void list_unlock_mutex(const list_head_t * const list)
{
	mutex_unlock(list->mutex);
}

#endif
