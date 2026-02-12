/*
 * libppcemu - Memory
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include "types.h"

bool HIDDEN ppcemu_virt2phys(u32 virt, u32 *phys, bool ifetch) {
	puts("STUB virt2phys");

	*phys = virt;
	return true;
}
