/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "proto_coap.h"
#include "net_utils.h"

#include <stdlib.h>


#define NET_COAP_CONNECT_LOWER(...)    NET_COAP_PROTO_LOWER(_connect)(__VA_ARGS__)
#define NET_COAP_PLOAD_POS_LOWER(...)  NET_COAP_PROTO_LOWER(_pload_pos)(__VA_ARGS__)
#define NET_COAP_RECV_LOWER(...)       NET_COAP_PROTO_LOWER(_recv)(__VA_ARGS__)
#define NET_COAP_SEND_LOWER(...)       NET_COAP_PROTO_LOWER(_send)(__VA_ARGS__)


#define NET_COAP_OPTLEN(optlen) \
	1 + ((optlen >= 13) ? 1 : 0) + optlen

#define NET_COAP_PUT_OPTION(option_number, option_value, option_valuelen) \
	do { \
		options_delta = option_number - options_delta; \
		if (option_valuelen < 13) { \
			NET_PUT_BYTE(((options_delta & 0x0F) << 4) | \
			              (option_valuelen & 0x0F)); \
		} else if ((option_valuelen >= 13) && (option_valuelen < 269)) { \
			NET_PUT_BYTE(((options_delta & 0x0F) << 4) | 0x0D); \
			NET_PUT_BYTE(option_valuelen - 13); \
		} else { \
			NET_PUT_BYTE(((options_delta & 0x0F) << 4) | 0x0E); \
			NET_PUT_SHORT(option_valuelen - 269); \
		} \
		NET_PUT_DATA(option_value, option_valuelen); \
	} while (0)



/**
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Ver| T |  TKL  |      Code     |          Message ID           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Token (if any, TKL bytes) ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Options (if any) ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |1 1 1 1 1 1 1 1|    Payload (if any) ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define NET_COAP_BASEHDRSIZE 4



static uint16_t _net_coap_compute_hdrsize(struct net_coap_ctx *coap)
{
	uint16_t hdrsize = 0;
	uint16_t datalen = 0;
	uint8_t n = 0;

	hdrsize = NET_COAP_BASEHDRSIZE + coap->tokenlen;

	/* Compute uripath options length */
	if (coap->uripath != NULL) {
		for (n=0; n<coap->uripathcnt; n++) {
			datalen = strlen(coap->uripath[n]);
			hdrsize += NET_COAP_OPTLEN(datalen);
		}
	}

	/* Compute uriquery options length */
	if (coap->uriquery != NULL) {
		for (n=0; n<coap->uriquerycnt; n++) {
			datalen = strlen(coap->uriquery[n]);
			hdrsize += NET_COAP_OPTLEN(datalen);
		}
	}

	/* Compute contenttype options length */
	if (coap->contenttype != 0) {
		hdrsize += NET_COAP_OPTLEN(sizeof(uint8_t));
	}

	/* Trailing pre-payload byte */
	return hdrsize + 1;
}

int8_t net_coap_set_method(struct net_coap_ctx *coap, uint8_t type, uint8_t request_method)
{
	coap->type = type;
	coap->request_method = request_method;

	return NET_STATUS_OK;
}

int8_t net_coap_set_token(struct net_coap_ctx *coap, uint8_t tokenlen, uint8_t *token)
{
	coap->tokenlen = tokenlen;
	coap->token = token;

	return NET_STATUS_OK;
}

int8_t net_coap_set_uripath(struct net_coap_ctx *coap, uint8_t uripathcnt, char * const uripath[])
{
	/* Re-initialize header size */
	coap->hdrsize = 0;

	/* Do register the option */
	coap->uripath = uripath;
	coap->uripathcnt = uripathcnt;

	return NET_STATUS_OK;
}

int8_t net_coap_set_uriquery(struct net_coap_ctx *coap, uint8_t uriquerycnt, char * const uriquery[])
{
	/* Re-initialize header size */
	coap->hdrsize = 0;

	/* Do register the option */
	coap->uriquery = uriquery;
	coap->uriquerycnt = uriquerycnt;

	return NET_STATUS_OK;
}

int8_t net_coap_set_contenttype(struct net_coap_ctx *coap, uint8_t contenttype)
{
	/* If existence of contenttype changes, re-initialize header size */
	if ((contenttype == 0) != (coap->contenttype == 0)) {
		coap->hdrsize = 0;
	}

	/* Do register the option */
	coap->contenttype = contenttype;

	return NET_STATUS_OK;
}

uint8_t net_coap_get_responsecode(struct net_coap_ctx *coap)
{
	return coap->response_code;
}


