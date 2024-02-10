/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tests.h"
#include "platform.h"

#include <stdlib.h>
#include <string.h>

#define DEBUG(...) serial_debug(__VA_ARGS__)

#define TEST_RECV_RETRY(call) \
	for (retry=0; retry<5; retry++) { \
		if ((call) != NET_EAGAIN) { \
			break; \
		} \
	}

#define TEST_ASSERT(cond) \
	do { \
		if (!(cond)) { \
			return VERDICT_NOK; \
		} \
	} while(0)


uint8_t buffer[1514];

uint8_t src_addr[16] = {0x20,0x01,
                        0x00,0x01,
                        0x00,0x02,
                        0x00,0x03,
                        0x00,0x0f,
                        0x00,0x0e,
                        0x00,0x0d,
                        0x00,0x0c};
uint8_t dst_addr[16] = {0x20,0x01,
                        0x00,0x01,
                        0x00,0x02,
                        0x00,0x03,
                        0x00,0x0a,
                        0x00,0x0b,
                        0x00,0x0c,
                        0x00,0x0d};

uint8_t src_l2addr[6] = {0x10, 0x22, 0x33, 0x44, 0x55, 0x66};
uint8_t dst_l2addr[6] = {0x76, 0x88, 0x99, 0xAA, 0xBB, 0xCC};


struct hw_serial_ctx serial;
struct net_mac_ctx mac = { .lower = &serial };
struct net_ip6_ctx ip6 = { .lower = &mac };
struct net_udp_ctx udp = { .lower = &ip6 };
struct net_coap_ctx coap = { .lower = &udp };


void tests_init()
{
}

static uint8_t test_mac_recv_nodata()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);


	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_LB) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_mac_recv(&mac, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(datalen == 0);

	return VERDICT_OK;
}

static uint8_t test_mac_recv_data_common()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_LB) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_mac_recv(&mac, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(datalen == 4);
	TEST_ASSERT(memcmp(&(buffer[dataoffset]), "test", datalen) == 0);

	return VERDICT_OK;
}

static uint8_t test_mac_recv_data_ucast()
{
	DEBUG(__FUNCTION__);

	return test_mac_recv_data_common();
}

static uint8_t test_mac_recv_data_mcast()
{
	DEBUG(__FUNCTION__);

	return test_mac_recv_data_common();
}

static uint8_t test_mac_recv_badcommon()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_LB) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_mac_recv(&mac, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_EAGAIN);

	return VERDICT_OK;
}

static uint8_t test_mac_recv_badethtype()
{
	DEBUG(__FUNCTION__);

	return test_mac_recv_badcommon();
}

static uint8_t test_mac_recv_baddst()
{
	DEBUG(__FUNCTION__);

	return test_mac_recv_badcommon();
}

static uint8_t test_mac_send_nodata()
{
	uint16_t dataoffset = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_LB) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	dataoffset = net_mac_pload_pos(&mac);
	TEST_ASSERT(net_mac_send(&mac, buffer, 1514, dataoffset, 0) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_mac_send_data()
{
	uint16_t dataoffset = 0;
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_LB) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	dataoffset = net_mac_pload_pos(&mac);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_mac_send(&mac, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	return VERDICT_OK;
}


static uint8_t test_ip6_recv_nodata()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_NONXT) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_ip6_recv(&ip6, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(datalen == 0);

	return VERDICT_OK;
}

static uint8_t test_ip6_recv_data()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, 253) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_ip6_recv(&ip6, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(datalen == 4);
	TEST_ASSERT(memcmp(&(buffer[dataoffset]), "test", datalen) == 0);

	return VERDICT_OK;
}

static uint8_t test_ip6_recv_badcommon()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, 253) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_ip6_recv(&ip6, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_EAGAIN);

	return VERDICT_OK;
}

static uint8_t test_ip6_recv_badnh()
{
	DEBUG(__FUNCTION__);

	return test_ip6_recv_badcommon();
}

static uint8_t test_ip6_recv_badsrc()
{
	DEBUG(__FUNCTION__);

	return test_ip6_recv_badcommon();
}

static uint8_t test_ip6_recv_baddst()
{
	DEBUG(__FUNCTION__);

	return test_ip6_recv_badcommon();
}

static uint8_t test_ip6_recv_badlen()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, 253) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_ip6_recv(&ip6, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_EOVERFLOW);

	return VERDICT_OK;
}

