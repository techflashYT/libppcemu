/*
 * libppcemu - Memory
 *
 * Copyright (C) 2026 Techflash
 *
 * BAT handling code based on code from dol-tools' dol-run:
 * Copyright (C) 2025 Techflash
 */

#define LOG_LEVEL virt2phys_loglevel
#include <stdio.h>
#include <ppcemu/msr.h>
#include "cache.h"
#include "caps.h"
#include "log.h"
#include "mem.h"
#include "ppcemu/spr.h"
#include "spr.h"
#include "state.h"
#include "types.h"

#ifdef DEBUG_VIRT2PHYS
#define mem_debug debug
#else
static void mem_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif

static uint bat_to_spr_idx(uint batnum, bool ibat, bool upper) {
	uint base, off;

	/* calculate base BAT SPRN */
	if (batnum <= 3) base = PPCEMU_SPRN_IBAT0U;
	else if (batnum >= 4 && batnum <= 7) base = PPCEMU_SPRN_IBAT4U;
	else assert(!"Unreachable");

	/* calculate offset (upper/lower) */
	if (upper) off = 0;
	else off = 1;

	/* calculate offset (instr/data) */
	if (!ibat) off += 8;

	/* calculate offset (number) */
	if (batnum <= 3) off += (2 * batnum);
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


enum virt2phys_err HIDDEN ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool *cacheable, bool ifetch, bool write) {
	u32 batu, batl, size, bepi, pp, wimg;
	uint i, max_bat;
	bool vs, vp;

	*cacheable = false;
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
		mem_debug("MEM: Checking if vaddr 0x%08x lives in %cBAT%d (%d bytes @ 0x%08x)...\r\n", virt, ifetch ? 'I' : 'D', i, size, bepi);

		/* Check if VA falls in this BAT */
		if (virt >= bepi && virt < bepi + size) {
			mem_debug("MEM: It does!\r\n");
			/* Compute physical base */
			*phys = ((batl & PPCEMU_BATL_BPRN) & ~(size - 1)) + (virt - bepi);

			/* Determine access permissions */
			pp = (batl & PPCEMU_BATL_PP) >> PPCEMU_BATL_PP_SHIFT;
			if (write && pp != PPCEMU_BATL_PP_RW)
				return V2P_NO_PERMS;
			else if (!write && pp == PPCEMU_BATL_PP___)
				return V2P_NO_PERMS;

			/* Determine if it can hit the cache by checking the cache inhibit bit */
			wimg = (batl & PPCEMU_BATL_WIMG) >> PPCEMU_BATL_WIMG_SHIFT;
			if (!(wimg & 0b0100))
				*cacheable = true; /* already set to false above */

			return V2P_SUCCESS;
		}
		mem_debug("MEM: It does not, continuing...\r\n");
	}

	/* No matching BAT */
	return V2P_NOT_MAPPED;
}

const char *v2p_strerror(enum virt2phys_err err) {
	switch (err) {
	case V2P_SUCCESS: return "Success";
	case V2P_NOT_MAPPED: return "Not Mapped";
	case V2P_NO_PERMS: return "Invalid Permissions";
	default: return "???";
	}
}

enum virt2phys_err _do_basic_store(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;
	bool cacheable;

	err = ppcemu_virt2phys(state, ea, &phys, &cacheable, false, true);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		warn("_do_basic_store: store %uB to 0x%08x @ PC=0x%08x: virt2phys error: %s (%d)\r\n", len, ea, state->pc, v2p_strerror(err), err);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	if (cacheable)
		ppcemu_dcache_store(&state->dcache, ea, len, val);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, len, val, true);
	return err;
}

enum virt2phys_err _do_basic_load(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;
	bool cacheable;

	err = ppcemu_virt2phys(state, ea, &phys, &cacheable, false, false);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		warn("_do_basic_load: load %uB from 0x%08x @ PC=0x%08x: virt2phys error: %s (%d)\r\n", len, ea, state->pc, v2p_strerror(err), err);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	if (cacheable)
		ppcemu_dcache_load(&state->dcache, ea, len, val);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, len, val, false);

	return err;
}
