#
# Copyright 2019 NXP
# Copyright 2022 Leica Geosystems AG
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += beluga.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
endif
