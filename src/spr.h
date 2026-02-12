/*
 * libppcemu - Internal Headers - PowerPC SPRs
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_SPR_H
#define _LIBPPCEMU_INTERNAL_SPR_H

#include <assert.h>
#include <ppcemu/spr.h>

#define MAX_SPR_COUNT 48
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
	case PPCEMU_SPRN_IBAT0U: return 16;
	case PPCEMU_SPRN_IBAT0L: return 17;
	case PPCEMU_SPRN_IBAT1U: return 18;
	case PPCEMU_SPRN_IBAT1L: return 19;
	case PPCEMU_SPRN_IBAT2U: return 20;
	case PPCEMU_SPRN_IBAT2L: return 21;
	case PPCEMU_SPRN_IBAT3U: return 22;
	case PPCEMU_SPRN_IBAT3L: return 23;
	case PPCEMU_SPRN_DBAT0U: return 24;
	case PPCEMU_SPRN_DBAT0L: return 25;
	case PPCEMU_SPRN_DBAT1U: return 26;
	case PPCEMU_SPRN_DBAT1L: return 27;
	case PPCEMU_SPRN_DBAT2U: return 28;
	case PPCEMU_SPRN_DBAT2L: return 29;
	case PPCEMU_SPRN_DBAT3U: return 30;
	case PPCEMU_SPRN_DBAT3L: return 31;
	case PPCEMU_SPRN_IBAT4U: return 32;
	case PPCEMU_SPRN_IBAT4L: return 33;
	case PPCEMU_SPRN_IBAT5U: return 34;
	case PPCEMU_SPRN_IBAT5L: return 35;
	case PPCEMU_SPRN_IBAT6U: return 36;
	case PPCEMU_SPRN_IBAT6L: return 37;
	case PPCEMU_SPRN_IBAT7U: return 38;
	case PPCEMU_SPRN_IBAT7L: return 39;
	case PPCEMU_SPRN_DBAT4U: return 40;
	case PPCEMU_SPRN_DBAT4L: return 41;
	case PPCEMU_SPRN_DBAT5U: return 42;
	case PPCEMU_SPRN_DBAT5L: return 43;
	case PPCEMU_SPRN_DBAT6U: return 44;
	case PPCEMU_SPRN_DBAT6L: return 45;
	case PPCEMU_SPRN_DBAT7U: return 46;
	case PPCEMU_SPRN_DBAT7L: return 47;
	default: assert(!"Unreachable");
	}
}

#endif /* _LIBPPCEMU_INTERNAL_SPR_H */
