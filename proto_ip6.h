/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PROTO_IP6_H
#define _PROTO_IP6_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NET_HAS_GET_L3_CKSUM  1

#define NET_IP6_NH_UDP    17
#define NET_IP6_NH_ICMPV6 58
#define NET_IP6_NH_NONXT  59

#define NET_IP6_L2_MCSUFFIXES(ip6_addr) \
{ \
	{0x00, 0x00, 0x00, 0x01}, \
	{0xff, (ip6_addr)[13], (ip6_addr)[14], (ip6_addr)[15]} \
}
#define NET_IP6_L2_MCSUFFIX_CNT 2

struct net_ip6_ctx {
	uint8_t src_addr[16];
	uint8_t dst_addr[16];
	uint8_t nh;

	struct NET_IP6_PROTO_LOWER(_ctx) *lower;
};

extern int8_t net_ip6_set_source_addr(struct net_ip6_ctx *ip6, uint8_t *src_addr);
extern int8_t net_ip6_set_destination_addr(struct net_ip6_ctx *ip6, uint8_t *dst_addr);
extern int8_t net_ip6_set_nexthdr(struct net_ip6_ctx *ip6, uint8_t nh);
extern uint16_t net_ip6_get_l3_cksum(struct net_ip6_ctx *ip6);

extern int8_t net_ip6_connect(struct net_ip6_ctx *ip6);
extern uint8_t net_ip6_pload_pos(struct net_ip6_ctx *ip6);
extern int8_t net_ip6_recv(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen,
                           uint16_t *dataoffset, uint16_t *datalen);
extern int8_t net_ip6_send(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen,
                           uint16_t dataoffset, uint16_t datalen);

#ifdef __cplusplus
}
#endif

#endif
