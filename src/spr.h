/*
 * libppcemu - Internal Headers - PowerPC SPRs
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_SPR_H
#define _LIBPPCEMU_INTERNAL_SPR_H

#include <assert.h>
#include <ppcemu/spr.h>

#define MAX_SPR_COUNT 47
static inline int ppcemu_sprn_to_idx(int sprn) {
	switch (sprn) {
	case PPCEMU_SPRN_XER: return 0;
	case PPCEMU_SPRN_LR: return 1;
	case PPCEMU_SPRN_CTR: return 2;
	case PPCEMU_SPRN_DEC: return 3;
	case PPCEMU_SPRN_TBL_OEA: return 4; /* map the timebase registers to the same index */
	case PPCEMU_SPRN_TBU_OEA: return 5;
	case PPCEMU_SPRN_TBL_VEA: return 4;
	case PPCEMU_SPRN_TBU_VEA: return 5;
	case PPCEMU_SPRN_SRR0: return 6;
	case PPCEMU_SPRN_SRR1: return 7;
	case PPCEMU_SPRN_SPRG0: return 8;
	case PPCEMU_SPRN_SPRG1: return 9;
	case PPCEMU_SPRN_SPRG2: return 10;
	case PPCEMU_SPRN_SPRG3: return 11;
	case PPCEMU_SPRN_HID0: return 12;
	case PPCEMU_SPRN_HID2_GEKKO: return 13;
	case PPCEMU_SPRN_HID4: return 14;
	case PPCEMU_SPRN_IBAT0U: return 15;
	case PPCEMU_SPRN_IBAT0L: return 16;
	case PPCEMU_SPRN_IBAT1U: return 17;
	case PPCEMU_SPRN_IBAT1L: return 18;
	case PPCEMU_SPRN_IBAT2U: return 19;
	case PPCEMU_SPRN_IBAT2L: return 20;
	case PPCEMU_SPRN_IBAT3U: return 21;
	case PPCEMU_SPRN_IBAT3L: return 22;
	case PPCEMU_SPRN_DBAT0U: return 23;
	case PPCEMU_SPRN_DBAT0L: return 24;
	case PPCEMU_SPRN_DBAT1U: return 25;
	case PPCEMU_SPRN_DBAT1L: return 26;
	case PPCEMU_SPRN_DBAT2U: return 27;
	case PPCEMU_SPRN_DBAT2L: return 28;
	case PPCEMU_SPRN_DBAT3U: return 29;
	case PPCEMU_SPRN_DBAT3L: return 30;
	case PPCEMU_SPRN_IBAT4U: return 31;
	case PPCEMU_SPRN_IBAT4L: return 32;
	case PPCEMU_SPRN_IBAT5U: return 33;
	case PPCEMU_SPRN_IBAT5L: return 34;
	case PPCEMU_SPRN_IBAT6U: return 35;
	case PPCEMU_SPRN_IBAT6L: return 36;
	case PPCEMU_SPRN_IBAT7U: return 37;
	case PPCEMU_SPRN_IBAT7L: return 38;
	case PPCEMU_SPRN_DBAT4U: return 39;
	case PPCEMU_SPRN_DBAT4L: return 40;
	case PPCEMU_SPRN_DBAT5U: return 41;
	case PPCEMU_SPRN_DBAT5L: return 42;
	case PPCEMU_SPRN_DBAT6U: return 43;
	case PPCEMU_SPRN_DBAT6L: return 44;
	case PPCEMU_SPRN_DBAT7U: return 45;
	case PPCEMU_SPRN_DBAT7L: return 46;
	default: assert(!"Unreachable");
	}
}

#endif /* _LIBPPCEMU_INTERNAL_SPR_H */
