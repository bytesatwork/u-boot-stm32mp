/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 bytes at work AG
 */

#ifndef BAW_CONFIG_EEPROM_H
#define BAW_CONFIG_EEPROM_H

#include "baw_config.h"
#include <config.h>

#if defined(CONFIG_BAW_CONFIG_EEPROM)
int baw_config_eeprom_read(struct baw_config *config);
int baw_config_eeprom_init(void);
#else
static inline int baw_config_eeprom_read(struct baw_config *config)
{
	return -1;
}
#endif

#if defined(CONFIG_BAW_CONFIG_EEPROM) && \
	(defined(CONFIG_SKIP_LOWLEVEL_INIT) || !defined(CONFIG_TARGET_BYTEENGINE_AM335X))
int baw_config_eeprom_write(struct baw_config *config);
int baw_config_eeprom_erase(void);
#else
static inline int baw_config_eeprom_write(struct baw_config *config)
{
	return -1;
}

static inline int baw_config_eeprom_erase(void)
{
	return -1;
}
#endif

#endif
