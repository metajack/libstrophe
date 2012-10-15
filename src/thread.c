/* thread.c
** strophe XMPP client library -- thread abstraction
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
 *  Thread absraction.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "couplet.h"
#include "common.h"
#include "thread.h"

struct _mutex_t {
	const xmpp_ctx_t *ctx;

#ifdef _WIN32
	HANDLE mutex;
#else
	pthread_mutex_t *mutex;
#endif
};

struct _thread_t {
	const xmpp_ctx_t *ctx;

#ifdef _WIN32
# error "win32 is not supported for now"
#else
	pthread_t *thread;
#endif
};

struct _xmpp_sem_t {
	const xmpp_ctx_t *ctx;

#ifdef _WIN32
# error "win32 is not supported for now"
#else
	sem_t *sem;
#endif
};

/* semaphore functions */

xmpp_sem_t *xmpp_sem_create(const xmpp_ctx_t *ctx)
{
	xmpp_sem_t *sem;

	sem = xmpp_alloc(ctx, sizeof(xmpp_sem_t));
	if (!sem)
		return NULL;

#ifdef _WIN32
# error "win32 is not supported for now"
#else
	sem->sem = xmpp_alloc(ctx, sizeof(*sem->sem));
	if (sem->sem && sem_init(sem->sem, 0, 0)) {
		/* semaphore is allocated but not initialized */
		xmpp_free(ctx, sem->sem);
		sem->sem = NULL;
	}
#endif

	if (!sem->sem) {
		xmpp_free(ctx, sem);
		sem = NULL;
	} else
		sem->ctx = ctx;

	return sem;
}

void xmpp_sem_wait(xmpp_sem_t *sem)
{
#ifdef _WIN32
# error "win32 is not supported for now"
#else
	sem_wait(sem->sem);
#endif
}

void xmpp_sem_post(xmpp_sem_t *sem)
{
#ifdef _WIN32
# error "win32 is not supported for now"
#else
	sem_post(sem->sem);
#endif
}

int xmpp_sem_destroy(xmpp_sem_t *sem)
{
	const xmpp_ctx_t *ctx = NULL;
	int ret;

	if (sem)
		ctx = sem->ctx;
	if (!ctx)
		return 0;

#ifdef _WIN32
# error "win32 is not supported for now"
#else
	if (sem->sem) {
		ret = sem_destoy(sem->sem) == 0;
		xmpp_free(ctx, sem->sem);
	}
#endif
	xmpp_free(ctx, sem);

	return ret;
}

/* thread functions */

thread_t *thread_create(const xmpp_ctx_t *ctx, thread_func_t start_func, void *arg)
{
	thread_t *thread;

	if (!ctx)
		return NULL;

	thread = xmpp_alloc(ctx, sizeof(thread_t));
	if (!thread)
		return NULL;

#ifdef _WIN32
# error "win32 is not supported for now"
#else
	thread->thread = xmpp_alloc(ctx, sizeof(*thread->thread));
	if (thread->thread &&
	    pthread_create(thread->thread, NULL, start_func, arg)) {
		/* thread is allocated but not initialized */
		xmpp_free(ctx, thread->thread);
		thread->thread = NULL;
	}
#endif

	if (!thread->thread) {
		xmpp_free(ctx, thread);
		thread = NULL;
	} else
		thread->ctx = ctx;

	return thread;
}

/* mutex functions */

mutex_t *mutex_create(const xmpp_ctx_t * ctx)
{
	mutex_t *mutex;

	mutex = xmpp_alloc(ctx, sizeof(mutex_t));
	if (!mutex)
		return NULL;

#ifdef _WIN32
	mutex->mutex = CreateMutex(NULL, FALSE, NULL);
#else
	mutex->mutex = xmpp_alloc(ctx, sizeof(pthread_mutex_t));
	if (mutex->mutex && pthread_mutex_init(mutex->mutex, NULL)) {
		/* mutex is allocated but not initialized */
		xmpp_free(ctx, mutex->mutex);
		mutex->mutex = NULL;
	}
#endif

	if (!mutex->mutex) {
		xmpp_free(ctx, mutex);
		mutex = NULL;
	} else
		mutex->ctx = ctx;

	return mutex;
}

int mutex_destroy(mutex_t *mutex)
{
	int ret = 1;
	const xmpp_ctx_t *ctx = NULL;

	if (mutex)
		ctx = mutex->ctx;
	if (!ctx)
		return XMPP_EMEM;

#ifdef _WIN32
	if (mutex->mutex)
		ret = CloseHandle(mutex->mutex);
#else
	if (mutex->mutex) {
		ret = pthread_mutex_destroy(mutex->mutex) == 0;
		xmpp_free(ctx, mutex->mutex);
	}
#endif
	xmpp_free(ctx, mutex);

	return ret;
}

int mutex_lock(mutex_t *mutex)
{
	int ret;

#ifdef _WIN32
	ret = WaitForSingleObject(mutex->mutex, INFINITE) == 0;
#else
	ret = pthread_mutex_lock(mutex->mutex) == 0;
#endif

	return ret;
}

int mutex_trylock(mutex_t *mutex)
{
	/* TODO */
	return 0;
}

int mutex_unlock(mutex_t *mutex)
{
	int ret;

#ifdef _WIN32
	ret = ReleaseMutex(mutex->mutex);
#else
	ret = pthread_mutex_unlock(mutex->mutex) == 0;
#endif

	return ret;
}
