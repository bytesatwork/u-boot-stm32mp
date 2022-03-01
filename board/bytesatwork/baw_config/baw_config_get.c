// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#include <linux/types.h>
#include <i2c.h>
#include <stdio.h>
#include "baw_config_get.h"

#include "baw_config_builtin.h"
#include "baw_config_eeprom.h"

#define PMIC_ADDRESS 0x2D

void baw_config_get(struct baw_config *config)
{
	u8 __attribute__((unused)) reg = 0;

	if (baw_config_eeprom_read(config) == 0) {
#ifdef CONFIG_SPL_BUILD
		printf("Use EEPROM RAM config: %u (%s)\n", config->ram,
		       baw_config_get_ram_name(config->ram));
#endif
		return;
	}

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_TARGET_BYTEENGINE_AM335X)
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

	if (IS_ENABLED(CONFIG_BAW_CONFIG_BUILTIN)) {
		config->ram = BAW_CONFIG_BUILTIN_RAM;
		if (IS_ENABLED(CONFIG_SPL_BUILD)) {
			printf("Use built in RAM config: %u (%s)\n", config->ram,
			       baw_config_get_ram_name(config->ram));

			return;
		}
	} else {
		if (IS_ENABLED(CONFIG_TARGET_BYTEENGINE_AM335X))
			config->ram = M2_RAM_K4B2G1646EBIH9;	/* set default to legacy DDR3 */
		else if (IS_ENABLED(CONFIG_TARGET_BYTEDEVKIT))
			config->ram = M2_RAM_K4B4G1646DBIK0;
		else if (IS_ENABLED(CONFIG_TARGET_IMX8MM_BYTEDEVKIT))
			config->ram = M6_RAM_MT53E128M32D2DS_053;
	}

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		printf("Error: no RAM configuration found, trying fallback config ");
		printf("%u (%s)\n", config->ram, baw_config_get_ram_name(config->ram));
	}
}
