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

#define CORE_CYCLES_PER_INSTR 3

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
	u32 instr, dec, ticks_per_tb;
	u64 tb_delta;
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

	_ppcemu_decode_exec(state, instr);
	state->instr_count++;

	ticks_per_tb = 4 * state->c2b_mult;
	state->tb_remainder += CORE_CYCLES_PER_INSTR;
	tb_delta = 0;
	while (state->tb_remainder >= ticks_per_tb) {
		state->tb_remainder -= ticks_per_tb;
		state->tb++;
		tb_delta++;
	}

	state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_PMC1)] += CORE_CYCLES_PER_INSTR;

	if (tb_delta > 0) {
		dec = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DEC)];
		/* queue decrementer exception on underflow, if enabled */
		if (!state->dec_exception_pending)
			state->dec_exception_pending = ((i32)dec >= 0 && (i32)(dec - tb_delta) < 0);

		dec -= (u32)tb_delta;
		state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DEC)] = dec;
	}

	/* throttle if we're too fast (unlikely) */
	if (state->sync_rt && !(state->instr_count & 0x3fff))
		ppcemu_rt_throttle((struct ppcemu_state *)state);

	if (atomic_load_explicit(&state->external_interrupt_pending, memory_order_relaxed) && state->msr & PPCEMU_MSR_EE)
		exception_fire(state, EXCEPTION_EXT);
	else if (state->dec_exception_pending && state->msr & PPCEMU_MSR_EE) {
		exception_fire(state, EXCEPTION_DEC);
		state->dec_exception_pending = false;
	}
}
