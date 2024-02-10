/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HW_W5500_H
#define _HW_W5500_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define HW_SPEED_NONE        0x00
#define HW_SPEED_10MBPS_HD   0x01
#define HW_SPEED_10MBPS_FD   0x02
#define HW_SPEED_100MBPS_HD  0x03
#define HW_SPEED_100MBPS_FD  0x04


struct hw_w5500_ctx {};

extern void hw_w5500_init();
extern void hw_w5500_destroy();
extern void hw_w5500_read_version(uint8_t *version);
extern void hw_w5500_set_phycfg(bool up, bool autoneg, uint8_t speed);
extern void hw_w5500_get_phycfg(bool *up, uint8_t *speed);
extern void hw_w5500_set_macaddress(uint8_t *macaddress);
extern void hw_w5500_get_macaddress(uint8_t *macaddress);

extern bool hw_w5500_open(struct hw_w5500_ctx *w5500);
extern uint16_t hw_w5500_recv(struct hw_w5500_ctx *w5500, uint8_t *buffer, uint16_t buflen);
extern uint16_t hw_w5500_send(struct hw_w5500_ctx *w5500, uint8_t *buffer, uint16_t buflen);

#ifdef __cplusplus
}
#endif

#endif
