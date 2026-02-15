/*
 * libppcemu - Running
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include <arpa/inet.h>
#include <ppcemu/types.h>
#include <ppcemu/state.h>
#include "state.h"
#include "mem.h"


/* for aggressive inlining */
#include "decode.c"

static bool _ppcemu_fetch(struct _ppcemu_state *state, u32 *instr) {
	u32 phys;

	if (!ppcemu_virt2phys(state, state->pc, &phys, true)) {
		printf("Instr fetch failed @ 0x%08x\r\n", state->pc);
		return false;
	}

	state->bus_hook((struct ppcemu_state *)state, phys, 4, instr, false);
	printf("Fetched instruction: %08x\r\n", ntohl(*instr));

	return true;
}

void ppcemu_step(struct ppcemu_state *emu) {
	struct _ppcemu_state *state;
	u32 instr;
	bool success;

	state = (struct _ppcemu_state *)emu;
	if (!state->ready)
		return;

	success = _ppcemu_fetch(state, &instr);
	if (!success) {
		exception_fire(state, EXCEPTION_ISI);
		return;
	}

	_ppcemu_decode_exec(state, htonl(instr));
}
