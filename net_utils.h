/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define NET_SET_CURSOR(buffer, position) \
	cursor = (((uint8_t*) buffer) + position)

#define NET_CHECK_BUFLEN(buffer, buflen, datalen) \
	((buffer + buflen) >= (cursor + datalen))

#define NET_PUT_BYTE(value) \
	do { \
		((uint8_t*) cursor)[0] = value; \
		cursor = ((uint8_t*) cursor) + 1; \
	} while (0)

#define NET_PUT_SHORT(value) \
	do { \
		((uint8_t*) cursor)[0] = (value & 0xFF00) >> 8; \
		((uint8_t*) cursor)[1] = (value & 0x00FF); \
		cursor = ((uint8_t*) cursor) + 2; \
	} while (0)

#define NET_PUT_INT(value) \
	do { \
		((uint8_t*) cursor)[0] = (value & 0xFF000000) >> 24; \
		((uint8_t*) cursor)[1] = (value & 0x00FF0000) >> 16; \
		((uint8_t*) cursor)[2] = (value & 0x0000FF00) >> 8; \
		((uint8_t*) cursor)[3] = (value & 0x000000FF); \
		cursor = ((uint8_t*) cursor) + 4; \
	} while (0)

#define NET_PUT_DATA(value, valuelen) \
	do { \
		memcpy(cursor, value, valuelen); \
		cursor = ((uint8_t*) cursor) + valuelen; \
	} while (0)


#define NET_GET_BYTE(value) \
	do { \
		value = ((uint8_t*) cursor)[0]; \
		cursor = ((uint8_t*) cursor) + 1; \
	} while (0)

#define NET_GET_SHORT(value) \
	do { \
		value = (((uint8_t*) cursor)[0] << 8) + \
		         ((uint8_t*) cursor)[1]; \
		cursor = ((uint8_t*) cursor) + 2; \
	} while (0)

#define NET_GET_INT(value) \
	do { \
		value = (((uint8_t*) cursor)[0] << 24) + \
		        (((uint8_t*) cursor)[1] << 16) + \
		        (((uint8_t*) cursor)[2] << 8) + \
		         ((uint8_t*) cursor)[3]; \
		cursor = ((uint8_t*) cursor) + 4; \
	} while (0)

#define NET_GET_DATA(value, valuelen) \
	do { \
		value = cursor; \
		cursor = ((uint8_t*) cursor) + valuelen; \
	} while (0)

#define NET_GET_COPY(value, valuelen) \
	do { \
		memcpy(value, cursor, valuelen); \
		cursor = ((uint8_t*) cursor) + valuelen; \
	} while (0)

#define NET_CMP_DATA(result, value, valuelen) \
	do { \
		result = memcmp(value, cursor, valuelen); \
		cursor = ((uint8_t*) cursor) + valuelen; \
	} while (0)

#define NET_SKIP_DATA(valuelen) \
	cursor = ((uint8_t*) cursor) + valuelen



inline static uint16_t _net_cksum_sum(uint16_t init, uint8_t *data, uint16_t datalen)
{
	uint32_t sum = init;
	uint16_t i = 0;

	for (i=0; i<datalen-1; i+=2) {
		sum += (uint16_t) ((data[i]<<8) | data[i+1]);
		if (sum & 0xFFFF0000) {
			sum = (sum & 0x0000FFFF) + 1;
		}
	}

	/* datalen was odd */
	if (i == datalen-1) {
		sum += (uint16_t) (data[i]<<8);
		if (sum & 0xFFFF0000) {
			sum = (sum & 0x0000FFFF) + 1;
		}
	}

	return (uint16_t) sum;
}

inline static uint16_t _net_cksum_finalize(uint16_t sum)
{
	/* Trim until sum fits into 16 bits */
	while (sum & 0xFFFF0000) {
		sum = ((sum & 0xFFFF0000) >> 16) + (sum & 0x0000FFFF);
	}

	return ~sum;
}


#ifdef __cplusplus
}
#endif

#endif
