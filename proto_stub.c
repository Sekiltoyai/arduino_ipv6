/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "proto_stub.h"
#include "net_utils.h"

#include <stdlib.h>


#ifdef NET_STUB_GET_L2_ADDR_ENABLE
uint8_t *net_stub_get_l2_addr(struct net_stub_ctx *stub)
{
	return stub->l2_addr;
}
#endif

int8_t net_stub_connect(struct net_stub_ctx *stub)
{
	return NET_STATUS_OK;
}

uint8_t net_stub_pload_pos(struct net_stub_ctx *stub)
{
	return 0;
}

int8_t net_stub_recv(struct net_stub_ctx *stub, uint8_t *buffer, uint16_t buflen,
                     uint16_t *dataoffset, uint16_t *datalen)
{
	*dataoffset = 0;
	*datalen = stub->recv_cback(buffer, buflen);
	if (*datalen == 0) {
		return NET_EAGAIN;
	}
	return NET_STATUS_OK;
}

int8_t net_stub_send(struct net_stub_ctx *stub, uint8_t *buffer, uint16_t buflen,
                     uint16_t dataoffset, uint16_t datalen)
{
	if (stub->send_cback(buffer + dataoffset, datalen) != datalen) {
		return NET_ENOMEM;
	}
	return NET_STATUS_OK;
}
