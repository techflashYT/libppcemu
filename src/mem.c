/*
 * libppcemu - Memory
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include <ppcemu/msr.h>
#include "state.h"
#include "types.h"

bool HIDDEN ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool ifetch) {
	if (ifetch && !(state->msr & PPCEMU_MSR_IR)) {
		*phys = virt;
		return true;
	}
	else if (!ifetch && !(state->msr & PPCEMU_MSR_DR)) {
		*phys = virt;
		return true;
	}

	/* TODO: the BATs */
	return false;
}
