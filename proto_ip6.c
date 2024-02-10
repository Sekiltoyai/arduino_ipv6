/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "proto_ip6.h"
#include "net_utils.h"

#include <stdlib.h>
#include <stdbool.h>


#define NET_IP6_GET_L2_ADDR_LOWER(...) NET_IP6_PROTO_LOWER(_get_l2_addr)(__VA_ARGS__)
#define NET_IP6_CONNECT_LOWER(...)    NET_IP6_PROTO_LOWER(_connect)(__VA_ARGS__)
#define NET_IP6_PLOAD_POS_LOWER(...)  NET_IP6_PROTO_LOWER(_pload_pos)(__VA_ARGS__)
#define NET_IP6_RECV_LOWER(...)       NET_IP6_PROTO_LOWER(_recv)(__VA_ARGS__)
#define NET_IP6_SEND_LOWER(...)       NET_IP6_PROTO_LOWER(_send)(__VA_ARGS__)

#define NET_IP6_PUT_HEADER_COMMON(len, nh, hl) \
	do { \
		NET_PUT_BYTE(NET_IP6_VERSION << 4); \
		NET_PUT_BYTE(0x00); \
		NET_PUT_SHORT(0x0000); \
		NET_PUT_SHORT(len); \
		NET_PUT_BYTE(nh); \
		NET_PUT_BYTE(hl); \
	} while(0)


#define NET_IP6_CMP_ADDR(theirs, ours) \
	(((theirs)[0] == (ours)[0]) && \
	 ((theirs)[1] == (ours)[1]) && \
	 ((theirs)[2] == (ours)[2]) && \
	 ((theirs)[3] == (ours)[3]) && \
	 ((theirs)[4] == (ours)[4]) && \
	 ((theirs)[5] == (ours)[5]) && \
	 ((theirs)[6] == (ours)[6]) && \
	 ((theirs)[7] == (ours)[7]) && \
	 ((theirs)[8] == (ours)[8]) && \
	 ((theirs)[9] == (ours)[9]) && \
	 ((theirs)[10] == (ours)[10]) && \
	 ((theirs)[11] == (ours)[11]) && \
	 ((theirs)[12] == (ours)[12]) && \
	 ((theirs)[13] == (ours)[13]) && \
	 ((theirs)[14] == (ours)[14]) && \
	 ((theirs)[15] == (ours)[15]))

#define NET_IP6_CMP_LLADDR(theirs, ours) \
	(((theirs)[0] == 0xFE) && \
	 ((theirs)[1] == 0x80) && \
	 ((theirs)[2] == 0x00) && \
	 ((theirs)[3] == 0x00) && \
	 ((theirs)[4] == 0x00) && \
	 ((theirs)[5] == 0x00) && \
	 ((theirs)[6] == 0x00) && \
	 ((theirs)[7] == 0x00) && \
	 ((theirs)[8] == (ours)[8]) && \
	 ((theirs)[9] == (ours)[9]) && \
	 ((theirs)[10] == (ours)[10]) && \
	 ((theirs)[11] == (ours)[11]) && \
	 ((theirs)[12] == (ours)[12]) && \
	 ((theirs)[13] == (ours)[13]) && \
	 ((theirs)[14] == (ours)[14]) && \
	 ((theirs)[15] == (ours)[15]))

#define NET_IP6_CMP_UNSPEC(theirs) \
	(((theirs)[0] == 0x00) && \
	 ((theirs)[1] == 0x00) && \
	 ((theirs)[2] == 0x00) && \
	 ((theirs)[3] == 0x00) && \
	 ((theirs)[4] == 0x00) && \
	 ((theirs)[5] == 0x00) && \
	 ((theirs)[6] == 0x00) && \
	 ((theirs)[7] == 0x00) && \
	 ((theirs)[8] == 0x00) && \
	 ((theirs)[9] == 0x00) && \
	 ((theirs)[10] == 0x00) && \
	 ((theirs)[11] == 0x00) && \
	 ((theirs)[12] == 0x00) && \
	 ((theirs)[13] == 0x00) && \
	 ((theirs)[14] == 0x00) && \
	 ((theirs)[15] == 0x00))

