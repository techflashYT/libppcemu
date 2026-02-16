/*
 * libppcemu - Internal Headers - Memory handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_MEM_H
#define _LIBPPCEMU_INTERNAL_MEM_H

#include "types.h"
#include "state.h"

enum virt2phys_err {
	V2P_SUCCESS,
	V2P_NOT_MAPPED,
	V2P_NO_PERMS
};

/* converts a virtual address to a physical one, returning various values if it failed */
enum virt2phys_err ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool ifetch, bool write);

#endif /* _LIBPPCEMU_INTERNAL_MEM_H */
