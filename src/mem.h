/*
 * libppcemu - Internal Headers - Memory handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_MEM_H
#define _LIBPPCEMU_INTERNAL_MEM_H

#include "types.h"

/* converts a virtual address to a physical one, returning false if it failed */
bool ppcemu_virt2phys(u32 virt, u32 *phys, bool ifetch);

#endif /* _LIBPPCEMU_INTERNAL_MEM_H */