/**
 *
 * IPv6 header
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Version| Traffic Class |           Flow Label                  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         Payload Length        |  Next Header  |   Hop Limit   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * +                                                               +
 * |                                                               |
 * +                         Source Address                        +
 * |                                                               |
 * +                                                               +
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * +                                                               +
 * |                                                               |
 * +                      Destination Address                      +
 * |                                                               |
 * +                                                               +
 * |                                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define NET_IP6_HDRSIZE        40
#define NET_ICMPV6_HDRSIZE     4
#define NET_ICMPV6_NS_HDRSIZE  20
#define NET_ICMPV6_NA_HDRSIZE  20
#define NET_ICMPV6_NDP_OPT_LLA_HDRSIZE 8

#define NET_IP6_VERSION 0x06
#define NET_IP6_HOPLIMIT 255

#define NET_ICMPV6_TYPE_RA 133
#define NET_ICMPV6_TYPE_NS 135
#define NET_ICMPV6_TYPE_NA 136
#define NET_ICMPV6_NDP_OPT_TGTLLADDR 2


static int8_t _net_ip6_process_icmpv6(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen,
                                      uint16_t *dataoffset, uint16_t *datalen,
                                      uint8_t *src_addr, uint8_t *dst_addr);
static int8_t _net_icmpv6_send_na(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen, uint16_t dataoffset,
                                  uint8_t *dst_addr, uint8_t *tgt_addr, bool solicited);


int8_t net_ip6_set_source_addr(struct net_ip6_ctx *ip6, uint8_t *src_addr)
{
	memcpy(ip6->src_addr, src_addr, 16);
	return NET_STATUS_OK;
}

int8_t net_ip6_set_destination_addr(struct net_ip6_ctx *ip6, uint8_t *dst_addr)
{
	memcpy(ip6->dst_addr, dst_addr, 16);
	return NET_STATUS_OK;
}

int8_t net_ip6_set_nexthdr(struct net_ip6_ctx *ip6, uint8_t nh)
{
	ip6->nh = nh;
	return NET_STATUS_OK;
}

uint16_t net_ip6_get_l3_cksum(struct net_ip6_ctx *ip6)
{
	uint16_t sum = 0;
	uint8_t nh[] = {0x00, ip6->nh};

	sum = _net_cksum_sum(sum, ip6->src_addr, 16);
	sum = _net_cksum_sum(sum, ip6->dst_addr, 16);
	sum = _net_cksum_sum(sum, nh, 2);

	return sum;
}


int8_t net_ip6_connect(struct net_ip6_ctx *ip6)
{
	return NET_STATUS_OK;
}

uint8_t net_ip6_pload_pos(struct net_ip6_ctx *ip6)
{
	return NET_IP6_PLOAD_POS_LOWER(ip6->lower) + NET_IP6_HDRSIZE;
}

int8_t net_ip6_recv(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen,
                    uint16_t *dataoffset, uint16_t *datalen)
{
	int8_t errno = 0;
	uint8_t *cursor = NULL;
	uint8_t version = 0;
	uint8_t nh = 0;
	uint8_t *src_addr;
	uint8_t *dst_addr;
	uint16_t length = 0;

	/* Get the packet from the lower layer */
	errno = NET_IP6_RECV_LOWER(ip6->lower, buffer, buflen, dataoffset, datalen);
	if (errno < 0) {
		goto out_zerodata;
	}

	/* Set the cursor to the position of the coap header in the buffer */
	NET_SET_CURSOR(buffer, *dataoffset);

	/* Check that buffer is big enough for parsing ipv6 header size */
	if (!NET_CHECK_BUFLEN(cursor, *datalen, NET_IP6_HDRSIZE)) {
		errno = NET_EOVERFLOW;
		goto out_zerodata;
	}

	/* Read Version + higher 4 bits TC */
	NET_GET_BYTE(version);

	/* Skip lower 4 bits TC + FL */
	NET_SKIP_DATA(3);

	/* Read the payload length */
	NET_GET_SHORT(length);

	/* Read the next header */
	NET_GET_BYTE(nh);

	/* Skip the hop limit */
	NET_SKIP_DATA(1);

	/* Read the source addr */
	NET_GET_DATA(src_addr, 16);

	/* Read the destination addr */
	NET_GET_DATA(dst_addr, 16);

	/* Check version */
	if ((version >> 4) != NET_IP6_VERSION) {
		errno = NET_EPROTO;
		goto out_zerodata;
	}

	/* Check IPv6 data length fits in buffer */
	if (length > (*datalen - NET_IP6_HDRSIZE)) {
		errno = NET_EOVERFLOW;
		goto out_zerodata;
	}

	/* First check the next header, addresses will be check then */
	if (nh == NET_IP6_NH_ICMPV6) {

		*dataoffset += NET_IP6_HDRSIZE;
		*datalen = length;

		/* ICMPv6 traffic, packet is checked for Neighbor Discovery Protocol */
		/* Source and destination address verification is delegated to the handler function */
		errno = _net_ip6_process_icmpv6(ip6, buffer, buflen, dataoffset, datalen,
		                                src_addr, dst_addr);

	} else if (nh == ip6->nh) {

		/* Check that source and destination addresses match our connection */
		if (NET_IP6_CMP_ADDR(src_addr, ip6->dst_addr) &&
		    NET_IP6_CMP_ADDR(dst_addr, ip6->src_addr)) {

			/* This is a data packet, return it to the upper layer */
			*dataoffset += NET_IP6_HDRSIZE;
			*datalen = length;
			errno = NET_STATUS_OK;
			goto out_data;

		} else {
			errno = NET_EAGAIN;
		}

	} else {
		/* Traffic is not relevant for us */
		errno = NET_EAGAIN;
	}

