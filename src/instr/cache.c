/*
 * libppcemu - PowerPC Instruction handling - Syncronization/Cache
 *
 * Copyright (C) 2026 Techflash
 */

#include <ppcemu/spr.h>
#include "../cache.h"
#include "../caps.h"
#include "../decode.h"
#include "../mem.h"
#include "../spr.h"
#include "../state.h"

static bool cache_ea_to_phys(struct _ppcemu_state *state, u32 ea, u32 *phys) {
	bool cacheable;

	return ppcemu_virt2phys(state, ea, phys, &cacheable, false, false) == V2P_SUCCESS;
}


void do_isync(struct _ppcemu_state *state, u32 inst) {
	NO_RC();
	/* TODO: actually syncronize something */
}

void do_sync(struct _ppcemu_state *state, u32 inst) {
	NO_RC();
	/* TODO: actually syncronize something */
}

void do_eieio(struct _ppcemu_state *state) {
	/* TODO: actually syncronize something */
	(void)state;
}

void do_tlbie(struct _ppcemu_state *state, uint rB) {
	/* TODO: actually invalidate TLB entries */
	(void)state;
	(void)rB;
}

void do_tlbsync(struct _ppcemu_state *state) {
	(void)state;
}

void do_dcbf(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, phys;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	if (!cache_ea_to_phys(state, ea, &phys))
		return;
	ppcemu_dcache_writeback_invalidate_line(&state->dcache, phys);
}

void do_dcbst(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, phys;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	if (!cache_ea_to_phys(state, ea, &phys))
		return;
	ppcemu_dcache_writeback_line(&state->dcache, phys);
}

void do_dcbi(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, phys;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	if (!cache_ea_to_phys(state, ea, &phys))
		return;
	ppcemu_dcache_invalidate_line(&state->dcache, phys);
}

void do_icbi(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, phys;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	/* icbi is translated and protected as a data load */
	if (!cache_ea_to_phys(state, ea, &phys))
		return;
	ppcemu_icache_invalidate_line(&state->icache, phys);
}

void do_dcbz(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, start, phys;
	bool cacheable;
	enum virt2phys_err err;
	u8 zero_line[CACHE_LINE_SIZE] = {0};

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	start = ea & ~31;
	err = ppcemu_virt2phys(state, start, &phys, &cacheable, false, true);
	if (err == V2P_DIRECT_STORE)
		return;
	if (err != V2P_SUCCESS) {
		ppcemu_set_dsi_info(state, ea, err, true);
		exception_fire(state, EXCEPTION_DSI);
		return;
	}

	if (cacheable)
		ppcemu_dcache_zero_line(&state->dcache, phys);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, CACHE_LINE_SIZE, zero_line, true);
}

void do_dcbz_l(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, start, phys;
	bool cacheable;
	enum virt2phys_err err;
	u8 zero_line[CACHE_LINE_SIZE] = {0};

	if (!(state->caps & CAPS_HID2_GEKKO) ||
	    !(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_LCE)) {
		exception_fire(state, EXCEPTION_PROGRAM);
		return;
	}

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	start = ea & ~31;
	err = ppcemu_virt2phys(state, start, &phys, &cacheable, false, true);
	if (err != V2P_SUCCESS)
		return;

	if (cacheable)
		ppcemu_dcache_zero_line_locked(&state->dcache, phys);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, CACHE_LINE_SIZE, zero_line, true);
}

void do_dcbt(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea, start, phys;
	u8 dummy;
	bool cacheable;
	enum virt2phys_err err;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	start = ea & ~31;
	err = ppcemu_virt2phys(state, start, &phys, &cacheable, false, true);
	if (err != V2P_SUCCESS) /* architecturally defined to ignore translation failure */
		return;

	if (cacheable)
		ppcemu_dcache_load(&state->dcache, phys, 1, &dummy);
}
