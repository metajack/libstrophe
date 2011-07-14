/* sock.h
** strophe XMPP client library -- socket abstraction header
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
 *  Socket abstraction API.
 */

#ifndef __LIBSTROPHE_SOCK_H__
#define __LIBSTROPHE_SOCK_H__

#include <stdio.h>
#include "../strophe.h"

void sock_initialize(void);
void sock_shutdown(void);

int sock_error(void);

xmpp_sock_t sock_connect(const char * const host, const unsigned int port);
int sock_close(const xmpp_sock_t sock);

int sock_set_blocking(const xmpp_sock_t sock);
int sock_set_nonblocking(const xmpp_sock_t sock);
int sock_read(const xmpp_sock_t sock, void * const buff, const size_t len);
int sock_write(const xmpp_sock_t sock, const void * const buff, const size_t len);
int sock_is_recoverable(const int error);
/* checks for an error after connect, return 0 if connect successful */
int sock_connect_error(const xmpp_sock_t sock);

int sock_srv_lookup(const char *service, const char *proto,
		     const char *domain, char *resulttarget,
		     int resulttargetlength, int *resultport);

#endif /* __LIBSTROPHE_SOCK_H__ */
