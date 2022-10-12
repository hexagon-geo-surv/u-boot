/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 * Copyright 2022 Leica Geosystems AG
 */

#ifndef __BELUGA_H
#define __BELUGA_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/

#define CONFIG_POWER_PCA9450

#endif

#if defined(CONFIG_CMD_NET)
#define CONFIG_FEC_MXC_PHYADDR          1

#define PHY_ANEG_TIMEOUT 20000

#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"console=ttymxc1,115200 earlycon=ec_imx6q,0x30890000,115200\0" \
	"initrd_addr=0x43800000\0"		\
	"bootm_size=0x10000000\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"bootscr_addr_blk="__stringify(CONFIG_LEICA_BOOTSCRIPT_EMMC_ADDR)"\0" \
	"bootscr_size_blk="__stringify(CONFIG_LEICA_BOOTSCRIPT_EMMC_SIZE)"\0" \
	"loadbootscript=" \
		"mmc partconf ${mmcdev} boot_partition; " \
		"if test \"${boot_partition}\" = \"1\"; then " \
			"echo Loading bootscript from boot0; " \
			"mmc dev ${mmcdev} 1; " \
		"elif test \"${boot_partition}\" = \"2\"; then " \
			"echo Loading bootscript from boot1; " \
			"mmc dev ${mmcdev} 2; " \
		"else " \
			"echo Loading bootscript from user partition; " \
		"fi; " \
		"mmc read ${scriptaddr} ${bootscr_addr_blk} ${bootscr_size_blk};" \
		"mmc dev ${mmcdev};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"swu=1\0"

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; "\
	"if mmc rescan; then " \
		"if run loadbootscript && run bootscript; then " \
			"echo Bootscript finished; " \
		"fi; " \
	"fi;"

/* Link Definitions */

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x80000


/* Totally 6GB DDR */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0xC0000000	/* 3 GB */
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0xC0000000	/* 3 GB */

#endif
