/*
 * libppcemu - Memory
 *
 * Copyright (C) 2026 Techflash
 *
 * BAT handling code based on code from dol-tools' dol-run:
 * Copyright (C) 2025 Techflash
 */

#include <stdio.h>
#include <ppcemu/msr.h>
#include "caps.h"
#include "mem.h"
#include "ppcemu/spr.h"
#include "spr.h"
#include "state.h"
#include "types.h"

static uint bat_to_spr_idx(uint batnum, bool ibat, bool upper) {
	uint base, off;

	/* calculate base BAT SPRN */
	if (batnum >= 0 && batnum <= 3) base = PPCEMU_SPRN_IBAT0U;
	else if (batnum >= 4 && batnum <= 7) base = PPCEMU_SPRN_IBAT4U;
	else assert(!"Unreachable");

	/* calculate offset (upper/lower) */
	if (upper) off = 0;
	else off = 1;

	/* calculate offset (instr/data) */
	if (!ibat) off += 8;

	/* calculate offset (number) */
	if (batnum >= 0 && batnum <= 3) off += (2 * batnum);
	else if (batnum >= 4 && batnum <= 7) off += (2 * (batnum - 4));
	else assert(!"Unreachable");

	/* actually grab the index */
	return ppcemu_sprn_to_idx(base + off);
}

static uint bat_blocklen_to_bytes(u32 bl) {
	switch (bl) {
		case PPCEMU_BATU_BL_128KB: return 128 * 1024;
		case PPCEMU_BATU_BL_256KB: return 256 * 1024;
		case PPCEMU_BATU_BL_512KB: return 512 * 1024;
		case PPCEMU_BATU_BL_1MB: return 1 * 1024 * 1024;
		case PPCEMU_BATU_BL_2MB: return 2 * 1024 * 1024;
		case PPCEMU_BATU_BL_4MB: return 4 * 1024 * 1024;
		case PPCEMU_BATU_BL_8MB: return 8 * 1024 * 1024;
		case PPCEMU_BATU_BL_16MB: return 16 * 1024 * 1024;
		case PPCEMU_BATU_BL_32MB: return 32 * 1024 * 1024;
		case PPCEMU_BATU_BL_64MB: return 64 * 1024 * 1024;
		case PPCEMU_BATU_BL_128MB: return 128 * 1024 * 1024;
		case PPCEMU_BATU_BL_256MB: return 256 * 1024 * 1024;
		default: return 0;
	}
	return 0;
}


enum virt2phys_err HIDDEN ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool ifetch, bool write) {
	u32 batu, batl, size, bepi, pp;
	uint i, max_bat;
	bool vs, vp;

	if (ifetch && !(state->msr & PPCEMU_MSR_IR)) {
		*phys = virt;
		return V2P_SUCCESS;
	}
	else if (!ifetch && !(state->msr & PPCEMU_MSR_DR)) {
		*phys = virt;
		return V2P_SUCCESS;
	}
	if (state->caps & CAPS_UPPER_BATS &&
	    state->caps & CAPS_HID4 &&
	    state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID4)] & PPCEMU_HID4_SBE)
		max_bat = 8;
	else
		max_bat = 4;

	for (i = 0; i < max_bat; i++) {
		batu = state->sprs[bat_to_spr_idx(i, ifetch, true)];
		batl = state->sprs[bat_to_spr_idx(i, ifetch, false)];

		/* Skip inactive BATs (VS and VP bits both 0) */
		vs = (batu & PPCEMU_BATU_VS) != 0;
		vp = (batu & PPCEMU_BATU_VP) != 0;
		if (!vs && !vp) continue;

		/* Compute BAT block size */
		size = bat_blocklen_to_bytes((batu & PPCEMU_BATU_BL) >> PPCEMU_BATU_BL_SHIFT);

		/* Compute virtual base */
		bepi = (batu & PPCEMU_BATU_BEPI);
		printf("MEM: Checking if vaddr 0x%08x lives in %cBAT%d (%d bytes @ 0x%08x)...\n", virt, ifetch ? 'I' : 'D', i, size, bepi);

		/* Check if VA falls in this BAT */
		if (virt >= bepi && virt < bepi + size) {
			puts("MEM: It does!");
			/* Compute physical base */
			*phys = ((batl & PPCEMU_BATL_BPRN) & ~(size - 1)) + (virt - bepi);

			/* Determine access permissions */
			pp = (batl & PPCEMU_BATL_PP) >> PPCEMU_BATL_PP_SHIFT;
			if (write && pp != PPCEMU_BATL_PP_RW)
				return V2P_NO_PERMS;
			else if (!write && pp == PPCEMU_BATL_PP___)
				return V2P_NO_PERMS;

			return V2P_SUCCESS;
		}
		puts("MEM: It does not, continuing...");
	}

	/* No matching BAT */
	return V2P_NOT_MAPPED;
}
