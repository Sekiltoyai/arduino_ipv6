/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define NET_STATUS_OK           0

#define NET_EAGAIN              -1  /* Try again */
#define NET_ENOMEM              -2  /* Not enough memory */
#define NET_EOVERFLOW           -3  /* Buffer overflow */
#define NET_EINVAL              -4  /* Invalid data */
#define NET_EPROTO              -5  /* Protocol error */
#define NET_ECONFIG             -6  /* Invalid configuration */


#ifdef __cplusplus
}
#endif

#endif
