// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright(C) 2022 Leica Geosystems AG
 */

#include <common.h>
#include <command.h>
#include <console.h>

struct bootmap_switch {
	u32 active_addr;
	u32 standby_addr;
	u32 size;
} __packed;

static int do_get_active_bootmap(struct cmd_tbl *cmdtp, int flag, int argc,
				 char * const argv[])
{
	void *addr;
	struct bootmap_switch *bms;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	if (!addr) {
		printf("Bootmap-Switch address is NULL!\n");
		return CMD_RET_USAGE;
	}

	bms = (struct bootmap_switch *) addr;

	bms->active_addr = be32_to_cpu(bms->active_addr);
	bms->standby_addr = be32_to_cpu(bms->standby_addr);
	bms->size = be32_to_cpu(bms->size);

	if (bms->active_addr % 512 ||
	    bms->standby_addr % 512 ||
	    bms->size % 512)
	{
		printf("Bootmap not aligned!\n");
		return CMD_RET_FAILURE;
	}

	printf(" active address: 0x%08x\n", bms->active_addr);
	printf("standby address: 0x%08x\n", bms->standby_addr);
	printf("  one copy size: 0x%08x\n", bms->size);

	env_set_hex("bootmap.active_addr", bms->active_addr);
	env_set_hex("bootmap.active_blk", bms->active_addr >> 9);
	env_set_hex("bootmap.standby_addr", bms->standby_addr);
	env_set_hex("bootmap.standby_blk", bms->standby_addr >> 9);
	env_set_hex("bootmap.size", bms->size);
	env_set_hex("bootmap.cnt", bms->size >> 9);

	return CMD_RET_SUCCESS;
}

#if CONFIG_IS_ENABLED(SYS_LONGHELP)
static char bootmap_help_text[] =
	"bootmap <address>"
	"    <address>        Address of bootmap-switch (hex)\n"
	"Variables added to environment:\n"
	"    bootmap.address  Address of bootmap (hex)\n"
	"    bootmap.blk      Block of bootmap (hex)\n"
	"    bootmap.size     Size of bootmap (hex)\n"
	"    bootmap.cnt      Size of bootmap in blocks (hex)"
	;
#endif

U_BOOT_CMD(
	bootmap, 2, 0, do_get_active_bootmap,
	"Add address and size of Leica System 1600 bootmap to environment.",
	bootmap_help_text
);

