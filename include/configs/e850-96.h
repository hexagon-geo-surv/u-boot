/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020, Linaro Limited
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Configuration for E850-96 board.
 */

#ifndef __E850_96_H
#define __E850_96_H

/* GUIDs for capsule updatable firmware images */
#define E850_96_FWBL1_IMAGE_GUID \
	EFI_GUID(0x181cd3f2, 0xe375, 0x44d2, 0x80, 0x78, \
		 0x32, 0x21, 0xe1, 0xdf, 0xb9, 0x5e)

#define E850_96_EPBL_IMAGE_GUID \
	EFI_GUID(0x66c1a54d, 0xd149, 0x415d, 0xaa, 0xda, \
		 0xb8, 0xae, 0xe4, 0x99, 0xb3, 0x70)

#define E850_96_BL2_IMAGE_GUID \
	EFI_GUID(0x89471c2a, 0x6c8d, 0x4158, 0xac, 0xad, \
		 0x23, 0xd3, 0xb2, 0x87, 0x3d, 0x35)

#define E850_96_BOOTLOADER_IMAGE_GUID \
	EFI_GUID(0x629578c3, 0xffb3, 0x4a89, 0xac, 0x0c, \
		 0x61, 0x18, 0x40, 0x72, 0x77, 0x79)

#define E850_96_EL3_MON_IMAGE_GUID \
	EFI_GUID(0xdf5718a2, 0x930a, 0x4916, 0xbb, 0x19, \
		 0x32, 0x13, 0x21, 0x4d, 0x84, 0x86)

#endif /* __E850_96_H */
