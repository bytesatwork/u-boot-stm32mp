// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include "baw_config.h"
#include <config.h>

int baw_config_builtin(struct baw_config *config)
{
	config->pcb    = BAW_CONFIG_BUILTIN_PCB;
	config->ram    = BAW_CONFIG_BUILTIN_RAM;
	config->flash  = BAW_CONFIG_BUILTIN_FLASH;
	return 0;
}