int8_t net_coap_connect(struct net_coap_ctx *coap)
{
	return NET_STATUS_OK;
}

uint16_t net_coap_pload_pos(struct net_coap_ctx *coap)
{
	if (coap->hdrsize == 0) {
		coap->hdrsize = _net_coap_compute_hdrsize(coap);
	}

	return NET_COAP_PLOAD_POS_LOWER(coap->lower) + coap->hdrsize;
}

int8_t net_coap_recv(struct net_coap_ctx *coap, uint8_t *buffer, uint16_t buflen,
                     uint16_t *dataoffset, uint16_t *datalen)
{
	int8_t errno = 0;
	uint8_t *cursor = NULL;
	uint8_t *cursor_before = NULL;
	uint8_t vtt = 0;
	uint8_t type = 0;
	uint8_t *token = NULL;
	uint8_t tokenlen = 0;
	uint8_t response_code = 0;
	uint16_t messageid = 0;
	uint8_t opttypelen;
	uint16_t optlen = 0;

	/* Get the packet from the lower layer */
	errno = NET_COAP_RECV_LOWER(coap->lower, buffer, buflen, dataoffset, datalen);
	if (errno < 0) {
		goto out_zerodata;
	}

	/* Set the cursor to the position of the coap header in the buffer */
	NET_SET_CURSOR(buffer, *dataoffset);
	cursor_before = cursor;

	/* Check that buffer is big enough for coap header size */
	if (!NET_CHECK_BUFLEN(cursor_before, *datalen, NET_COAP_BASEHDRSIZE)) {
		errno = NET_EOVERFLOW;
		goto out_zerodata;
	}

	/* Read the Version + Type + Token Length fields */
	NET_GET_BYTE(vtt);

	/* Only version 1 is supported */
	if ((vtt >> 6) != NET_COAP_VERSION) {
		errno = NET_EPROTO;
		goto out_zerodata;
	}
	type = (vtt >> 4) & 0x03;
	tokenlen = vtt & 0x0F;

	/* Get the Code field */
	NET_GET_BYTE(response_code);

	/* Get the Message ID field */
	NET_GET_SHORT(messageid);

	/* Types acknowledge or reset */
	if (type & 0x02) {
		if ((messageid == coap->last_messageid) && (response_code == NET_COAP_CODE_EMPTY)) {
			/* This is a non-piggybacked acknowledgement */
			if (type == NET_COAP_TYPE_ACKNOWLEDGE) {
				coap->response_code = response_code;
				errno = NET_COAP_STATUS_ACK;
			} else {
				errno = NET_COAP_STATUS_RST;
			}
		} else {
			errno = NET_EAGAIN;
		}
		goto out_zerodata;
	} else if (type == NET_COAP_TYPE_NONCONFIRMABLE) {

		if (!NET_CHECK_BUFLEN(cursor_before, *datalen, tokenlen)) {
			errno = NET_EOVERFLOW;
			goto out_zerodata;
		}

		/* Make sure token length is the same as ours */
		if (tokenlen != coap->tokenlen) {
			errno = NET_EAGAIN;
			goto out_zerodata;
		}

		/* Read and compare the token */
		NET_GET_DATA(token, tokenlen);
		if (memcmp(token, coap->token, coap->tokenlen) != 0) {
			errno = NET_EAGAIN;
			goto out_zerodata;
		}

		*datalen -= (NET_COAP_BASEHDRSIZE + tokenlen);

		/* Skip options */
		while (*datalen > 0) {
			NET_GET_BYTE(opttypelen);

			/* Check whether we reached the end of options */
			if (opttypelen == 0xFF) {
				if (*datalen == 1) {
					/* Payload mark but zero data */
					errno = NET_EOVERFLOW;
					goto out_zerodata;
				}
				break;
			}

			/* Skip the option type field */
			switch (opttypelen & 0xF0) {
			case 0xD0:
				/* Skip 1 byte of option type/length + 1 bytes of extended option type */
				if (*datalen <= 1) {
					errno = NET_EOVERFLOW;
					goto out_zerodata;
				}
				NET_SKIP_DATA(1);
				*datalen -= 2;
				break;
			case 0xE0:
				/* Skip 1 byte of option type/length + 2 bytes of extended option type */
				if (*datalen <= 2) {
					errno = NET_EOVERFLOW;
					goto out_zerodata;
				}
				NET_SKIP_DATA(2);
				*datalen -= 3;
				break;
			case 0xF0:
				/* 0xFx is reserved for payload marker */
				errno = NET_EPROTO;
				goto out_zerodata;
			default:
				/* Skip 1 byte of option type/length */
				*datalen -= 1;
				break;
			}

			/* Get the option length field */
			optlen = (opttypelen & 0x0F);
			switch (optlen) {
			case 0x0D:
				/* Skip 1 bytes of option length */
				if (*datalen <= 1) {
					errno = NET_EOVERFLOW;
					goto out_zerodata;
				}
				NET_GET_BYTE(optlen);
				optlen += 13;
				*datalen -= 1;
				break;
			case 0x0E:
				/* Skip 2 bytes of option type */
				if (*datalen <= 2) {
					errno = NET_EOVERFLOW;
					goto out_zerodata;
				}
				NET_GET_SHORT(optlen);
				optlen += 269;
				*datalen -= 2;
				break;
			case 0x0F:
				/* 0xF0 is reserved for payload mark */
				errno = NET_EPROTO;
				goto out_zerodata;
			}

			if (*datalen < optlen) {
				errno = NET_EOVERFLOW;
				goto out_zerodata;
			}

			NET_SKIP_DATA(optlen);
			*datalen -= optlen;
		}

		/* Adjust the dataoffset */
		*dataoffset += (cursor - cursor_before);

		coap->response_code = response_code;
		errno = NET_STATUS_OK;
		goto out_data;
	} else {
		/* Confirmable message, we cannot send ack to these messages yet */
		errno = NET_EINVAL;
		goto out_zerodata;
	}

out_zerodata:
	*dataoffset = 0;
	*datalen = 0;
out_data:
	return errno;
}

