/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 * Copyright 2022 Leica Geosystems AG
 */

#ifndef __CROCODILE_H
#define __CROCODILE_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_BOOTM_LEN		(64 * SZ_1M)
#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define UBOOT_ITB_OFFSET		0x57C00
#define QSPI0_AMBA_BASE			0x08000000
#define FSPI_CONF_BLOCK_SIZE		0x1000
#define UBOOT_ITB_OFFSET_FSPI	\
	(UBOOT_ITB_OFFSET + FSPI_CONF_BLOCK_SIZE)
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + UBOOT_ITB_OFFSET_FSPI)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_STACK		0x920000
#define CONFIG_SPL_BSS_START_ADDR	0x910000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K	/* 8 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K	/* 512 KB */

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc2,115200\0" \
	"initrd_addr=0x43800000\0" \
	"loadaddr=0x40480000\0"    \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"bootscr_addr_blk="__stringify(CONFIG_LEICA_BOOTSCRIPT_EMMC_ADDR)"\0" \
	"bootscr_size_blk="__stringify(CONFIG_LEICA_BOOTSCRIPT_EMMC_SIZE)"\0" \
	"bootm_size=0x10000000\0" \
	"board_id_addr=0x206\0" \
	"loadbootscript="\
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
		"mmc read ${loadaddr} ${bootscr_addr_blk} ${bootscr_size_blk};" \
		"mmc dev ${mmcdev};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"bootqspi=echo Booting from qspi ...;" \
		"nor_img_addr="__stringify(CONFIG_LEICA_FSPI_NOR_IMG_ADDR)"; " \
		"nor_img_size="__stringify(CONFIG_LEICA_FSPI_NOR_IMG_SIZE)"; " \
		"sf read ${initrd_addr} ${nor_img_addr} ${nor_img_size}; "\
		"bootm ${initrd_addr}#${fit_config};\0" \
	"get_fit_config=sf probe;" \
		"mw.l ${loadaddr} 0x0 0x1;" \
		"sf read ${loadaddr} ${board_id_addr} 0x1;" \
		"env set fit_config conf-freescale_crocodile-pt1.dtb;" \
		"if itest.s *${loadaddr} -eq A; then" \
		"    echo \"Board A - select config conf-freescale_crocodile-pt1.dtb\";" \
		"else" \
		"    echo \"Board ID unknown - falling back to default config conf-freescale_crocodile-pt1.dtb\";" \
		"fi;\0" \
	"swu=1\0" \

#ifndef CONFIG_LEICA_FSPI_NOR_BOOTFLOW
#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; "\
	"if mmc rescan; then " \
		"if run loadbootscript && run bootscript; then " \
			"echo Bootscript finished; " \
		"fi; " \
	"fi;"
#else
#define CONFIG_BOOTCOMMAND \
	"run get_fit_config;" \
	"run bootqspi;"
#endif

/* Link Definitions */

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        0x200000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_MMCROOT			"/dev/mmcblk1p2"  /* USDHC2 */

#define CONFIG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			0x40000000 /* 1GB DDR */

#define CONFIG_MXC_UART_BASE		UART3_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_FEC_MXC_PHYADDR          0
#define FEC_QUIRK_ENET_MAC

#endif
