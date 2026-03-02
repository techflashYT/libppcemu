/*
 * libppcemu - PowerPC Instruction handling - Branches
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"
#include "../cr.h"
#include <stdio.h>
#include <ppcemu/spr.h>

static void do_rfi(struct _ppcemu_state *state, u32 inst) {
	NO_RC();

	state->pc = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR0)];
	/* TODO: mask MSR */
	state->msr = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR1)];

	state->branched = true;
}

static void do_branch(struct _ppcemu_state *state, u32 li, uint aa, uint lk) {
	u32 oldpc = state->pc;

	if (aa)
		state->pc = (u32)(i32)(i16)(li << 2);
	else
		state->pc += (u32)(i32)(i16)(li << 2);

	if (lk)
		state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)] = oldpc + 4;

	state->branched = true;
}

static void _do_cond_branch(struct _ppcemu_state *state, uint bo, uint bi, uint lk, u32 target_addr) {
	bool ctr_ignore, cond_ignore, ctr_ok, cond_ok;
	u32 oldpc;

	ctr_ignore = !!(bo & 0x4);
	cond_ignore = !!(bo & 0x10);
	ctr_ok = true;
	cond_ok = true;

	if (!ctr_ignore)
		ctr_ok = (--state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_CTR)] == 0) && !!(bo & 0x2);

	if (!cond_ignore)
		cond_ok = (cr_get_bit(state, bi) == !!(bo & 0x8));

	if (ctr_ok && cond_ok) {
		oldpc = state->pc;
		state->pc = target_addr;
		state->branched = true;
		if (lk)
			state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)] = oldpc + 4;
	}
}

#define do_bclr(s, bo, bi, lk) _do_cond_branch(s, bo, bi, lk, s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)])

static void do_bc(struct _ppcemu_state *state, uint bo, uint bi, uint bd, uint aa, uint lk) {
	u32 ea;

	if (aa)
		ea = (u32)(i32)(i16)(bd << 2);
	else
		ea = state->pc + (i32)(i16)(bd << 2);

	_do_cond_branch(state, bo, bi, lk, ea);
}
