/* event.c
** strophe XMPP client library -- event loop and management
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
 *  Event loop and management.
 */

/** @defgroup EventLoop Event loop
 *  These functions manage the Strophe event loop.  
 *  
 *  Simple tools can use xmpp_run() and xmpp_stop() to manage the life
 *  cycle of the program.  A common idiom is to set up a few initial
 *  event handers, call xmpp_run(), and then respond and react to
 *  events as they come in.  At some point, one of the handlers will
 *  call xmpp_stop() to quit the event loop which leads to the program
 *  terminating.
 * 
 *  More complex programs will have their own event loops, and should
 *  ensure that xmpp_run_once() is called regularly from there.  For
 *  example, a GUI program will already include an event loop to
 *  process UI events from users, and xmpp_run_once() would be called
 *  from an idle function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/select.h>
#include <errno.h>
#else
#include <winsock2.h>
#define ETIMEDOUT WSAETIMEDOUT
#define ECONNRESET WSAECONNRESET
#define ECONNABORTED WSAECONNABORTED
#endif

#include "couplet.h"
#include "common.h"
#include "parser.h"
#include "thread.h"
#include "list.h"

#ifndef DEFAULT_TIMEOUT
/** @def DEFAULT_TIMEOUT
 *  The default timeout in milliseconds for the event loop.
 *  This is set to 1 millisecond.
 */
#define DEFAULT_TIMEOUT 1
#endif

/** Run send loop once.
 *  This function will run send any data that has been queued by xmpp_send
 *  and related functions.
 *
 *  @param ctx a Strophe context object
 *
 *  @ingroup EventLoop
 */
void xmpp_run_send_queue_once(xmpp_ctx_t *ctx)
{
	list_t *connitem;
	list_t *sqitem;
	xmpp_conn_t *conn;
	xmpp_send_queue_t *sq;
	int sent;
	int ret;
	int towrite;

	/* send queued data */
	connitem = list_get_first(ctx->connlist);
	while (connitem) {
		conn = (xmpp_conn_t *)connitem->data;
		if (conn->state != XMPP_STATE_CONNECTED)
			goto loop_next;

		/* if we're running tls, there may be some remaining data waiting to
		 * be sent, so push that out */
		if (conn->tls) {
			ret = tls_clear_pending_write(conn->tls);

			if (ret < 0 && !tls_is_recoverable(tls_error(conn->tls))) {
				/* an error occured */
				xmpp_debug(ctx, "xmpp", "Send error occured, disconnecting.");
				conn->error = ECONNABORTED;
				conn_disconnect(conn);
			}
		}

		/* write all data from the send queue to the socket */
		sqitem = list_shift(conn->send_queue);
		while (sqitem) {
			sent = 1;
			sq = (xmpp_send_queue_t *)sqitem->data;
			towrite = sq->len - sq->written;

			if (conn->tls) {
				ret = tls_write(conn->tls, &sq->data[sq->written], towrite);

				if (ret < 0 && !tls_is_recoverable(tls_error(conn->tls))) {
					/* an error occured */
					conn->error = tls_error(conn->tls);
					sent = 0;
				} else if (ret < towrite) {
					/* not all data could be sent now */
					if (ret >= 0)
						sq->written += ret;
					sent = 0;
				}

			} else {
				ret = sock_write(conn->sock, &sq->data[sq->written], towrite);

				if (ret < 0 && !sock_is_recoverable(sock_error())) {
					/* an error occured */
					conn->error = sock_error();
					sent = 0;
				} else if (ret < towrite) {
					/* not all data could be sent now */
					if (ret >= 0)
						sq->written += ret;
					sent = 0;
				}
			}

			if (!sent) {
				/* insert sq to the head of send queue */
				list_insert(conn->send_queue, sqitem);
				break;
			}

			/* all data for this queue item written, delete and move on */
			xmpp_free(ctx, sq->data);
			xmpp_free(ctx, sq);
			xmpp_free(ctx, sqitem);
			sqitem = list_shift(conn->send_queue);
		}

		/* tear down connection on error */
		if (conn->error) {
			/* FIXME: need to tear down send queues and random other things
			 * maybe this should be abstracted */
			xmpp_debug(ctx, "xmpp", "Send error occured, disconnecting.");
			conn->error = ECONNABORTED;
			conn_disconnect(conn);
		}

loop_next:
		connitem = list_get_next(ctx->connlist, connitem);
	}
}

