/*
 * libppcemu - Internal Headers - PowerPC SPRs
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_SPR_H
#define _LIBPPCEMU_INTERNAL_SPR_H

#include <assert.h>
#include <ppcemu/spr.h>

#define MAX_SPR_COUNT 60
static inline int ppcemu_sprn_to_idx_raw(int sprn) {
	switch (sprn) {
	case PPCEMU_SPRN_XER: return 0;
	case PPCEMU_SPRN_LR: return 1;
	case PPCEMU_SPRN_CTR: return 2;
	case PPCEMU_SPRN_DEC: return 3;
	case PPCEMU_SPRN_SRR0: return 4;
	case PPCEMU_SPRN_SRR1: return 5;
	case PPCEMU_SPRN_SPRG0: return 6;
	case PPCEMU_SPRN_SPRG1: return 7;
	case PPCEMU_SPRN_SPRG2: return 8;
	case PPCEMU_SPRN_SPRG3: return 9;
	case PPCEMU_SPRN_HID0: return 10;
	case PPCEMU_SPRN_HID2_GEKKO: return 11;
	case PPCEMU_SPRN_HID4: return 12;
	case PPCEMU_SPRN_IBAT0U: return 13;
	case PPCEMU_SPRN_IBAT0L: return 14;
	case PPCEMU_SPRN_IBAT1U: return 15;
	case PPCEMU_SPRN_IBAT1L: return 16;
	case PPCEMU_SPRN_IBAT2U: return 17;
	case PPCEMU_SPRN_IBAT2L: return 18;
	case PPCEMU_SPRN_IBAT3U: return 19;
	case PPCEMU_SPRN_IBAT3L: return 20;
	case PPCEMU_SPRN_DBAT0U: return 21;
	case PPCEMU_SPRN_DBAT0L: return 22;
	case PPCEMU_SPRN_DBAT1U: return 23;
	case PPCEMU_SPRN_DBAT1L: return 24;
	case PPCEMU_SPRN_DBAT2U: return 25;
	case PPCEMU_SPRN_DBAT2L: return 26;
	case PPCEMU_SPRN_DBAT3U: return 27;
	case PPCEMU_SPRN_DBAT3L: return 28;
	case PPCEMU_SPRN_IBAT4U: return 29;
	case PPCEMU_SPRN_IBAT4L: return 30;
	case PPCEMU_SPRN_IBAT5U: return 31;
	case PPCEMU_SPRN_IBAT5L: return 32;
	case PPCEMU_SPRN_IBAT6U: return 33;
	case PPCEMU_SPRN_IBAT6L: return 34;
	case PPCEMU_SPRN_IBAT7U: return 35;
	case PPCEMU_SPRN_IBAT7L: return 36;
	case PPCEMU_SPRN_DBAT4U: return 37;
	case PPCEMU_SPRN_DBAT4L: return 38;
	case PPCEMU_SPRN_DBAT5U: return 39;
	case PPCEMU_SPRN_DBAT5L: return 40;
	case PPCEMU_SPRN_DBAT6U: return 41;
	case PPCEMU_SPRN_DBAT6L: return 42;
	case PPCEMU_SPRN_DBAT7U: return 43;
	case PPCEMU_SPRN_DBAT7L: return 44;
	case PPCEMU_SPRN_GQR0: return 45;
	case PPCEMU_SPRN_GQR1: return 46;
	case PPCEMU_SPRN_GQR2: return 47;
	case PPCEMU_SPRN_GQR3: return 48;
	case PPCEMU_SPRN_GQR4: return 49;
	case PPCEMU_SPRN_GQR5: return 50;
	case PPCEMU_SPRN_GQR6: return 51;
	case PPCEMU_SPRN_GQR7: return 52;
	case PPCEMU_SPRN_L2CR: return 53;
	case PPCEMU_SPRN_MMCR0: return 54;
	case PPCEMU_SPRN_MMCR1: return 55;
	case PPCEMU_SPRN_PMC1: return 56;
	case PPCEMU_SPRN_PMC2: return 57;
	case PPCEMU_SPRN_PMC3: return 58;
	case PPCEMU_SPRN_PMC4: return 59;
	default: return -1;
	}
}

static inline int ppcemu_sprn_to_idx(int sprn) {
	int idx = ppcemu_sprn_to_idx_raw(sprn);

	if (idx == -1)
		assert(!"bad sprn");

	return idx;
}

static inline int ppcemu_gqrn_to_spr_idx(int gqrn) {
	switch (gqrn) {
	case 0: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR0);
	case 1: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR1);
	case 2: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR2);
	case 3: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR3);
	case 4: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR4);
	case 5: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR5);
	case 6: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR6);
	case 7: return ppcemu_sprn_to_idx(PPCEMU_SPRN_GQR7);
	default: assert(!"Unreachable");
	}
}

#endif /* _LIBPPCEMU_INTERNAL_SPR_H */
