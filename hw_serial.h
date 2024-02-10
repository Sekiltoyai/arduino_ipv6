/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HW_SERIAL_H
#define _HW_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


struct hw_serial_ctx {
#ifdef NET_SERIAL_GET_L2_ADDR_ENABLE
	uint8_t *l2_addr;
#endif
};

#ifdef NET_SERIAL_GET_L2_ADDR_ENABLE
#define NET_HAS_GET_L2_ADDR 1
extern uint8_t *hw_serial_get_l2_addr(struct hw_serial_ctx *serial);
#endif
extern uint16_t hw_serial_recv(struct hw_serial_ctx *serial,
                               uint8_t *buffer, uint16_t buflen);
extern uint16_t hw_serial_send(struct hw_serial_ctx *serial,
                               uint8_t *buffer, uint16_t buflen);


#ifdef __cplusplus
}
#endif

#endif
