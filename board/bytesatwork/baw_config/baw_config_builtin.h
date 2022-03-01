/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 bytes at work AG
 */

#ifndef BAW_CONFIG_BUILTIN_H
#define BAW_CONFIG_BUILTIN_H

#include "baw_config.h"
#include <config.h>

#if defined(CONFIG_BAW_CONFIG_BUILTIN)
int baw_config_builtin(struct baw_config *config);
#else
static inline int baw_config_builtin(struct baw_config *config)
{
	return -1;
}
#endif

#endif
