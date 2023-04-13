# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (c) 2019 bytes at work AG. All rights reserved.
#
# based on stm32mp1/Makefile:
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y += stm32mp1.o
obj-$(CONFIG_CMD_HWID) += hwid.o
obj-y += ../baw_config/
endif

obj-y += board.o

obj-$(CONFIG_SYS_MTDPARTS_RUNTIME) += ../../st/common/stm32mp_mtdparts.o
