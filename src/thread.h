/* thread.h
** strophe XMPP client library -- thread abstraction header
**
** Copyright (C) 2005-2009 Collecta, Inc. 
**
**  This software is provided AS-IS with no warranty, either express
**  or implied.
**
**  This program is dual licensed under the MIT and GPLv3 licenses.
*/

/** @file
 *  Threading abstraction API.
 */

#ifndef __LIBSTROPHE_THREAD_H__
#define __LIBSTROPHE_THREAD_H__

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "strophe.h"

typedef struct _mutex_t mutex_t;

/* mutex functions */

mutex_t *mutex_create(const xmpp_ctx_t *ctx);
int mutex_destroy(mutex_t *mutex);
int mutex_lock(mutex_t *mutex);
int mutex_trylock(mutex_t *mutex);
int mutex_unlock(mutex_t *mutex);

#endif /* __LIBSTROPHE_THREAD_H__ */