out_zerodata:
	*dataoffset = 0;
	*datalen = 0;
out_data:
	return errno;
}

int8_t net_ip6_send(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen,
                    uint16_t dataoffset, uint16_t datalen)
{
	uint8_t *cursor = NULL;
	uint8_t *cursor_before = NULL;
	uint8_t header_pos = 0;

	/* Retrieve the start of header from lower layers */
	header_pos = NET_IP6_PLOAD_POS_LOWER(ip6->lower);

	/* Set the cursor to the position of the ip6 header in the buffer */
	NET_SET_CURSOR(buffer, header_pos);
	cursor_before = cursor;

	/* Check that buffer is big enough for the IPv6 header size */
	if (!NET_CHECK_BUFLEN(buffer, dataoffset, NET_IP6_HDRSIZE)) {
		return NET_EOVERFLOW;
	}

	/* Put common parts of the IP6 header */
	NET_IP6_PUT_HEADER_COMMON(datalen, ip6->nh, NET_IP6_HOPLIMIT);

	/* Put source and destination addresses */
	NET_PUT_DATA(ip6->src_addr, 16);
	NET_PUT_DATA(ip6->dst_addr, 16);

	dataoffset -= NET_IP6_HDRSIZE;
	datalen += NET_IP6_HDRSIZE;

	/* Pass to the lower layer */
	return NET_IP6_SEND_LOWER(ip6->lower, buffer, buflen, dataoffset, datalen);
}





#define MATCH_NONE           0
#define MATCH_UNICAST        1
#define MATCH_ALLNODES       2
#define MATCH_SOLICITEDNODES 3
#define MATCH_LLADDR         4

static int8_t _net_icmpv6_match_addr(struct net_ip6_ctx *ip6, uint8_t *dst_addr)
{
	/* Match dst addr with our global-unicast address */
	if (NET_IP6_CMP_ADDR(dst_addr, ip6->src_addr)) {
		return MATCH_UNICAST;

	/* Match dst addr with our link-local address (assuming /64 prefix) */
	} else if (NET_IP6_CMP_LLADDR(dst_addr, ip6->src_addr)) {
		return MATCH_LLADDR;

	/* Link-local multicast destinations */
	} else if ((dst_addr[0] == 0xFF) && (dst_addr[1] == 0x02) &&
	           (dst_addr[2] == 0x00) && (dst_addr[3] == 0x00) &&
	           (dst_addr[4] == 0x00) && (dst_addr[5] == 0x00) &&
	           (dst_addr[6] == 0x00) && (dst_addr[7] == 0x00) &&
	           (dst_addr[8] == 0x00) && (dst_addr[9] == 0x00) &&
	           (dst_addr[10] == 0x00)) {

		/* Match dst addr with the all-nodes multicast address */
		if (/*(dst_addr[10] == 0x00) &&*/ (dst_addr[11] == 0x00) &&
		    (dst_addr[12] == 0x00) && (dst_addr[13] == 0x00) &&
		    (dst_addr[14] == 0x00) && (dst_addr[15] == 0x01)) {

			return MATCH_ALLNODES;

		/* Match dst addr with the sollicited-node multicast address */
		/* Note: Global and link-local addresses have the same */
		} else if (/*(dst_addr[10] == 0x00) &&*/ (dst_addr[11] == 0x01) &&
		           (dst_addr[12] == 0xFF) && (dst_addr[13] == ip6->src_addr[13]) &&
		           (dst_addr[14] == ip6->src_addr[14]) && (dst_addr[15] == ip6->src_addr[15])) {

			return MATCH_SOLICITEDNODES;

		} else {
			return MATCH_NONE;
		}

	} else {
		return MATCH_NONE;
	}
}

