/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


#define NET_PROTO_DEFAULT(SUFFIX)    net_coap ## SUFFIX
#define NET_COAP_PROTO_LOWER(SUFFIX) net_udp ## SUFFIX
#define NET_UDP_PROTO_LOWER(SUFFIX)  net_ip6 ## SUFFIX
#define NET_IP6_PROTO_LOWER(SUFFIX)  net_mac ## SUFFIX
#define NET_MAC_PROTO_LOWER(SUFFIX)  hw_serial ## SUFFIX

#if 0
#define NET_PROTO_DEFAULT(SUFFIX)    net_coap ## SUFFIX
#define NET_COAP_PROTO_LOWER(SUFFIX) net_udp ## SUFFIX
#define NET_UDP_PROTO_LOWER(SUFFIX)  net_ip6 ## SUFFIX
#define NET_IP6_PROTO_LOWER(SUFFIX)  net_mac ## SUFFIX
#define NET_MAC_PROTO_LOWER(SUFFIX)  hw_w5500 ## SUFFIX
#endif

#define NET_DEFAULT_RECV(...)     NET_PROTO_DEFAULT(_recv)(__VA_ARGS__)
#define NET_DEFAULT_SEND(...)     NET_PROTO_DEFAULT(_send)(__VA_ARGS__)

#define USE_SPI
#define USE_SERIAL

#define NET_STUB_GET_L2_ADDR_ENABLE 1

#include "common.h"
#include "hw_serial.h"
//#include "hw_w5500.h"
#include "proto_mac.h"
#include "proto_ip6.h"
#include "proto_udp.h"
#include "proto_coap.h"

#ifdef __cplusplus
}
#endif

#endif
