// SPDX-License-Identifier: GPL-2.0+
// Copyright (C) 2022 Leica Geosystems AG
#pragma once

#define SEP_RESPONSE_LENGTH  128
#define SEP_CMD_LENGTH        16

int leica_sep_init(struct udevice **dev);
int leica_sep_transfer(char *cmd,
		       char *data, size_t data_size,
		       ulong timeout_ms);
