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

#include "../config.h"

/* for aggressive inlining */
#include "decode.c"

#ifdef DEBUG_IFETCH
#define ifetch_debug printf
#else
static void ifetch_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif

static enum virt2phys_err _ppcemu_fetch(struct _ppcemu_state *state, u32 *instr) {
	u32 phys;
	enum virt2phys_err err;

	err = ppcemu_virt2phys(state, state->pc, &phys, true, false);

	if (err != V2P_SUCCESS) {
		ifetch_debug("Instr fetch failed @ 0x%08x due to virt2phys err %d\r\n", state->pc, err);
		return err;
	}

	state->bus_hook((struct ppcemu_state *)state, phys, 4, instr, false);
	ifetch_debug("Fetched instruction @ 0x%08x: %08x\r\n", state->pc, ntohl(*instr));

	return err;
}

void ppcemu_step(struct ppcemu_state *emu) {
	struct _ppcemu_state *state;
	u32 instr;
	enum virt2phys_err err;

	state = (struct _ppcemu_state *)emu;
	if (!state->ready)
		return;

	err = _ppcemu_fetch(state, &instr);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		exception_fire(state, EXCEPTION_ISI);
		return;
	}

	_ppcemu_decode_exec(state, htonl(instr));
}
