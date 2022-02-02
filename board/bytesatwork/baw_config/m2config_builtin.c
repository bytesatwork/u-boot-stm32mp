// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include "m2config.h"
#include <config.h>

int m2config_builtin(struct m2config *config)
{
	config->pcb    = M2CONFIG_BUILTIN_PCB;
	config->ram    = M2CONFIG_BUILTIN_RAM;
	config->flash  = M2CONFIG_BUILTIN_FLASH;
	return 0;
}
