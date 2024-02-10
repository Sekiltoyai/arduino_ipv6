/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PROTO_UDP_H
#define _PROTO_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct net_udp_ctx {
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t cksum_pre_compute;

	struct NET_UDP_PROTO_LOWER(_ctx) *lower;
};


extern int8_t net_udp_set_source_port(struct net_udp_ctx *udp, uint16_t source_port);
extern int8_t net_udp_set_destination_port(struct net_udp_ctx *udp, uint16_t destination_port);

extern int8_t net_udp_connect(struct net_udp_ctx *udp);
extern uint8_t net_udp_pload_pos(struct net_udp_ctx *udp);
extern int8_t net_udp_recv(struct net_udp_ctx *udp, uint8_t *buffer, uint16_t buflen,
                           uint16_t *dataoffset, uint16_t *datalen);
extern int8_t net_udp_send(struct net_udp_ctx *udp, uint8_t *buffer, uint16_t buflen,
                           uint16_t dataoffset, uint16_t datalen);


#ifdef __cplusplus
}
#endif

#endif
