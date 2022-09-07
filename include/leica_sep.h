// SPDX-License-Identifier: GPL-2.0+
// Copyright (C) 2022 Leica Geosystems AG
#ifndef _LEICA_SEP_H_
#define _LEICA_SEP_H_

#define SEP_RESPONSE_LENGTH      64
#define SEP_CMD_LENGTH           16
#define SEP_RESPONSE_TIMEOUT_MS 200

int leica_sep_init(struct udevice **dev);
int leica_sep_transfer(struct udevice **dev,
                       char *cmd,
                       char *data, size_t data_size,
                       ulong timeout_ms);

#endif // _LEICA_SEP_H_
