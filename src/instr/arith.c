/*
 * libppcemu - PowerPC Instruction handling - Arithmetic
 *
 * Copyright (C) 2026 Techflash
 */

#include "../cr.h"
#include "../state.h"


static void generic_addi(struct _ppcemu_state *state, uint rD, uint rA, u32 simm) {
	if (rA == 0)
		state->gpr[rD] = simm;
	else
		state->gpr[rD] = simm + state->gpr[rA];
}

#define do_addi(s, rD, rA, simm) generic_addi(s, rD, rA, simm)
#define do_addis(s, rD, rA, simm) generic_addi(s, rD, rA, (simm << 16))

static void do_add(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	state->gpr[rD] = state->gpr[rA] + state->gpr[rB];

	/* TODO: update XER if OE */

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

static void do_addic(struct _ppcemu_state *state, uint rD, uint rA, u16 simm, uint Rc) {
	state->gpr[rD] = state->gpr[rA] + (i32)(i16)simm;

	/* TODO: update XER.CA */

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

static void do_subf(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	state->gpr[rD] = (~state->gpr[rA]) + state->gpr[rB] + 1;

	/* TODO: update XER if OE */

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}
