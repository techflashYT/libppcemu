/*
 * libppcemu - PowerPC Instruction handling - Addition
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"


static void generic_addi(struct _ppcemu_state *state, uint rD, uint rA, u32 simm) {
	if (rA == 0)
		state->gpr[rD] = simm;
	else
		state->gpr[rD] = simm + state->gpr[rA];
}

#define do_addi(s, rD, rA, simm) generic_addi(s, rD, rA, simm)
#define do_addis(s, rD, rA, simm) generic_addi(s, rD, rA, (simm << 16))
