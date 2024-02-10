/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "proto_mac.h"
#include "net_utils.h"


#define NET_MAC_RECV_LOWER(...)       NET_MAC_PROTO_LOWER(_recv)(__VA_ARGS__)
#define NET_MAC_SEND_LOWER(...)       NET_MAC_PROTO_LOWER(_send)(__VA_ARGS__)

#define NET_MAC_CMP_ADDR(theirs, ours) \
	(((theirs)[0] == (ours)[0]) && \
	 ((theirs)[1] == (ours)[1]) && \
	 ((theirs)[2] == (ours)[2]) && \
	 ((theirs)[3] == (ours)[3]) && \
	 ((theirs)[4] == (ours)[4]) && \
	 ((theirs)[5] == (ours)[5]))

#define NET_MAC_IS_IP6MCAST(l2addr) \
	(((l2addr)[0] == 0x33) && ((l2addr)[1] == 0x33))

#define NET_MAC_CMP_IP6MCAST(l2addr, suffix) \
	(((l2addr)[2] == (suffix)[0]) && \
	 ((l2addr)[3] == (suffix)[1]) && \
 	 ((l2addr)[4] == (suffix)[2]) && \
 	 ((l2addr)[5] == (suffix)[3]))

#define NET_MAC_HDRSIZE 14

uint8_t *net_mac_get_l2_addr(struct net_mac_ctx *mac)
{
	return mac->src_l2addr;
}

int8_t net_mac_set_source_addr(struct net_mac_ctx *mac, uint8_t *src_l2addr)
{
	memcpy(mac->src_l2addr, src_l2addr, 6);
	return NET_STATUS_OK;
}

int8_t net_mac_set_destination_addr(struct net_mac_ctx *mac, uint8_t *dst_l2addr)
{
	memcpy(mac->dst_l2addr, dst_l2addr, 6);
	return NET_STATUS_OK;
}

int8_t net_mac_set_ethertype(struct net_mac_ctx *mac, uint16_t ethertype)
{
	mac->ethertype[0] = (uint8_t) ((ethertype & 0xFF00) >> 8);
	mac->ethertype[1] = (uint8_t) (ethertype & 0x00FF);
	return NET_STATUS_OK;
}

int8_t net_mac_set_ip6mcast(struct net_mac_ctx *mac,
                            uint8_t suffix_cnt, net_mac_mcsuffix_t *suffix)
{
	mac->ip6mcast_suffix_cnt = suffix_cnt;
	mac->ip6mcast_suffix = suffix;

	return NET_STATUS_OK;
}

uint8_t net_mac_pload_pos(struct net_mac_ctx *mac)
{
	return NET_MAC_HDRSIZE;
}

int8_t net_mac_recv(struct net_mac_ctx *mac, uint8_t *buffer, uint16_t buflen,
                           uint16_t *dataoffset, uint16_t *datalen)
{
	uint16_t frame_length = 0;
	int8_t errno = NET_EAGAIN;

	/**
	 * Note: The prototype of hw_w5500_recv is different from other *_recv
	 * functions. Will be fixed when the buffer structure will be changed.
	 */
	frame_length = NET_MAC_RECV_LOWER(mac->lower, buffer, buflen);
	if (frame_length < NET_MAC_HDRSIZE) {
		errno = NET_EAGAIN;
		goto out_zerodata;
	} else if ((buffer[12] != mac->ethertype[0]) ||
	           (buffer[13] != mac->ethertype[1])) {
		errno = NET_EAGAIN;
		goto out_zerodata;
	}

	if (NET_MAC_CMP_ADDR(&(buffer[0]), mac->src_l2addr)) {
		errno = NET_STATUS_OK;
		goto out_data;
	} else if (NET_MAC_IS_IP6MCAST(&(buffer[0]))) {
		for (int i=0; i<mac->ip6mcast_suffix_cnt; i++) {
			if (NET_MAC_CMP_IP6MCAST(&(buffer[0]),
			                         mac->ip6mcast_suffix[i])) {
				errno = NET_STATUS_OK;
				goto out_data;
			}
		}
	}

out_zerodata:
	*datalen = 0;
	*dataoffset = 0;
	return errno;

out_data:
	*dataoffset = NET_MAC_HDRSIZE;
	*datalen = frame_length - NET_MAC_HDRSIZE;
	return errno;
}

int8_t net_mac_send(struct net_mac_ctx *mac, uint8_t *buffer, uint16_t buflen,
                           uint16_t dataoffset, uint16_t datalen)
{
	uint8_t *cursor = NULL;
	uint16_t sent = 0;

	/* Set the cursor to the position of the mac header in the buffer */
	NET_SET_CURSOR(buffer, 0);

	/* Check that buffer is big enough for the MAC header size */
	if (!NET_CHECK_BUFLEN(buffer, dataoffset, NET_MAC_HDRSIZE)) {
		return NET_EOVERFLOW;
	}

	/* Put source and destination mac addresses */
	NET_PUT_DATA(mac->dst_l2addr, 6);
	NET_PUT_DATA(mac->src_l2addr, 6);

	/* Put the ethertype */
	NET_PUT_DATA(mac->ethertype, 2);

	datalen += NET_MAC_HDRSIZE;

	/**
	 * Note: The prototype of hw_w5500_send is different from other *_send
	 * functions. Will be fixed when the buffer structure will be changed.
	 */
	sent = NET_MAC_SEND_LOWER(mac->lower, buffer, datalen);
	if (sent == datalen) {
		return NET_STATUS_OK;
	} else {
		return NET_EAGAIN;
	}
}
