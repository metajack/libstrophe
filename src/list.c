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
#include "couplet.h"
#include "common.h"

/** Create new list.
 *
 *  @param ctx a context object
 *
 *  @return a pointer to allocated list_head_t structure or NULL pointer
 */
list_head_t *list_init(xmpp_ctx_t * const ctx)
{
	list_head_t *list;

	if (!ctx)
		return NULL;

	list = (list_head_t *)xmpp_alloc(ctx, sizeof(*list));
	if (list) {
		list->mutex = mutex_create(ctx);
		if (!list->mutex) {
			xmpp_free(ctx, list);
			return NULL;
		}
		list->ctx = ctx;
		list->head = NULL;
		list->tail = NULL;
	}

	return list;
}

void list_destroy(list_head_t *list)
{
}

/** Create new list item
 *
 *  @param ctx a context object
 *
 *  @return a pointer to allocated list_t structure or NULL pointer
 */
list_t *list_init_item(const xmpp_ctx_t * const ctx)
{
	list_t *item;

	if (!ctx)
		return NULL;

	item = (list_t *)xmpp_alloc(ctx, sizeof(*item));
	if (item) {
		item->data = NULL;
		item->next = NULL;
	}

	return item;
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

/** Get the first element of the list and remove it
 *
 *  @param list a list object
 *
 *  @return pointer to the first element or NULL pointer if such doesn't exist
 */
list_t *list_shift(list_head_t * const list)
{
	list_t *item;

	mutex_lock(list->mutex);
	item = list->head;
	if (item) {
		list->head = item->next;
		if (!list->head)
			list->tail = NULL;
	}
	mutex_unlock(list->mutex);

	return item;
}

list_t *list_shift_next(list_head_t * const list, list_t * const item)
{
}

list_t *list_pop(list_head_t * const list)
{
}

/** Insert the element to the head of the list
 *
 *  @param list a list object
 *  @param item an element for insertion
 */
void list_insert(list_head_t * const list, list_t * const item)
{
	if (!item)
		return;

	mutex_lock(list->mutex);
	item->next = list->head;
	list->head = item;
	if (!item->next)
		list->tail = item;
	mutex_unlock(list->mutex);
}

void list_insert_after(list_head_t * const list, list_t * const item, list_t * const after)
{
}

/** Append the element to the end of the list
 *
 *  @param list a list object
 *  @param item an element
 */
void list_push(list_head_t * const list, list_t * const item)
{
	if (!item)
		return;

	mutex_lock(list->mutex);
	if (!list->tail)
		list->head = item;
	else
		list->tail->next = item;
	list->tail = item;
	mutex_unlock(list->mutex);
}

void list_update(list_head_t * const list, list_t * const item, const void * const data)
{
}