static uint8_t test_ip6_send_nodata()
{
	uint16_t dataoffset = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, 59) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	dataoffset = net_ip6_pload_pos(&ip6);
	TEST_ASSERT(net_ip6_send(&ip6, buffer, 1514, dataoffset, 0) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_ip6_send_data()
{
	uint16_t dataoffset = 0;
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, 253) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	dataoffset = net_ip6_pload_pos(&ip6);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_ip6_send(&ip6, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_ip6_icmpv6_nsna_recv_common()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, 253) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_connect(&ip6) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_ip6_recv(&ip6, &(buffer[1514 - 256]), 256, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_EAGAIN);

	return VERDICT_OK;
}


static uint8_t test_ip6_icmpv6_nsna_recv_uc()
{
	DEBUG(__FUNCTION__);

	return test_ip6_icmpv6_nsna_recv_common();
}

static uint8_t test_ip6_icmpv6_nsna_recv_lla()
{
	DEBUG(__FUNCTION__);

	return test_ip6_icmpv6_nsna_recv_common();
}

static uint8_t test_ip6_icmpv6_nsna_recv_mcsn()
{
	DEBUG(__FUNCTION__);

	return test_ip6_icmpv6_nsna_recv_common();
}

static uint8_t test_ip6_icmpv6_nsna_recv_dad()
{
	DEBUG(__FUNCTION__);

	return test_ip6_icmpv6_nsna_recv_common();
}

static uint8_t test_ip6_icmpv6_nsna_recv_badtgt()
{
	DEBUG(__FUNCTION__);

	return test_ip6_icmpv6_nsna_recv_common();
}


static uint8_t test_udp_recv_nodata()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5678) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_connect(&udp) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_udp_recv(&udp, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(datalen == 0);

	return VERDICT_OK;
}

static uint8_t test_udp_recv_data()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5678) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_connect(&udp) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_udp_recv(&udp, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(datalen == 4);
	TEST_ASSERT(memcmp(&(buffer[dataoffset]), "test", datalen) == 0);

	return VERDICT_OK;
}

static uint8_t test_udp_recv_badcommon()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5678) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_connect(&udp) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_udp_recv(&udp, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_EAGAIN);

	return VERDICT_OK;
}

static uint8_t test_udp_recv_badsrc()
{
	DEBUG(__FUNCTION__);

	return test_udp_recv_badcommon();
}

static uint8_t test_udp_recv_baddst()
{
	DEBUG(__FUNCTION__);

	return test_udp_recv_badcommon();
}

static uint8_t test_udp_recv_badlen()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5678) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_connect(&udp) == NET_STATUS_OK);
	TEST_RECV_RETRY(err = net_udp_recv(&udp, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_EOVERFLOW);

	return VERDICT_OK;
}

static uint8_t test_udp_send_nodata()
{
	uint16_t dataoffset = 0;
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5678) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_connect(&udp) == NET_STATUS_OK);
	dataoffset = net_udp_pload_pos(&udp);
	TEST_ASSERT(net_udp_send(&udp, buffer, 1514, dataoffset, 0) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_udp_send_data()
{
	uint16_t dataoffset = 0;
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5678) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_connect(&udp) == NET_STATUS_OK);
	dataoffset = net_udp_pload_pos(&udp);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_udp_send(&udp, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_coap_noncf_send_nodata()
{
	uint16_t dataoffset = 0;
	uint8_t token[] = {0x12};
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_NONCONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);
	dataoffset = net_coap_pload_pos(&coap);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 0) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_coap_noncf_send_data()
{
	uint16_t dataoffset = 0;
	uint8_t token[] = {0x34};
	char * const uriquery[] = { "stub=stub" };
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_NONCONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_uriquery(&coap, 1, uriquery) == NET_STATUS_OK);
	dataoffset = net_coap_pload_pos(&coap);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	return VERDICT_OK;
}

static uint8_t test_coap_noncf_send_data_resp()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	uint8_t token[] = {0x56};
	char * const uriquery[] = { "stub=stub" };
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_NONCONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_uriquery(&coap, 1, uriquery) == NET_STATUS_OK);

	dataoffset = net_coap_pload_pos(&coap);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_coap_recv(&coap, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(net_coap_get_responsecode(&coap) == NET_COAP_CODE_CREATED);

	return VERDICT_OK;
}

static uint8_t test_coap_cf_send_nodata()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	uint8_t token[] = {0x78};
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_CONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);

	dataoffset = net_coap_pload_pos(&coap);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 0) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_coap_recv(&coap, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_COAP_STATUS_ACK);

	return VERDICT_OK;
}

static uint8_t test_coap_cf_send_data()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	uint8_t token[] = {0x9a};
	char * const uriquery[] = { "stub=stub" };
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_CONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_uriquery(&coap, 1, uriquery) == NET_STATUS_OK);

	dataoffset = net_coap_pload_pos(&coap);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_coap_recv(&coap, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_COAP_STATUS_ACK);

	return VERDICT_OK;
}

