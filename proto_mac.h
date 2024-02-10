/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PROTO_MAC_H
#define _PROTO_MAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NET_HAS_GET_L2_ADDR 1

#define NET_MAC_ETHERTYPE_IPV6 0x86DD
#define NET_MAC_ETHERTYPE_LB   0x9000

typedef uint8_t net_mac_mcsuffix_t[4];

struct net_mac_ctx {
	uint8_t src_l2addr[6];
	uint8_t dst_l2addr[6];
	uint8_t ethertype[2];
	uint8_t ip6mcast_suffix_cnt;
	net_mac_mcsuffix_t *ip6mcast_suffix;

	struct NET_MAC_PROTO_LOWER(_ctx) *lower;
};

extern uint8_t *net_mac_get_l2_addr(struct net_mac_ctx *mac);
extern int8_t net_mac_set_source_addr(struct net_mac_ctx *mac, uint8_t *src_l2addr);
extern int8_t net_mac_set_destination_addr(struct net_mac_ctx *mac, uint8_t *dst_l2addr);
extern int8_t net_mac_set_ethertype(struct net_mac_ctx *mac, uint16_t ethertype);
extern int8_t net_mac_set_ip6mcast(struct net_mac_ctx *mac,
                                   uint8_t suffix_cnt, net_mac_mcsuffix_t *suffix);

extern int8_t net_mac_connect(struct net_mac_ctx *mac);
extern uint8_t net_mac_pload_pos(struct net_mac_ctx *mac);
extern int8_t net_mac_recv(struct net_mac_ctx *mac, uint8_t *buffer, uint16_t buflen,
                           uint16_t *dataoffset, uint16_t *datalen);
extern int8_t net_mac_send(struct net_mac_ctx *mac, uint8_t *buffer, uint16_t buflen,
                           uint16_t dataoffset, uint16_t datalen);


#ifdef __cplusplus
}
#endif

#endif
