/*
 * libppcemu - PowerPC Instruction handling - Segment Registers
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#define LOG_LEVEL misc_loglevel
#include "../state.h"
#include "../log.h"

void do_mtsr(struct _ppcemu_state *state, uint sreg, uint rS) {
	u32 val;

	val = state->gpr[rS];
	if (!(val & 0x80000000))
		warn("Writing a non-direct-store segment which will not be interpreted!!!");

	state->sr[sreg] = val;
}

void do_mfsr(struct _ppcemu_state *state, uint sreg, uint rD) {
	state->gpr[rD] = state->sr[sreg];
}