static uint8_t test_coap_cf_send_data_ackresp()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	uint8_t token[] = {0xbc};
	char * const uriquery[] = { "stub=stub" };
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_CONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_uriquery(&coap, 1, uriquery) == NET_STATUS_OK);

	dataoffset = net_coap_pload_pos(&coap);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_coap_recv(&coap, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_COAP_STATUS_ACK);

	TEST_RECV_RETRY(err = net_coap_recv(&coap, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_STATUS_OK);
	TEST_ASSERT(net_coap_get_responsecode(&coap) == NET_COAP_CODE_CREATED);

	return VERDICT_OK;
}

static uint8_t test_coap_cf_send_data_piggybacked()
{
	int8_t retry = 0;
	int8_t err = 0;
	uint16_t dataoffset = 0;
	uint16_t datalen = 0;
	uint8_t token[] = {0xde};
	char * const uriquery[] = { "stub=stub" };
	uint8_t payload[] = "test";
	net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);

	DEBUG(__FUNCTION__);

	TEST_ASSERT(net_mac_set_source_addr(&mac, src_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_destination_addr(&mac, dst_l2addr) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6) == NET_STATUS_OK);
	TEST_ASSERT(net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes) == NET_STATUS_OK);

	TEST_ASSERT(net_ip6_set_source_addr(&ip6, src_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_destination_addr(&ip6, dst_addr) == NET_STATUS_OK);
	TEST_ASSERT(net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP) == NET_STATUS_OK);

	TEST_ASSERT(net_udp_set_source_port(&udp, 1234) == NET_STATUS_OK);
	TEST_ASSERT(net_udp_set_destination_port(&udp, 5683) == NET_STATUS_OK);

	TEST_ASSERT(net_coap_connect(&coap)  == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_method(&coap, NET_COAP_TYPE_CONFIRMABLE, NET_COAP_CODE_POST) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_token(&coap, 1, token) == NET_STATUS_OK);
	TEST_ASSERT(net_coap_set_uriquery(&coap, 1, uriquery) == NET_STATUS_OK);

	dataoffset = net_coap_pload_pos(&coap);
	memcpy(&(buffer[dataoffset]), payload, 4);
	TEST_ASSERT(net_coap_send(&coap, buffer, 1514, dataoffset, 4) == NET_STATUS_OK);

	TEST_RECV_RETRY(err = net_coap_recv(&coap, buffer, 1514, &dataoffset, &datalen));
	TEST_ASSERT(err == NET_COAP_STATUS_ACK);
	TEST_ASSERT(net_coap_get_responsecode(&coap) == NET_COAP_CODE_CREATED);

	return VERDICT_OK;
}

uint8_t tests_exec(uint8_t test_id)
{
	switch(test_id) {
	case 0x11: return test_mac_recv_nodata();
	case 0x12: return test_mac_recv_data_ucast();
	case 0x13: return test_mac_recv_data_mcast();
	case 0x14: return test_mac_recv_badethtype();
	case 0x15: return test_mac_recv_baddst();
	case 0x16: return test_mac_send_nodata();
	case 0x17: return test_mac_send_data();

	case 0x21: return test_ip6_recv_nodata();
	case 0x22: return test_ip6_recv_data();
	case 0x23: return test_ip6_recv_badnh();
	case 0x24: return test_ip6_recv_badsrc();
	case 0x25: return test_ip6_recv_baddst();
	case 0x26: return test_ip6_recv_badlen();
	case 0x27: return test_ip6_send_nodata();
	case 0x28: return test_ip6_send_data();

	case 0x31: return test_ip6_icmpv6_nsna_recv_uc();
	case 0x32: return test_ip6_icmpv6_nsna_recv_lla();
	case 0x33: return test_ip6_icmpv6_nsna_recv_mcsn();
	case 0x34: return test_ip6_icmpv6_nsna_recv_dad();
	case 0x35: return test_ip6_icmpv6_nsna_recv_badtgt();

	case 0x51: return test_udp_recv_nodata();
	case 0x52: return test_udp_recv_data();
	case 0x53: return test_udp_recv_badsrc();
	case 0x54: return test_udp_recv_baddst();
	case 0x55: return test_udp_recv_badlen();
	case 0x56: return test_udp_send_nodata();
	case 0x57: return test_udp_send_data();

	case 0x61: return test_coap_noncf_send_nodata();
	case 0x62: return test_coap_noncf_send_data();
	case 0x63: return test_coap_noncf_send_data_resp();
	case 0x64: return test_coap_cf_send_nodata();
	case 0x65: return test_coap_cf_send_data();
	case 0x66: return test_coap_cf_send_data_ackresp();
	case 0x67: return test_coap_cf_send_data_piggybacked();
	}

	return 0x01;
}
