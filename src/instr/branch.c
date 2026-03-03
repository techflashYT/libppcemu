/*
 * libppcemu - PowerPC Instruction handling - Branches
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include <ppcemu/spr.h>
#include "../cr.h"
#include "../state.h"
#include "../../config.h"

#ifdef DEBUG_BRANCH
#define branch_debug printf
#else
static void branch_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif


static void do_rfi(struct _ppcemu_state *state, u32 inst) {
	u32 oldpc = state->pc;

	NO_RC();

	state->pc = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR0)];
	/* TODO: mask MSR */
	state->msr = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR1)];

	branch_debug("rfi branched from 0x%08x -> 0x%08x\r\n", oldpc, state->pc);
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

	branch_debug("branched from 0x%08x -> 0x%08x\r\n", oldpc, state->pc);
	state->branched = true;
}

static void _do_cond_branch(struct _ppcemu_state *state, uint bo, uint bi, uint lk, u32 target_addr) {
	bool ctr_ignore, cond_ignore, ctr_ok, cond_ok;
	u32 oldpc, *ctr, oldctr;

	ctr_ignore = !!(bo & 0x4);
	cond_ignore = !!(bo & 0x10);
	ctr_ok = true;
	cond_ok = true;

	ctr = &state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_CTR)];
	oldctr = *ctr;
	if (!ctr_ignore) {
		(*ctr)--;

		if (bo & 2)
			ctr_ok = *ctr == 0;
		else
			ctr_ok = *ctr != 0;
	}

	if (!cond_ignore)
		cond_ok = (cr_get_bit(state, (31 - bi)) == !!(bo & 0x8));
	branch_debug("_do_cond_branch: BO=0x%x, BI=0x%x, ctr_ignore=%d, cond_ignore=%d, ctr_ok=%d, cond_ok=%d, CR=0x%08x, oldCTR=0x%08x, CTR=0x%08x\r\n", bo, bi, ctr_ignore, cond_ignore, ctr_ok, cond_ok, state->cr, oldctr, *ctr);

	oldpc = state->pc;
	if (ctr_ok && cond_ok) {
		state->pc = target_addr;
		state->branched = true;
		if (lk)
			state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)] = oldpc + 4;

		branch_debug("conditional branch from 0x%08x -> 0x%08x WAS taken\r\n", oldpc, target_addr);
	}
	else
		branch_debug("conditional branch from 0x%08x -> 0x%08x NOT taken\r\n", oldpc, target_addr);
}

#define do_bclr(s, bo, bi, lk) _do_cond_branch(s, bo, bi, lk, s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)])
#define do_bcctr(s, bo, bi, lk) _do_cond_branch(s, bo, bi, lk, s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_CTR)])

static void do_bc(struct _ppcemu_state *state, uint bo, uint bi, uint bd, uint aa, uint lk) {
	u32 ea;

	if (aa)
		ea = (u32)(i32)(i16)(bd << 2);
	else
		ea = state->pc + (i32)(i16)(bd << 2);

	_do_cond_branch(state, bo, bi, lk, ea);
}