int8_t net_coap_send(struct net_coap_ctx *coap, uint8_t *buffer, uint16_t buflen,
                     uint16_t dataoffset, uint16_t datalen)
{
	uint8_t *cursor = NULL;
	uint8_t header_pos = 0;
	uint16_t actual_hdrsize = 0;
	uint16_t messageid = ++coap->last_messageid;
	uint8_t options_delta = 0;
	uint8_t n = 0;

	coap->response_code = 0;

	/* Retrieve the start of header from lower layers */
	header_pos = NET_COAP_PLOAD_POS_LOWER(coap->lower);

	/* Set the cursor to the position of the coap header in the buffer */
	NET_SET_CURSOR(buffer, header_pos);

	/* Calculate the header size if not already done */
	if (coap->hdrsize == 0) {
		coap->hdrsize = _net_coap_compute_hdrsize(coap);
	}

	/* The trailing 0xFF byte is added only if payload exists */
	actual_hdrsize = coap->hdrsize - ((datalen > 0) ? 0 : 1);

	/* Check that dataoffset is big enough for coap header size */
	NET_CHECK_BUFLEN(buffer, dataoffset, coap->hdrsize);

	/* Put the Version + Type + Token Length fields */
	NET_PUT_BYTE((NET_COAP_VERSION << 6) |
	             ((coap->type & 0x03) << 4) |
	             (coap->tokenlen & 0x0F));

	/* Put the Code field */
	NET_PUT_BYTE(coap->request_method);

	/* Put the Message ID field */
	NET_PUT_SHORT(messageid);

	/* Put token field */
	NET_PUT_DATA(coap->token, coap->tokenlen);

	/* Put Uri-path options, if provided */
	if (coap->uripath != NULL) {
		for (n=0; n<coap->uripathcnt; n++) {
			NET_COAP_PUT_OPTION(NET_COAP_OPTION_URIPATH,
			                    coap->uripath[n], strlen(coap->uripath[n]));
		}
	}

	/* Put Content-Type option, if provided */
	if (coap->contenttype != 0) {
		NET_COAP_PUT_OPTION(NET_COAP_OPTION_CONTENTFORMAT, &coap->contenttype, 1);
	}

	/* Put Uri-query options, if provided */
	if (coap->uriquery != NULL) {
		for (n=0; n<coap->uriquerycnt; n++) {
			NET_COAP_PUT_OPTION(NET_COAP_OPTION_URIQUERY,
			                    coap->uriquery[n], strlen(coap->uriquery[n]));
		}
	}

	if (datalen > 0) {
		/* Put static field */
		NET_PUT_BYTE(0xFF);

		/* Note: we assume that the caller already placed the payload at this position */
	}

	/* Pass to the lower layer */
	return NET_COAP_SEND_LOWER(coap->lower, buffer, buflen,
	                           header_pos, actual_hdrsize + datalen);
}
