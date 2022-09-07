// SPDX-License-Identifier: GPL-2.0+
// Copyright (C) 2022 Leica Geosystems AG

#include <command.h>
#include <common.h>
#include <console.h>
#include <dm.h>
#include <leica_sep.h>
#include <serial.h>

#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mm_pins.h>


/*
 * global/"singleton" to do the uart initialization on demand and
 * exactly once - even across multiple calls/callers in one session
 */
static struct udevice **leica_sep_udevice;


/*
 * commandline argument handeling wrapper around
 * 'leica_sep_init' and 'leica_sep_transfer'
 */
static int do_leica_sep(struct cmd_tbl *cmdtp, int flag, int argc,
                        char * const argv[])
{
    int ret = 0;
    char* command;
    char data[SEP_RESPONSE_LENGTH];
    bool verbose = false;

    if (argc < 2) {
        return CMD_RET_USAGE;
    } else if (argc == 2) {
        command = argv[1];
    } else if (argc == 3) {
        if (!strcmp(argv[1], "-v")) {
            verbose = true;
        } else {
            return CMD_RET_USAGE;
        }
        command = argv[2];
    }

    /*
     *NOTE: there is no "udevice (pointer) valid"; only burried in other
     * functions like ofnode_get_name... which returns NULL should the
     * udevice pointer be uninitialized; abusing that to implement a
     * "do once only" setup = set baudrate, software reset, ...
     */
    if (NULL == dev_read_name(*leica_sep_udevice)) {
        ret = leica_sep_init(leica_sep_udevice);
        if (ret) {
            log_err("%s: unable to initialize serial driver used for SEP command\n", __FUNCTION__);
            return -ENOTTY;
        }
    }

    ret = leica_sep_transfer(leica_sep_udevice,
                             command,
                             data, sizeof(data),
                             SEP_RESPONSE_TIMEOUT_MS);

    if (verbose)
        printf("SEP command = %s: ", command);
    if (ret < 0) {
        if (verbose)
            printf("error during sep_transfer: %i\n", ret);
        return CMD_RET_FAILURE;
    } else if (ret == 0) {
        if (verbose)
            printf("Ok\n");
        return CMD_RET_SUCCESS;
    } else {
        if (verbose)
            printf("response = %s (%i bytes)\n", data, ret);
        else
            printf("%s\n", data);
        return CMD_RET_SUCCESS;
    }
}

U_BOOT_CMD(
           leica_sep,      3,      1,      do_leica_sep,
           "send a 'SEP' query to the coprocessor",
           "[-v] <SEP-command>"
           );
