/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PROTO_COAP_H
#define _PROTO_COAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NET_COAP_STATUS_ACK          1
#define NET_COAP_STATUS_RST          2


#define NET_COAP_VERSION                       0x01

#define NET_COAP_TYPE_CONFIRMABLE              0x00
#define NET_COAP_TYPE_NONCONFIRMABLE           0x01
#define NET_COAP_TYPE_ACKNOWLEDGE              0x02
#define NET_COAP_TYPE_RESET                    0x03

#define NET_COAP_CODE_EMPTY                    0x00
#define NET_COAP_CODE_GET                      0x01
#define NET_COAP_CODE_POST                     0x02
#define NET_COAP_CODE_PUT                      0x03
#define NET_COAP_CODE_DELETE                   0x04
#define NET_COAP_CODE_CREATED                  0x41
#define NET_COAP_CODE_DELETED                  0x42
#define NET_COAP_CODE_VALID                    0x43
#define NET_COAP_CODE_CHANGED                  0x44
#define NET_COAP_CODE_CONTENT                  0x45
#define NET_COAP_CODE_BADREQUEST               0x80
#define NET_COAP_CODE_UNAUTHORIZED             0x81
#define NET_COAP_CODE_BADOPTION                0x82
#define NET_COAP_CODE_FORBIDDEN                0x83
#define NET_COAP_CODE_NOTFOUND                 0x84
#define NET_COAP_CODE_METHODNOTALLOWED         0x85
#define NET_COAP_CODE_NOTACCEPTABLE            0x86
#define NET_COAP_CODE_PRECONDITIONFAILED       0x8C
#define NET_COAP_CODE_REQUESTENTITYTOOLARGE    0x8D
#define NET_COAP_CODE_UNSUPPORTEDCONTENTFORMAT 0x8F
#define NET_COAP_CODE_INTERNALSERVERERROR      0xA0
#define NET_COAP_CODE_NOTIMPLEMENTED           0xA1
#define NET_COAP_CODE_BADGATEWAY               0xA2
#define NET_COAP_CODE_SERVICEUNAVAILABLE       0xA3
#define NET_COAP_CODE_GATEWAYTIMEOUT           0xA4
#define NET_COAP_CODE_PROXYINGNOTSUPPORTED     0xA5


#define NET_COAP_OPTION_URIPATH           11
#define NET_COAP_OPTION_CONTENTFORMAT     12
#define NET_COAP_OPTION_URIQUERY          15

#define NET_COAP_CONTENTTYPE_TEXTPLAIN    0
#define NET_COAP_CONTENTTYPE_LINKFORMAT   40
#define NET_COAP_CONTENTTYPE_XML          41
#define NET_COAP_CONTENTTYPE_OCTETSTRING  42
#define NET_COAP_CONTENTTYPE_EXI          47
#define NET_COAP_CONTENTTYPE_JSON         50



struct net_coap_ctx {
	uint16_t hdrsize;
	uint8_t type;
	uint8_t request_method;
	uint8_t response_code;
	uint8_t tokenlen;
	uint8_t * token;
	uint16_t last_messageid;
	uint8_t contenttype;
	char * const *uripath;
	uint8_t uripathcnt;
	char * const *uriquery;
	uint8_t uriquerycnt;

	struct NET_COAP_PROTO_LOWER(_ctx) *lower;
};

extern int8_t net_coap_set_method(struct net_coap_ctx *coap, uint8_t type, uint8_t request_method);
extern int8_t net_coap_set_token(struct net_coap_ctx *coap, uint8_t tokenlen, uint8_t *token);
extern int8_t net_coap_set_uripath(struct net_coap_ctx *coap, uint8_t uripathcnt, char * const uripath[]);
extern int8_t net_coap_set_uriquery(struct net_coap_ctx *coap, uint8_t uriquerycnt, char * const uriquery[]);
extern int8_t net_coap_set_contenttype(struct net_coap_ctx *coap, uint8_t contenttype);

extern uint8_t net_coap_get_responsecode(struct net_coap_ctx *coap);

extern int8_t net_coap_connect(struct net_coap_ctx *coap);
extern uint16_t net_coap_pload_pos(struct net_coap_ctx *coap);
extern int8_t net_coap_recv(struct net_coap_ctx *coap, uint8_t *buffer, uint16_t buflen,
                            uint16_t *dataoffset, uint16_t *datalen);
extern int8_t net_coap_send(struct net_coap_ctx *coap, uint8_t *buffer, uint16_t buflen,
                            uint16_t dataoffset, uint16_t datalen);


#ifdef __cplusplus
}
#endif

#endif
