/*
 * libppcemu - Internal Headers - State handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_STATE_H
#define _LIBPPCEMU_INTERNAL_STATE_H

#include <ppcemu/types.h>
#include <stdbool.h>
#include "types.h"
#include "spr.h"

struct _ppcemu_state {
	/* emulator state */
	bool ready;
	bool branched;
	enum ppcemu_cpu_model model;
	u32 caps;
	ppcemu_bus_hook bus_hook;
	uint bus_speed_khz;
	uint c2b_mult;

	/* CPU state */
	u32 gpr[32];
	u32 fpr[32];
	u32 fpcsr;
	u32 cr;
	u32 pc;
	u32 msr;
	u32 sprs[MAX_SPR_COUNT];
	u32 sr[16];
};


#endif /* _LIBPPCEMU_INTERNAL_STATE_H */
