/*
 * libppcemu - PowerPC Instruction handling - Floating Point
 *
 * Copyright (C) 2026 Techflash
 */

#include <assert.h>
#include <ppcemu/endian.h>
#include <ppcemu/spr.h>
#include "../exception.h"
#include "../mem.h"
#include "../state.h"

#define ENFORCE_MSR_FP() \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_PROGRAM); \
		return; \
	} \

void do_lfd(struct _ppcemu_state *state, uint frD, uint rA, u16 d) {
	u32 b, ea, val_hi, val_lo;
	enum virt2phys_err v2p_err;

	ENFORCE_MSR_FP();

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)(i16)d;

	v2p_err = _do_basic_load(state, 4, ea, &val_hi);
	if (v2p_err != V2P_SUCCESS)
		return;
	v2p_err = _do_basic_load(state, 4, ea + 4, &val_lo);
	if (v2p_err != V2P_SUCCESS)
		return;

	state->fpr[frD] = (u64)ppcemu_be32_to_cpu(val_hi) << 32;
	state->fpr[frD] |= ppcemu_be32_to_cpu(val_lo);
}

void do_fmr(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	ENFORCE_MSR_FP();

	state->fpr[frD] = state->gpr[frB];

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_mtfsf(struct _ppcemu_state *state, uint FM, uint frB, uint Rc) {
	u32 mask;
	uint i;

	if (FM == 0xff) /* fast path for "set all" */
		state->fpcsr = state->fpr[frB];
	else {
		/* actually mask it */
		mask = 0;
		for (i = 0; i < 8; i++) {
			if (FM & (1 << i))
				mask |= (0xf << (i * 4));
		}

		state->fpcsr &= ~mask;
		state->fpcsr |= (state->fpr[frB] & mask);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}


void do_mtfsbN(struct _ppcemu_state *state, uint crbD, uint set, uint Rc) {
	u32 mask = 1 << (31 - crbD);
	if (set)
		state->fpcsr |= mask;
	else
		state->fpcsr &= ~mask;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}
