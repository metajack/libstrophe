/* thread.h
** strophe XMPP client library -- thread abstraction header
**
** Copyright (C) 2005-2009 Collecta, Inc. 
**
**  This software is provided AS-IS with no warranty, either express
**  or implied.
**
**  This software is distributed under license and may not be copied,
**  modified or distributed except as expressly authorized under the
**  terms of the license contained in the file LICENSE.txt in this
**  distribution.
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
#include <semaphore.h>
#endif

#include "couplet.h"

typedef struct _mutex_t mutex_t;
typedef struct _thread_t thread_t;
typedef struct _xmpp_sem_t xmpp_sem_t;

typedef void *(*thread_func_t) (void *);

/* semaphore functions */

xmpp_sem_t *xmpp_sem_create(const xmpp_ctx_t *ctx);
void xmpp_sem_wait(xmpp_sem_t *sem);
void xmpp_sem_post(xmpp_sem_t *sem);
int xmpp_sem_destroy(xmpp_sem_t *sem);

/* thread functions */

thread_t *thread_create(const xmpp_ctx_t *ctx, thread_func_t start_func, void *arg);

/* mutex functions */

mutex_t *mutex_create(const xmpp_ctx_t *ctx);
int mutex_destroy(mutex_t *mutex);
int mutex_lock(mutex_t *mutex);
int mutex_trylock(mutex_t *mutex);
int mutex_unlock(mutex_t *mutex);

#endif /* __LIBSTROPHE_THREAD_H__ */
