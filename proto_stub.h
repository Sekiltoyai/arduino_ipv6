/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PROTO_STUB_H
#define _PROTO_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


typedef uint16_t (*net_stub_recv_callback)(uint8_t*, uint16_t);
typedef uint16_t (*net_stub_send_callback)(uint8_t*, uint16_t);

struct net_stub_ctx {
#ifdef NET_STUB_GET_L2_ADDR_ENABLE
	uint8_t *l2_addr;
#endif
	net_stub_recv_callback recv_cback;
	net_stub_send_callback send_cback;
};

#ifdef NET_STUB_GET_L2_ADDR_ENABLE
#define NET_HAS_GET_L2_ADDR 1
extern uint8_t *net_stub_get_l2_addr(struct net_stub_ctx *stub);
#endif
extern int8_t net_stub_connect(struct net_stub_ctx *stub);
extern uint8_t net_stub_pload_pos(struct net_stub_ctx *stub);
extern int8_t net_stub_recv(struct net_stub_ctx *stub, uint8_t *buffer, uint16_t buflen,
                            uint16_t *dataoffset, uint16_t *datalen);
extern int8_t net_stub_send(struct net_stub_ctx *stub, uint8_t *buffer, uint16_t buflen,
                            uint16_t dataoffset, uint16_t datalen);


#ifdef __cplusplus
}
#endif

#endif
