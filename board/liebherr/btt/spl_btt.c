// SPDX-License-Identifier: GPL-2.0+
/*
 * DENX M28 Boot setup
 *
 * Copyright (C) 2025 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

#define MUX_CONFIG_LCD (MXS_PAD_3V3 | MXS_PAD_4MA | MXS_PAD_NOPULL)
#define MUX_CONFIG_BOOT (MXS_PAD_3V3)
#define MUX_CONFIG_TSC (MXS_PAD_3V3 | MXS_PAD_8MA | MXS_PAD_PULLUP)
#define MUX_CONFIG_SSP0 (MXS_PAD_3V3 | MXS_PAD_12MA | MXS_PAD_PULLUP)
#define MUX_CONFIG_SSP2 (MXS_PAD_3V3 | MXS_PAD_4MA | MXS_PAD_PULLUP)
#define MUX_CONFIG_ENET (MXS_PAD_3V3 | MXS_PAD_8MA | MXS_PAD_NOPULL)
#define MUX_CONFIG_EMI (MXS_PAD_3V3 | MXS_PAD_12MA | MXS_PAD_NOPULL)

/* HW_PINCTRL_EMI_DS_CTRL */
#define EMI_DS_CTRL_CLR 0x3FFF
#define EMI_DS_CTRL_SLICE0_SHIFT 0
#define EMI_DS_CTRL_SLICE1_SHIFT 2
#define EMI_DS_CTRL_SLICE2_SHIFT 4
#define EMI_DS_CTRL_SLICE3_SHIFT 6
#define EMI_DS_CTRL_DUALPAD_SHIFT 8
#define EMI_DS_CTRL_CONTROL_SHIFT 10
#define EMI_DS_CTRL_ADDRESS_SHIFT 12
#define EMI_DS_CTRL_MA_20 0x10

/*
 * The BTT devide can be boot in several ways; U-Boot's 'falcon' mode,
 * normal boot (through U-Boot proper) and also via recovery system.
 *
 * To fix some HW issues on the device, as well as providing all the
 * above boot method's the same "start" state when kernel starts it
 * has been decided to configure PINMUXes for all relevant IP blocks
 * (and GPIOs during early SPI state).
 *
 * It also shall be mentioned, that during early SPL stage, the BTT
 * device needs to cooperate with uC based co-processor - this
 * requires correct pins configuration.
 */
