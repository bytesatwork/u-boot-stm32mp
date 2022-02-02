// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#ifndef M2CONFIG_EEPROM_H
#define M2CONFIG_EEPROM_H

#include "m2config.h"
#include <config.h>

#if defined(CONFIG_M2CONFIG_EEPROM)
int m2config_eeprom_read(struct m2config *config);
int m2config_eeprom_init(void);
#else
static inline int m2config_eeprom_read(struct m2config *config)
{
	return -1;
}
#endif

#if defined(CONFIG_M2CONFIG_EEPROM) && defined(CONFIG_SKIP_LOWLEVEL_INIT)
int m2config_eeprom_write(struct m2config *config);
int m2config_eeprom_erase(void);
#else
static inline int m2config_eeprom_write(struct m2config *config)
{
	return -1;
}

static inline int m2config_eeprom_erase(void)
{
	return -1;
}
#endif

#endif