static void _net_icmpv6_fix_cksum(uint8_t *ip6hdr, uint16_t payloadlen)
{
	uint16_t sum = 0;
	uint8_t nh[] = {0x00, NET_IP6_NH_ICMPV6};

	/* Compute the L4 checksum */
	sum = _net_cksum_sum(sum, &(ip6hdr[8]), 32);
	sum = _net_cksum_sum(sum, &(ip6hdr[4]), 2);
	sum = _net_cksum_sum(sum, nh, 2);
	sum = _net_cksum_sum(sum, &(ip6hdr[40]), payloadlen);
	sum = _net_cksum_finalize(sum);
	if (sum == 0) {
		sum = 0xFFFF;
	}

	/* Rewrite the ICMPv6 checksum field */
	ip6hdr[NET_IP6_HDRSIZE+2] = (uint8_t) ((sum & 0x0000FF00) >> 8);
	ip6hdr[NET_IP6_HDRSIZE+3] = (uint8_t) (sum & 0x000000FF);
}


/**
 *
 * ICMPv6 header
 *
 * 0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Type      |     Code      |          Checksum             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * +                         Message Body                          +
 * |                                                               |
 */

int8_t _net_ip6_process_icmpv6(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen,
                               uint16_t *dataoffset, uint16_t *datalen,
                               uint8_t *src_addr, uint8_t *dst_addr)
{
	int8_t errno = 0;
	uint8_t *cursor = NULL;
	uint8_t type = 0;
	uint8_t src_addr_copy[16];
	uint8_t tgt_addr[16];
	int8_t dst_match;

	/* Set the cursor to the position of the ipv6 header in the buffer */
	NET_SET_CURSOR(buffer, *dataoffset);

	/* Check the ICMPV6 header fits in the packet length */
	if (!NET_CHECK_BUFLEN(cursor, *datalen, NET_ICMPV6_HDRSIZE)) {
		errno = NET_EOVERFLOW;
		goto out_end;
	}

	/* Read ICMPv6 type (RS/RA/NS/NA) */
	NET_GET_BYTE(type);

	/* Skip code and checksum, which won't be verified */
	NET_SKIP_DATA(3);

	if (type == NET_ICMPV6_TYPE_NS) {
		/* Most important NDP message, IPv6 will be broken in most cases if not processed */

		dst_match = _net_icmpv6_match_addr(ip6, dst_addr);
		if (dst_match == MATCH_NONE) {
			/* Not for us */
			errno = NET_EAGAIN;
			goto out_end;
		}

		/**
		 * 0                   1                   2                   3
		 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |     Type      |     Code      |          Checksum             |
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |                           Reserved                            |
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |                                                               |
		 * +                                                               +
		 * |                                                               |
		 * +                       Target Address                          +
		 * |                                                               |
		 * +                                                               +
		 * |                                                               |
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * |   Options ...
		 * +-+-+-+-+-+-+-+-+-+-+-+-
		 */

		/* Check the ICMPV6 NS header fits in the packet length */
		if (!NET_CHECK_BUFLEN(cursor, *datalen,
		                      NET_ICMPV6_HDRSIZE + NET_ICMPV6_NS_HDRSIZE)) {
			errno = NET_EOVERFLOW;
			goto out_end;
		}

		/* Skip reserved field */
		NET_SKIP_DATA(4);

		/* Read target address, need a copy because buffer will be reused */
		NET_GET_COPY(tgt_addr, 16);

		/* Compare the target address with our unicast and link-local addresses */
		if (!NET_IP6_CMP_ADDR(tgt_addr, ip6->src_addr) &&
		    !NET_IP6_CMP_LLADDR(tgt_addr, ip6->src_addr)) {

			/* Not for us */
			errno = NET_EAGAIN;
			goto out_end;
		}

		/* NS from non-unspec addresses will be replied to the unicast source */
		if (!NET_IP6_CMP_UNSPEC(src_addr)) {
			src_addr_copy[0] = src_addr[0];
			src_addr_copy[1] = src_addr[1];
			src_addr_copy[2] = src_addr[2];
			src_addr_copy[3] = src_addr[3];
			src_addr_copy[4] = src_addr[4];
			src_addr_copy[5] = src_addr[5];
			src_addr_copy[6] = src_addr[6];
			src_addr_copy[7] = src_addr[7];
			src_addr_copy[8] = src_addr[8];
			src_addr_copy[9] = src_addr[9];
			src_addr_copy[10] = src_addr[10];
			src_addr_copy[11] = src_addr[11];
			src_addr_copy[12] = src_addr[12];
			src_addr_copy[13] = src_addr[13];
			src_addr_copy[14] = src_addr[14];
			src_addr_copy[15] = src_addr[15];

			/* Do send the neighbor advertisement to the unicast address */
			_net_icmpv6_send_na(ip6, buffer, buflen, *dataoffset - NET_IP6_HDRSIZE,
			                    src_addr_copy, tgt_addr, true);
		} else {
			/* Do send the neighbor advertisement to the all-nodes address */
			_net_icmpv6_send_na(ip6, buffer, buflen, *dataoffset - NET_IP6_HDRSIZE,
			                    NULL, tgt_addr, true);
		}

	} else if (type == NET_ICMPV6_TYPE_NA) {

	} else if (type == NET_ICMPV6_TYPE_RA) {
		/* Not implemented yet */
	}
	/* Other ICMPv6 messages are of no interest for us */

	errno = NET_EAGAIN;

out_end:
	return errno;
}

