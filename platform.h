/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PLATFORM_H
#define _PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern void msleep(uint16_t time_ms);

extern void spi_init();
extern void spi_destroy();
extern void spi_start_transaction();
extern void spi_stop_transaction();
extern void spi_start_transfer();
extern void spi_stop_transfer();
extern uint8_t spi_read_byte();
extern void spi_read(uint8_t *buffer, uint16_t buflen);
extern void spi_write(uint8_t *buffer, uint16_t buflen);

extern void serial_init();
extern void serial_debug_beg();
extern void serial_debug_end();
extern void serial_debug(const char * const message);
extern void serial_signal(uint8_t signal);
extern uint16_t serial_read(uint8_t *buffer, uint16_t buflen);
extern uint8_t serial_wait_for_signal(uint16_t timeout);
extern uint16_t serial_write(uint8_t *buffer, uint16_t buflen);

#ifdef __cplusplus
}
#endif

#endif
