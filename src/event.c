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

#include <strophe.h>
#include "common.h"
#include "parser.h"

#ifndef DEFAULT_TIMEOUT
/** @def DEFAULT_TIMEOUT
 *  The default timeout in milliseconds for the event loop.
 *  This is set to 1 millisecond.
 */
#define DEFAULT_TIMEOUT 1
#endif

/** Run the event loop once.
 *  This function will run send any data that has been queued by
 *  xmpp_send and related functions and run through the Strophe even
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
    xmpp_connlist_t *connitem;
    xmpp_conn_t *conn;
    fd_set rfds, wfds;
    xmpp_sock_t max = 0;
    int ret;
    struct timeval tv;
    xmpp_send_queue_t *sq, *tsq;
    int towrite;
    char buf[4096];
    uint64_t next;
    long usec;

    if (ctx->loop_status == XMPP_LOOP_QUIT) return;
    ctx->loop_status = XMPP_LOOP_RUNNING;


    /* fire any ready timed handlers, then
       make sure we don't wait past the time when timed handlers need 
       to be called */
    next = handler_fire_timed(ctx);

    usec = ((next < timeout) ? next : timeout) * 1000;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;

    FD_ZERO(&rfds); 
    FD_ZERO(&wfds);

    /* find events to watch */
    connitem = ctx->connlist;
    while (connitem) {
	conn = connitem->conn;
	
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
            if (conn->send_queue_head) {
                FD_SET(conn->sock, &wfds);
            }
	    break;
	case XMPP_STATE_DISCONNECTED:
	    /* do nothing */
	default:
	    break;
	}
	
	if (conn->sock > max) max = conn->sock;

	connitem = connitem->next;
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
    if (ret == 0) return;
    
    /* process events */
    connitem = ctx->connlist;
    while (connitem) {
	conn = connitem->conn;

        if (FD_ISSET(conn->sock, &wfds)) {
            xmpp_conn_write(conn);
        }

        if (FD_ISSET(conn->sock, &rfds)) {
            xmpp_conn_read(conn);
        }

	connitem = connitem->next;
    }

    /* fire any ready handlers */
    handler_fire_timed(ctx);
}

/** Start the event loop.
 *  This function continuously calls xmpp_run_once and does not return
 *  until xmpp_stop has been called.
 *
 *  @param ctx a Strophe context object
 *
 *  @ingroup EventLoop
 */
void xmpp_run(xmpp_ctx_t *ctx)
{
    if (ctx->loop_status != XMPP_LOOP_NOTSTARTED) return;

    ctx->loop_status = XMPP_LOOP_RUNNING;
    while (ctx->loop_status == XMPP_LOOP_RUNNING) {
	xmpp_run_once(ctx, DEFAULT_TIMEOUT);
    }

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