/** Start the sending loop.
 *  This function is start routine for a separated thread. It continuously
 *  calls xmpp_run_send_queue_once.
 *
 *  @param data is a pointer to a Strophe context object
 *
 *  @return always NULL
 *
 *  @ingroup EventLoop
 */
void *xmpp_send_queue_thread(void *data)
{
	xmpp_ctx_t *ctx = (xmpp_ctx_t *)data;

	if (!ctx)
		return NULL;

	/** @TODO make condition for break */
	/* infinite loop */
	while (1) {
		/* lock thread if nothing to be sent */
		xmpp_sem_wait(ctx->send_queue_sem);
		xmpp_run_send_queue_once(ctx);
	}

	return NULL;
}

/** Run the event loop once.
 *  This function will run through the Strophe event
 *  loop a single time, and will not wait more than timeout
 *  milliseconds for events.  This is provided to support integration
 *  with event loops outside the library, and if used, should be
 *  called regularly to achieve low latency event handling.
 *
 *  @param ctx a Strophe context object
 *  @param timeout time to wait for events in milliseconds
 *
 *  @ingroup EventLoop
 */
void xmpp_run_once(xmpp_ctx_t *ctx, const unsigned long timeout)
{
	list_t *connitem;
	xmpp_conn_t *conn;
	fd_set rfds, wfds;
	sock_t max = 0;
	int ret;
	struct timeval tv;
	char buf[4096];
	uint64_t next;
	long usec;
	int tls_read_bytes = 0;

	if (ctx->loop_status == XMPP_LOOP_QUIT)
		return;
	ctx->loop_status = XMPP_LOOP_RUNNING;

	/* reset parsers if needed */
	connitem = list_get_first(ctx->connlist);
	while (connitem) {
		conn = (xmpp_conn_t *)connitem->data;
		if (conn->reset_parser)
			conn_parser_reset(conn);
		connitem = list_get_next(ctx->connlist, connitem);
	}

	/* fire any ready timed handlers, then
	 * make sure we don't wait past the time when timed handlers need 
	 * to be called */
	next = handler_fire_timed(ctx);

	usec = ((next < timeout) ? next : timeout) * 1000;
	tv.tv_sec = usec / 1000000;
	tv.tv_usec = usec % 1000000;

	FD_ZERO(&rfds); 
	FD_ZERO(&wfds);

	/* find events to watch */
	connitem = list_get_first(ctx->connlist);
	while (connitem) {
		conn = (xmpp_conn_t *)connitem->data;
	
		switch (conn->state) {
		case XMPP_STATE_CONNECTING:
			/* connect has been called and we're waiting for it to complete */
			/* connection will give us write or error events */
	    
			/* make sure the timeout hasn't expired */
			if (time_elapsed(conn->timeout_stamp, time_stamp()) <=
							conn->connect_timeout)
				FD_SET(conn->sock, &wfds);
			else {
				conn->error = ETIMEDOUT;
				xmpp_info(ctx, "xmpp", "Connection attempt timed out.");
				conn_disconnect(conn);
			}
			break;
		case XMPP_STATE_CONNECTED:
			FD_SET(conn->sock, &rfds);
			break;
		case XMPP_STATE_DISCONNECTED:
			/* do nothing */
		default:
			break;
		}
	
		/* Check if there is something in the SSL buffer. */
		if (conn->tls)
			tls_read_bytes += tls_pending(conn->tls);
	
		if (conn->sock > max)
			max = conn->sock;

		connitem = list_get_next(ctx->connlist, connitem);
	}

	/* check for events */
	ret = select(max + 1, &rfds,  &wfds, NULL, &tv);

	/* select errored */
	if (ret < 0) {
		if (!sock_is_recoverable(sock_error()))
			xmpp_error(ctx, "xmpp", "event watcher internal error %d", 
				   sock_error());
		return;
	}
    
	/* no events happened */
	if (ret == 0 && tls_read_bytes == 0)
		return;

	/* process events */
	connitem = list_get_first(ctx->connlist);
	while (connitem) {
		conn = (xmpp_conn_t *)connitem->data;

		switch (conn->state) {
		case XMPP_STATE_CONNECTING:
			if (FD_ISSET(conn->sock, &wfds)) {
				/* connection complete */

				/* check for error */
				if (sock_connect_error(conn->sock) != 0) {
					/* connection failed */
					xmpp_debug(ctx, "xmpp", "connection failed");
					conn_disconnect(conn);
					break;
				}

				conn->state = XMPP_STATE_CONNECTED;
				xmpp_debug(ctx, "xmpp", "connection successful");

				/* send stream init */
				conn_open_stream(conn);
			}

			break;
		case XMPP_STATE_CONNECTED:
			if (FD_ISSET(conn->sock, &rfds) || (conn->tls && tls_pending(conn->tls))) {
				if (conn->tls)
					ret = tls_read(conn->tls, buf, 4096);
				else
					ret = sock_read(conn->sock, buf, 4096);

				if (ret > 0) {
					ret = parser_feed(conn->parser, buf, ret);
					if (!ret) {
						/* parse error, we need to shut down */
						/* FIXME */
						xmpp_debug(ctx, "xmpp", "parse error, disconnecting");
						conn_disconnect(conn);
					}
				} else if (conn->tls) {
					if (!tls_is_recoverable(tls_error(conn->tls))) {
						xmpp_debug(ctx, "xmpp", "Unrecoverable TLS error, %d.", tls_error(conn->tls));
						conn->error = tls_error(conn->tls);
						conn_disconnect(conn);
					}
				} else {
					/* return of 0 means socket closed by server */
					xmpp_debug(ctx, "xmpp", "Socket closed by remote host.");
					conn->error = ECONNRESET;
					conn_disconnect(conn);
				}
			}

			break;
		case XMPP_STATE_DISCONNECTED:
			/* do nothing */
		default:
			break;
		}

		connitem = list_get_next(ctx->connlist, connitem);
	}

	/* fire any ready handlers */
	handler_fire_timed(ctx);
}

