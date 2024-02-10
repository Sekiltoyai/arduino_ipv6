/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "hw_serial.h"
#include "platform.h"
#include "net_utils.h"

#include <stdlib.h>


#ifdef NET_SERIAL_GET_L2_ADDR_ENABLE
uint8_t *hw_serial_get_l2_addr(struct hw_serial_ctx *serial)
{
	return serial->l2_addr;
}
#endif

//int8_t hw_serial_recv(struct net_serial_ctx *serial, uint8_t *buffer, uint16_t buflen,
//                      uint16_t *dataoffset, uint16_t *datalen)
uint16_t hw_serial_recv(struct hw_serial_ctx *serial, uint8_t *buffer, uint16_t buflen)
{
	return serial_read(buffer, buflen);
}

//int8_t hw_serial_send(struct net_serial_ctx *serial, uint8_t *buffer, uint16_t buflen,
//                      uint16_t dataoffset, uint16_t datalen)
uint16_t hw_serial_send(struct hw_serial_ctx *serial, uint8_t *buffer, uint16_t buflen)
{
	return serial_write(buffer, buflen);
}
