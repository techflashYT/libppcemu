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
	u32 b, ea, start, zero = 0;
	uint i;
	enum virt2phys_err err;

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)state->gpr[rB];
	start = ea & ~31;

	for (i = 0; i < 32; i += 8) {
		err = _do_basic_store(state, 4, start + i, &zero);
		if (err != V2P_SUCCESS)
			return;
	}
}