static const iomux_cfg_t iomux_setup[] = {
	/* AUART0 IRDA */
	MX28_PAD_AUART0_RX__AUART0_RX,
	MX28_PAD_AUART0_TX__AUART0_TX,

	/* AUART 4 RS422 */
	MX28_PAD_AUART0_CTS__AUART4_RX,
	MX28_PAD_AUART0_RTS__AUART4_TX,

	/* USB0 */
	MX28_PAD_AUART1_CTS__USB0_OVERCURRENT,
	MX28_PAD_AUART1_RTS__USB0_ID,
	MX28_PAD_LCD_VSYNC__GPIO_1_28, /* PRW_On */

	/* USB1 */
	MX28_PAD_PWM2__USB1_OVERCURRENT,

	/* eMMC */
	MX28_PAD_SSP0_CMD__SSP0_CMD | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA0__SSP0_D0 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA1__SSP0_D1 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA2__SSP0_D2 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA3__SSP0_D3 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA4__SSP0_D4 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA5__SSP0_D5 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA6__SSP0_D6 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DATA7__SSP0_D7 | MUX_CONFIG_SSP0,
	MX28_PAD_SSP0_DETECT__GPIO_2_9, /* Reset for eMMC */
	MX28_PAD_SSP0_SCK__SSP0_SCK | MUX_CONFIG_SSP0,

	/* DIG Keys */
	MX28_PAD_GPMI_D00__GPIO_0_0,
	MX28_PAD_GPMI_D01__GPIO_0_1,
	MX28_PAD_GPMI_D02__GPIO_0_2,
	MX28_PAD_GPMI_D03__GPIO_0_3,
	MX28_PAD_GPMI_D04__GPIO_0_4,
	MX28_PAD_GPMI_D05__GPIO_0_5,
	MX28_PAD_GPMI_D06__GPIO_0_6,
	MX28_PAD_GPMI_D07__GPIO_0_7,

	/* ADR_0-2 */
	MX28_PAD_GPMI_CE1N__GPIO_0_17,
	MX28_PAD_GPMI_CE2N__GPIO_0_18,
	MX28_PAD_GPMI_CE3N__GPIO_0_19,

	/* Read Keys */
	MX28_PAD_GPMI_RDY0__GPIO_0_20,

	/* LATCH_EN */
	MX28_PAD_GPMI_RDY1__GPIO_0_21,

	/* Power off */
	MX28_PAD_GPMI_RDN__GPIO_0_24,

	/* WIFI EN */
	MX28_PAD_GPMI_CLE__GPIO_0_27,

	/* I2C1 Touch */
	MX28_PAD_AUART2_CTS__GPIO_3_10,
	MX28_PAD_AUART2_RTS__GPIO_3_11,
	MX28_PAD_GPMI_RDY2__GPIO_0_22, /* Touch Reset */
	MX28_PAD_GPMI_RDY3__GPIO_0_23, /* Touch INT */

	/* TIVA */
	MX28_PAD_AUART1_RX__SSP2_CARD_DETECT,
	MX28_PAD_SSP2_MISO__SSP2_D0,
	MX28_PAD_SSP2_MOSI__SSP2_CMD,
	MX28_PAD_SSP2_SCK__SSP2_SCK,
	MX28_PAD_SSP2_SS0__SSP2_D3,
	MX28_PAD_SSP2_SS1__GPIO_2_20,
	MX28_PAD_SSP2_SS2__GPIO_2_21,

	/* SPI3 NOR-Flash */
	MX28_PAD_AUART1_TX__SSP3_CARD_DETECT,
	MX28_PAD_AUART2_RX__SSP3_D1,
	MX28_PAD_AUART2_TX__SSP3_D2,
	MX28_PAD_SSP3_MISO__SSP3_D0,
	MX28_PAD_SSP3_MOSI__SSP3_CMD,
	MX28_PAD_SSP3_SCK__SSP3_SCK,
	MX28_PAD_SSP3_SS0__SSP3_D3,

	/* NOR-Flash CMD */
	MX28_PAD_LCD_RS__GPIO_1_26, /* Hold */
	MX28_PAD_LCD_WR_RWN__GPIO_1_25, /* write protect */

	/* I2C0 Codec */
	MX28_PAD_I2C0_SCL__I2C0_SCL,
	MX28_PAD_I2C0_SDA__I2C0_SDA,

	/* I2S Codec */
	MX28_PAD_SAIF0_BITCLK__SAIF0_BITCLK,
	MX28_PAD_SAIF0_LRCLK__SAIF0_LRCLK,
	MX28_PAD_SAIF0_MCLK__SAIF0_MCLK,
	MX28_PAD_SAIF0_SDATA0__SAIF0_SDATA0,
	MX28_PAD_SAIF1_SDATA0__SAIF1_SDATA0,

	/* BT_EN */
	MX28_PAD_SPDIF__GPIO_3_27,

	/* EMI */
	MX28_PAD_EMI_D00__EMI_DATA0 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D01__EMI_DATA1 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D02__EMI_DATA2 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D03__EMI_DATA3 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D04__EMI_DATA4 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D05__EMI_DATA5 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D06__EMI_DATA6 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D07__EMI_DATA7 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D08__EMI_DATA8 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D09__EMI_DATA9 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D10__EMI_DATA10 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D11__EMI_DATA11 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D12__EMI_DATA12 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D13__EMI_DATA13 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D14__EMI_DATA14 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_D15__EMI_DATA15 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_ODT0__EMI_ODT0 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_DQM0__EMI_DQM0 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_ODT1__EMI_ODT1 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_DQM1__EMI_DQM1 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_DDR_OPEN_FB__EMI_DDR_OPEN_FEEDBACK | MUX_CONFIG_EMI,
	MX28_PAD_EMI_CLK__EMI_CLK | MUX_CONFIG_EMI,
	MX28_PAD_EMI_DQS0__EMI_DQS0 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_DQS1__EMI_DQS1 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_DDR_OPEN__EMI_DDR_OPEN | MUX_CONFIG_EMI,

	MX28_PAD_EMI_A00__EMI_ADDR0 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A01__EMI_ADDR1 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A02__EMI_ADDR2 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A03__EMI_ADDR3 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A04__EMI_ADDR4 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A05__EMI_ADDR5 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A06__EMI_ADDR6 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A07__EMI_ADDR7 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A08__EMI_ADDR8 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A09__EMI_ADDR9 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A10__EMI_ADDR10 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A11__EMI_ADDR11 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A12__EMI_ADDR12 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A13__EMI_ADDR13 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_A14__EMI_ADDR14 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_BA0__EMI_BA0 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_BA1__EMI_BA1 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_BA2__EMI_BA2 | MUX_CONFIG_EMI,
	MX28_PAD_EMI_CASN__EMI_CASN | MUX_CONFIG_EMI,
	MX28_PAD_EMI_RASN__EMI_RASN | MUX_CONFIG_EMI,
	MX28_PAD_EMI_WEN__EMI_WEN | MUX_CONFIG_EMI,
	MX28_PAD_EMI_CE0N__EMI_CE0N | MUX_CONFIG_EMI,
	MX28_PAD_EMI_CE1N__EMI_CE1N | MUX_CONFIG_EMI,
	MX28_PAD_EMI_CKE__EMI_CKE | MUX_CONFIG_EMI,

	/* Uart3 Bluetooth-Interface */
	MX28_PAD_AUART3_CTS__AUART3_CTS,
	MX28_PAD_AUART3_RTS__AUART3_RTS,
	MX28_PAD_AUART3_RX__AUART3_RX,
	MX28_PAD_AUART3_TX__AUART3_TX,

	/* framebuffer */
	MX28_PAD_LCD_CS__LCD_CS | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D00__LCD_D0 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D01__LCD_D1 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D02__LCD_D2 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D03__LCD_D3 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D04__LCD_D4 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D05__LCD_D5 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D06__LCD_D6 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D07__LCD_D7 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D08__LCD_D8 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D09__LCD_D9 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D10__LCD_D10 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D11__LCD_D11 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D12__LCD_D12 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D13__LCD_D13 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D14__LCD_D14 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D15__LCD_D15 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D16__LCD_D16 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D17__LCD_D17 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D18__LCD_D18 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D19__LCD_D19 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D20__LCD_D20 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D21__LCD_D21 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D22__LCD_D22 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_D23__LCD_D23 | MUX_CONFIG_LCD,
	MX28_PAD_LCD_DOTCLK__LCD_DOTCLK | MUX_CONFIG_LCD,
	MX28_PAD_LCD_ENABLE__LCD_ENABLE | MUX_CONFIG_LCD,
	MX28_PAD_LCD_HSYNC__LCD_HSYNC | MUX_CONFIG_LCD,
	MX28_PAD_LCD_RD_E__LCD_VSYNC | MUX_CONFIG_LCD,
	MX28_PAD_LCD_RESET__LCD_RESET | MUX_CONFIG_LCD,

	/* DUART RS232 */
	MX28_PAD_PWM0__DUART_RX,
	MX28_PAD_PWM1__DUART_TX,

	/* Backlight */
	MX28_PAD_PWM3__PWM_3,

	/* FEC Ethernet */
	MX28_PAD_ENET_CLK__CLKCTRL_ENET | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_COL__ENET1_TX_EN | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_CRS__ENET1_RX_EN | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_MDC__ENET0_MDC | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_MDIO__ENET0_MDIO | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_RX_CLK__GPIO_4_13, /* Phy Interrupt */
	MX28_PAD_ENET0_RX_EN__ENET0_RX_EN | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_RXD0__ENET0_RXD0 | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_RXD1__ENET0_RXD1 | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_RXD3__ENET1_RXD1 | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_TX_CLK__GPIO_4_5, /* n.c. */
	MX28_PAD_ENET0_TX_EN__ENET0_TX_EN | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_TXD0__ENET0_TXD0 | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_TXD1__ENET0_TXD1 | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_TXD3__ENET1_TXD1 | MUX_CONFIG_ENET,
	MX28_PAD_ENET0_TXD3__GPIO_4_12, /* PHY reset */

	/* boot/rescue pins */
	MX28_PAD_ENET0_RXD2__GPIO_4_9,
	MX28_PAD_ENET0_TXD2__GPIO_4_11,

	/* HW revision setup pins - by default pullup DISABLED */
	MX28_PAD_ENET0_RXD3__GPIO_4_10,
	MX28_PAD_ENET0_TX_CLK__GPIO_4_5,
	MX28_PAD_ENET0_COL__GPIO_4_14,
	MX28_PAD_ENET0_CRS__GPIO_4_15,
};

