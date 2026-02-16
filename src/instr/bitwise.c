/*
 * libppcemu - PowerPC Instruction handling - Bitwise operations
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"

static void do_xor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] ^ state->gpr[rB];

	if (Rc) {
		/* TODO: update CR0 */
	}
}

static void ori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] | uimm;
}

#define do_ori(s, rS, rA, uimm) ori_common(s, rS, rA, uimm)
#define do_oris(s, rS, rA, uimm) ori_common(s, rS, rA, (uimm << 16))

static void do_or(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc)  {
	state->gpr[rA] = state->gpr[rS] | state->gpr[rB];

	if (Rc) {
		/* TODO: update CR0 */
	}
}
