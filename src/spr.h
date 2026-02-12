/*
 * libppcemu - Internal Headers - PowerPC SPRs
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_SPR_H
#define _LIBPPCEMU_INTERNAL_SPR_H

#include <assert.h>
#include <ppcemu/spr.h>

#define MAX_SPR_COUNT 16
static inline int ppcemu_sprn_to_idx(int sprn) {
	switch (sprn) {
	case PPCEMU_SPRN_LR: return 0;
	case PPCEMU_SPRN_CTR: return 1;
	case PPCEMU_SPRN_DEC: return 2;
	case PPCEMU_SPRN_TBL_OEA: return 3; /* map the timebase registers to the same index */
	case PPCEMU_SPRN_TBU_OEA: return 4;
	case PPCEMU_SPRN_TBL_VEA: return 3;
	case PPCEMU_SPRN_TBU_VEA: return 4;
	case PPCEMU_SPRN_SRR0: return 7;
	case PPCEMU_SPRN_SRR1: return 8;
	case PPCEMU_SPRN_SPRG0: return 9;
	case PPCEMU_SPRN_SPRG1: return 10;
	case PPCEMU_SPRN_SPRG2: return 11;
	case PPCEMU_SPRN_SPRG3: return 12;
	case PPCEMU_SPRN_HID0: return 13;
	case PPCEMU_SPRN_HID2_GEKKO: return 14;
	case PPCEMU_SPRN_HID4: return 15;
	default: assert(!"Unreachable");
	}
}

#endif /* _LIBPPCEMU_INTERNAL_SPR_H */
