// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include <asm/arch/ddr_defs.h>
#include <i2c.h>
#include "m2config_get.h"

#include "m2config_builtin.h"
#include "m2config_eeprom.h"

#define PMIC_ADDRESS 0x2D

void m2config_get(struct m2config *config)
{
	u8 __attribute__((unused)) reg = 0;

	if (m2config_eeprom_read(config) == 0)
		return;

#ifdef CONFIG_SPL_BUILD
	if (i2c_read(PMIC_ADDRESS, 0x20, 1, &reg, 1) != 0) {
		printf("Error: PMIC read failed\n");
		goto default_config;
	} else {
		if (reg == 0x05) {
			config->ram = M2_RAM_MT47H128M16RT25E;	/* set DDR2 */
			printf("DDR2 detected\n");
		} else if (reg == 0x01) {
			config->ram = M2_RAM_K4B2G1646EBIH9;	/* set legacy DDR3 */
			printf("DDR3 detected\n");
		} else {
			goto default_config;
		}
	}

	return;

default_config:
#endif

	config->ram = M2_RAM_K4B2G1646EBIH9;	/* set default to legacy DDR3 */
	printf("Error: no RAM configuration found, trying fallback configuration %u\n", config->ram);
}
