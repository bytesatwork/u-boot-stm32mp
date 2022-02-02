// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 bytes at work AG
 */

#ifndef M2CONFIG_BUILTIN_H
#define M2CONFIG_BUILTIN_H

#include "m2config.h"
#include <config.h>

#if defined(CONFIG_M2CONFIG_BUILTIN)
int m2config_builtin(struct m2config *config);
#else
static inline int m2config_builtin(struct m2config *config)
{
	return -1;
}
#endif

#endif
