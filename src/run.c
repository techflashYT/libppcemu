/*
 * libppcemu - Running
 *
 * Copyright (C) 2026 Techflash
 */

#define LOG_LEVEL ifetch_loglevel

#include <stdio.h>
#include <ppcemu/endian.h>
#include <ppcemu/msr.h>
#include <ppcemu/spr.h>
#include <ppcemu/state.h>
#include <ppcemu/types.h>
#include "cache.h"
#include "decode.h"
#include "exception.h"
#include "log.h"
#include "mem.h"
#include "spr.h"
#include "state.h"


#ifdef DEBUG_IFETCH
#define ifetch_debug debug
#else
static void ifetch_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif

static enum virt2phys_err _ppcemu_fetch(struct _ppcemu_state *state, u32 *instr) {
	u32 phys;
	enum virt2phys_err err;
	bool cacheable;

	err = ppcemu_virt2phys(state, state->pc, &phys, &cacheable, true, false);

	if (err != V2P_SUCCESS) {
		warn("Instr fetch failed @ 0x%08x due to virt2phys err %d\r\n", state->pc, err);
		return err;
	}

	if (cacheable)
		ppcemu_icache_fetch(&state->icache, state->pc, instr);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, 4, instr, false);

	*instr = ppcemu_be32_to_cpu(*instr);
	verbose("Fetched instruction @ 0x%08x: %08x\r\n", state->pc, *instr);

	return err;
}

void ppcemu_step(struct ppcemu_state *emu) {
	struct _ppcemu_state *state;
	u32 instr, dec;
	u64 cycles;
	enum virt2phys_err err;
	bool fire_dec_exception;

	state = (struct _ppcemu_state *)emu;
	if (!state->ready)
		return;

	err = _ppcemu_fetch(state, &instr);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		exception_fire(state, EXCEPTION_ISI);
		return;
	}

	_ppcemu_decode_exec(state, instr);

	/* handle timing stuff */
	state->instr_count++;
	/* TODO: eventually implement these... */
	if (!(state->instr_count & 0x7f) && state->sync_rt) {
		cycles = ppcemu_rt_refresh((struct ppcemu_state *)state);

		/* increase timebase */
		state->tb += cycles;
		/* SPRs determined from state->tb at runtime */

		/* decrement decrementer */
		dec = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DEC)];
		fire_dec_exception = ((i32)dec >= 0 && (i32)(dec - cycles) < 0 && state->msr & PPCEMU_MSR_EE);
		dec -= cycles;
		state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DEC)] = dec;

		if (fire_dec_exception)
			exception_fire(state, EXCEPTION_DEC);
	}
	else if (!(state->instr_count & 0x7)) {
		/* increase timebase */
		/*
		 * Ugly hack: core can execute approximately 2 instructions per bus cycle, and TB is 1/4 bus clock,
		 * so increasing it by the core multiplier every 8 instructions gets _vaguely_ close to legitimate timing.
		 * This isn't all that accurate but it is very fast, since it's just an addition.
		 */
		state->tb += state->c2b_mult;
		/* SPRs determined from state->tb at runtime */

		/* decrement decrementer */
		dec = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DEC)];
		/* fire decrementer exception on underflow, if enabled */
		fire_dec_exception = ((i32)dec >= 0 && (i32)(dec - state->c2b_mult) < 0 && state->msr & PPCEMU_MSR_EE);
		dec -= state->c2b_mult;
		state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DEC)] = dec;

		if (fire_dec_exception)
			exception_fire(state, EXCEPTION_DEC);
	}
}
