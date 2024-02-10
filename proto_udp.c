/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "proto_udp.h"
#include "net_utils.h"

#define NET_UDP_GET_L3_CKSUM(...)     NET_UDP_PROTO_LOWER(_get_l3_cksum)(__VA_ARGS__)
#define NET_UDP_CONNECT_LOWER(...)    NET_UDP_PROTO_LOWER(_connect)(__VA_ARGS__)
#define NET_UDP_PLOAD_POS_LOWER(...)  NET_UDP_PROTO_LOWER(_pload_pos)(__VA_ARGS__)
#define NET_UDP_RECV_LOWER(...)       NET_UDP_PROTO_LOWER(_recv)(__VA_ARGS__)
#define NET_UDP_SEND_LOWER(...)       NET_UDP_PROTO_LOWER(_send)(__VA_ARGS__)


/**
 *  0      7 8     15 16    23 24    31
 * +--------+--------+--------+--------+
 * |     Source      |   Destination   |
 * |      Port       |      Port       |
 * +--------+--------+--------+--------+
 * |                 |                 |
 * |     Length      |    Checksum     |
 * +--------+--------+--------+--------+
 * |
 * |          data octets ...
 * +---------------- ...
 */

#define NET_UDP_HDRSIZE 8

static void _net_udp_fix_cksum(uint16_t cksum_pre_compute, uint8_t *udpbuf, uint16_t udplen)
{
	uint16_t sum = cksum_pre_compute;

	/* Payload length from the L3 pseudo-header */
	sum = _net_cksum_sum(sum, &(udpbuf[4]), 2);

	/* UDP header + data */
	sum = _net_cksum_sum(sum, udpbuf, udplen);

	sum = _net_cksum_finalize(sum);
	if (sum == 0) {
		sum = 0xFFFF;
	}

	/* Rewrite the UDP checksum field */
	udpbuf[6] = (uint8_t) ((sum & 0x0000FF00) >> 8);
	udpbuf[7] = (uint8_t) (sum & 0x000000FF);
}

int8_t net_udp_set_source_port(struct net_udp_ctx *udp, uint16_t source_port)
{
	udp->source_port = source_port;
	return NET_STATUS_OK;
}

int8_t net_udp_set_destination_port(struct net_udp_ctx *udp, uint16_t destination_port)
{
	udp->destination_port = destination_port;
	return NET_STATUS_OK;
}


int8_t net_udp_connect(struct net_udp_ctx *udp)
{
	int8_t errno;

	if (udp->source_port == 0) {
		/* Todo: use a pseudo-random */
		return NET_ECONFIG;
	}

	if (udp->destination_port == 0) {
		return NET_ECONFIG;
	}

	errno = NET_UDP_CONNECT_LOWER(udp->lower);

#ifdef NET_HAS_GET_L3_CKSUM
	udp->cksum_pre_compute = NET_UDP_GET_L3_CKSUM(udp->lower);
#else
	udp->cksum_pre_compute = 0;
#endif

	return errno;
}

uint8_t net_udp_pload_pos(struct net_udp_ctx *udp)
{
	return NET_UDP_PLOAD_POS_LOWER(udp->lower) + NET_UDP_HDRSIZE;
}

int8_t net_udp_recv(struct net_udp_ctx *udp, uint8_t *buffer, uint16_t buflen,
                    uint16_t *dataoffset, uint16_t *datalen)
{
	int8_t errno = 0;
	uint8_t *cursor = NULL;
	uint16_t source_port = 0;
	uint16_t destination_port = 0;
	uint16_t length = 0;

	/* Get the packet from the lower layer */
	errno = NET_UDP_RECV_LOWER(udp->lower, buffer, buflen, dataoffset, datalen);
	if (errno < 0) {
		goto out_zerodata;
	}

	/* Set the cursor to the position of the coap header in the buffer */
	NET_SET_CURSOR(buffer, *dataoffset);

	/* Check that buffer is big enough for coap header size */
	if (!NET_CHECK_BUFLEN(cursor, *datalen, NET_UDP_HDRSIZE)) {
		errno = NET_EOVERFLOW;
		goto out_zerodata;
	}

	/* Read the source port */
	NET_GET_SHORT(source_port);

	/* Read the destination port */
	NET_GET_SHORT(destination_port);

	/* Check that ports match */
	if ((source_port != udp->destination_port) ||
	    (destination_port != udp->source_port)) {
		errno = NET_EAGAIN;
		goto out_zerodata;
	}

	/* Read the payload length */
	NET_GET_SHORT(length);

	/* Check that length fits in the remaining packet length */
	if (*datalen < length) {
		return NET_EOVERFLOW;
	}


	*dataoffset += NET_UDP_HDRSIZE;
	*datalen -= NET_UDP_HDRSIZE;

	return NET_STATUS_OK;

out_zerodata:
	*dataoffset = 0;
	*datalen = 0;
out_data:
	return errno;
}

int8_t net_udp_send(struct net_udp_ctx *udp, uint8_t *buffer, uint16_t buflen,
                    uint16_t dataoffset, uint16_t datalen)
{
	uint8_t *cursor = NULL;
	uint8_t *cursor_before = NULL;
	uint8_t header_pos = 0;
	uint16_t checksum = 0;


	/* Retrieve the start of header from lower layers */
	header_pos = NET_UDP_PLOAD_POS_LOWER(udp->lower);

	/* Set the cursor to the position of the udp header in the buffer */
	NET_SET_CURSOR(buffer, header_pos);
	cursor_before = cursor;

	/* Check that buffer is big enough for udp header size */
	if (!NET_CHECK_BUFLEN(buffer, dataoffset, NET_UDP_HDRSIZE)) {
		return NET_EOVERFLOW;
	}

	/* Set the source port */
	NET_PUT_SHORT(udp->source_port);

	/* Set the destination port */
	NET_PUT_SHORT(udp->destination_port);

	/* Set the data length */
	NET_PUT_SHORT(NET_UDP_HDRSIZE + datalen);

	/* Placeholder for the checksum */
	NET_PUT_SHORT(0x0000);

#ifdef NET_HAS_GET_L3_CKSUM
	/* Set the checksum */
	_net_udp_fix_cksum(udp->cksum_pre_compute, cursor_before, NET_UDP_HDRSIZE + datalen);
#endif

	dataoffset -= NET_UDP_HDRSIZE;
	datalen += NET_UDP_HDRSIZE;

	/* Pass to the lower layer */
	return NET_UDP_SEND_LOWER(udp->lower, buffer, buflen, dataoffset, datalen);
}