/** Start the event loop.
 *  This function starts sending loop thread and continuously calls
 *  xmpp_run_once. Loop does not stop until xmpp_stop has been called.
 *
 *  @param ctx a Strophe context object
 *
 *  @ingroup EventLoop
 */
void xmpp_run(xmpp_ctx_t *ctx)
{
	thread_t *thread;

	if (ctx->loop_status != XMPP_LOOP_NOTSTARTED)
		return;

	thread = thread_create(ctx, &xmpp_send_queue_thread, ctx);
	/** @TODO check return value */

	ctx->loop_status = XMPP_LOOP_RUNNING;
	while (ctx->loop_status == XMPP_LOOP_RUNNING)
		xmpp_run_once(ctx, DEFAULT_TIMEOUT);

	/** @TODO stop sending thread */

	xmpp_debug(ctx, "event", "Event loop completed.");
}

/** Stop the event loop.
 *  This will stop the event loop after the current iteration and cause
 *  xmpp_run to exit.
 *
 *  @param ctx a Strophe context object
 *
 *  @ingroup EventLoop
 */
void xmpp_stop(xmpp_ctx_t *ctx)
{
	xmpp_debug(ctx, "event", "Stopping event loop.");

	if (ctx->loop_status == XMPP_LOOP_RUNNING)
		ctx->loop_status = XMPP_LOOP_QUIT;
}