int8_t _net_icmpv6_send_na(struct net_ip6_ctx *ip6, uint8_t *buffer, uint16_t buflen, uint16_t dataoffset,
                           uint8_t *dst_addr, uint8_t *tgt_addr, bool solicited)
{
	uint8_t *cursor = NULL;
	uint8_t *cursor_before = NULL;
	uint8_t header_pos = 0;

	/* Set the cursor to the position of the ipv6 header in the buffer */
	NET_SET_CURSOR(buffer, dataoffset);
	cursor_before = cursor;

	/* Check that buffer is big enough for the Neighbor Advertisement header size */
	if (!NET_CHECK_BUFLEN(buffer, buflen,
	                      NET_IP6_HDRSIZE + NET_ICMPV6_NA_HDRSIZE + NET_ICMPV6_NDP_OPT_LLA_HDRSIZE)) {
		return NET_EOVERFLOW;
	}

	/* Put common parts of the IP6 header */
	NET_IP6_PUT_HEADER_COMMON(NET_ICMPV6_HDRSIZE + NET_ICMPV6_NA_HDRSIZE + NET_ICMPV6_NDP_OPT_LLA_HDRSIZE,
	                          NET_IP6_NH_ICMPV6, NET_IP6_HOPLIMIT);

	/* Put source address (link-local unicast addr) */
	NET_PUT_SHORT(0xFE80);
	NET_PUT_SHORT(0x0000);
	NET_PUT_SHORT(0x0000);
	NET_PUT_SHORT(0x0000);
	NET_PUT_DATA(&(ip6->src_addr[8]), 8);

	/* Put destination address (source address of the sollicitation, or multicast all-nodes) */
	if (dst_addr) {
		NET_PUT_DATA(dst_addr, 16);
	} else {
		NET_PUT_SHORT(0xFF02);
		NET_PUT_SHORT(0x0000);
		NET_PUT_SHORT(0x0000);
		NET_PUT_SHORT(0x0000);
		NET_PUT_SHORT(0x0000);
		NET_PUT_SHORT(0x0000);
		NET_PUT_SHORT(0x0000);
		NET_PUT_SHORT(0x0001);
	}

	/* Put the Type field */
	NET_PUT_BYTE(NET_ICMPV6_TYPE_NA);

	/* Put the Code field */
	NET_PUT_BYTE(0x00);

	/* Put the Checksum field (to be computed later) */
	NET_PUT_SHORT(0x0000);

	/* Put options + reserved */
	if (solicited) {
		NET_PUT_SHORT(0x6000);
	} else {
		NET_PUT_SHORT(0x2000);
	}
	NET_PUT_SHORT(0x0000);

	/* Put the Target field */
	NET_PUT_DATA(tgt_addr, 16);

	/* Put the Target Link-Layer address Option */
	NET_PUT_BYTE(NET_ICMPV6_NDP_OPT_TGTLLADDR);
	NET_PUT_BYTE(0x01);
#ifdef NET_HAS_GET_L2_ADDR
	NET_PUT_DATA(NET_IP6_GET_L2_ADDR_LOWER(ip6->lower), 6);
#else
	NET_PUT_SHORT(0x0000);
	NET_PUT_SHORT(0x0000);
	NET_PUT_SHORT(0x0000);
#endif

	/* Fix the checksum in the packet */
	_net_icmpv6_fix_cksum(cursor_before,
	                      NET_ICMPV6_HDRSIZE + NET_ICMPV6_NA_HDRSIZE + NET_ICMPV6_NDP_OPT_LLA_HDRSIZE);

	/**
	 * Pass to the lower layer
	 * Note: we don't touch to dataoffset since it is already positionned at the beginning of the packet
	 */
	/* TODO send to the soliciting node */
	return NET_IP6_SEND_LOWER(ip6->lower, buffer, buflen, dataoffset,
	                          NET_IP6_HDRSIZE + NET_ICMPV6_HDRSIZE + NET_ICMPV6_NA_HDRSIZE + NET_ICMPV6_NDP_OPT_LLA_HDRSIZE);
}
