/*
 * libppcemu - PowerPC Instruction handling - Syncronization/Cache
 *
 * Copyright (C) 2026 Techflash
 */

#include "../cache.h"
#include "../decode.h"
#include "../mem.h"
#include "../state.h"


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
}

void do_dcbf(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	ppcemu_dcache_writeback_invalidate_line(&state->dcache, ea);
}

void do_dcbst(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	ppcemu_dcache_writeback_line(&state->dcache, ea);
}

void do_dcbi(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	ppcemu_dcache_invalidate_line(&state->dcache, ea);
}

void do_icbi(struct _ppcemu_state *state, uint rA, uint rB) {
	u32 b, ea;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	ppcemu_icache_invalidate_line(&state->icache, ea);
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
	if (err != V2P_SUCCESS) {
		exception_fire(state, EXCEPTION_DSI);
		return;
	}

	if (cacheable)
		ppcemu_dcache_zero_line(&state->dcache, start);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, CACHE_LINE_SIZE, zero_line, true);
}
