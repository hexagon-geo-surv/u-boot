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

#define CONFIG_SYS_BOOTM_LEN		(32 * SZ_1M)

#define CONFIG_SPL_MAX_SIZE		(152 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/
#define CONFIG_SPL_STACK		0x960000
#define CONFIG_SPL_BSS_START_ADDR	0x0098FC00
#define CONFIG_SPL_BSS_MAX_SIZE		0x400	/* 1 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K	/* 512 KB */

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#define CONFIG_POWER_PCA9450

#endif

#if defined(CONFIG_CMD_NET)
#define CONFIG_FEC_MXC_PHYADDR          1

#define DWC_NET_PHYADDR			1

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
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)


/* Totally 6GB DDR */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0xC0000000	/* 3 GB */
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0xC0000000	/* 3 GB */

#define CONFIG_MXC_UART_BASE		UART_BASE_ADDR(2)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#endif