u32 mxs_dram_vals[] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000100, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00010101, 0x01010101,
	0x000f0f01, 0x0f02010a, 0x00000000, 0x00010101,
	0x00000100, 0x00000100, 0x00000000, 0x00000002,
	0x01010000, 0x07080403, 0x07005303, 0x0b0000c8,
	0x0200a0c1, 0x0002040c, 0x0038430a, 0x04290322,
	0x02040203, 0x00c8002b, 0x00000000, 0x00000000,
	0x00012100, 0xffff0303, 0x00012100, 0xffff0303,
	0x00012100, 0xffff0303, 0x00012100, 0xffff0303,
	0x00000003, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000612, 0x01000102,
	0x06120612, 0x00000200, 0x00020007, 0xf4004a27,
	0xf4004a27, 0xf4004a27, 0xf4004a27, 0x07400300,
	0x07400300, 0x07400300, 0x07400300, 0x00000005,
	0x00000000, 0x00000000, 0x01000000, 0x00000000,
	0x00000001, 0x000f1133, 0x00000000, 0x00001f04,
	0x00001f04, 0x00001f04, 0x00001f04, 0x00001f04,
	0x00001f04, 0x00001f04, 0x00001f04, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00010000, 0x00030404,
	0x00000002, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x01010000,
	0x01000000, 0x03030000, 0x00010303, 0x01020202,
	0x00000000, 0x02040101, 0x21002103, 0x00061200,
	0x06120612, 0x00000642, 0x00000000, 0x00000004,
	0x00000000, 0x00000080, 0x00000000, 0x00000000,
	0x00000000, 0xffffffff
};

#ifndef CONFIG_SPL_FRAMEWORK
void board_init_ll(const u32 arg, const uint32_t *resptr)
{
	mxs_common_spl_init(arg, resptr, iomux_setup, ARRAY_SIZE(iomux_setup));
}
#else
void lowlevel_init(void)
{
	struct mxs_pinctrl_regs *pinctrl_regs =
		(struct mxs_pinctrl_regs *)MXS_PINCTRL_BASE;

	/* Set EMI drive strength - the HW_PINCTRL_EMI_DS_CTRL */
	writel(EMI_DS_CTRL_CLR, &pinctrl_regs->hw_pinctrl_emi_ds_ctrl_clr);
	writel(EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_SLICE0_SHIFT |
	       EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_SLICE1_SHIFT |
	       EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_SLICE2_SHIFT |
	       EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_SLICE3_SHIFT |
	       EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_DUALPAD_SHIFT |
	       EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_CONTROL_SHIFT |
	       EMI_DS_CTRL_MA_20 << EMI_DS_CTRL_ADDRESS_SHIFT,
	       &pinctrl_regs->hw_pinctrl_emi_ds_ctrl_set);

	mxs_common_spl_init(0, NULL, iomux_setup, ARRAY_SIZE(iomux_setup));
}
#endif
