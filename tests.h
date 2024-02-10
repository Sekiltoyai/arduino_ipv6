/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TESTS_H
#define _TESTS_H

#include "config.h"
#include "proto_stub.h"
#include "proto_ip6.h"
#include "proto_udp.h"
#include "proto_coap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VERDICT_OK  0x00
#define VERDICT_NOK 0x01

extern void tests_init();
extern uint8_t tests_exec(uint8_t test_id);

#ifdef __cplusplus
}
#endif

#endif
