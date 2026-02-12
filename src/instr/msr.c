/*
 * libppcemu - PowerPC Instruction handling - MSR
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"

static void do_mtmsr(struct _ppcemu_state *state, uint rS) {
	state->msr = state->gpr[rS];
}


static void do_mfmsr(struct _ppcemu_state *state, uint rD) {
	state->gpr[rD] = state->msr;
}
