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

#define ENFORCE_MSR_FP(x) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_PROGRAM); \
		return x; \
	} \

u32 do_lfd(struct _ppcemu_state *state, uint frD, uint rA, u16 d) {
	u32 b, ea, val_hi, val_lo;
	enum virt2phys_err v2p_err;

	ENFORCE_MSR_FP(0);

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)(i16)d;

	v2p_err = _do_basic_load(state, 4, ea, &val_hi);
	if (v2p_err != V2P_SUCCESS)
		return 0;
	v2p_err = _do_basic_load(state, 4, ea + 4, &val_lo);
	if (v2p_err != V2P_SUCCESS)
		return 0;

	state->fpr[frD].u64 = ((u64)ppcemu_be32_to_cpu(val_hi) << 32) |
				ppcemu_be32_to_cpu(val_lo);

	return ea;
}

u32 do_lfs(struct _ppcemu_state *state, uint frD, uint rA, u16 d) {
	u32 b, ea;
	union {
		u64 u64;
		u32 u32;
		float singleFloat;
		double doubleFloat;
	} val;
	enum virt2phys_err v2p_err;

	ENFORCE_MSR_FP(0);

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + (i32)(i16)d;

	v2p_err = _do_basic_load(state, 4, ea, &val.u32);
	if (v2p_err != V2P_SUCCESS)
		return 0;

	if (state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE) {
		/* load into both PS slots */
		state->fpr[frD].u32[0] = ppcemu_be32_to_cpu(val.u32);
		state->fpr[frD].u32[1] = ppcemu_be32_to_cpu(val.u32);
	}
	else {
		/* byteswap if needed */
		val.u32 = ppcemu_be32_to_cpu(val.u32);

		/* reinterpret as single precision float, cast to double */
		val.doubleFloat = (double)val.singleFloat;

		/* store that double back into the FPR */
		state->fpr[frD].u64 = val.u64;
	}

	return ea;
}

void do_fmr(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	ENFORCE_MSR_FP();

	state->fpr[frD].u64 = state->fpr[frB].u64;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_mtfsf(struct _ppcemu_state *state, uint FM, uint frB, uint Rc) {
	u32 mask;
	uint i;

	if (FM == 0xff) /* fast path for "set all" */
		state->fpcsr = state->fpr[frB].u32[0];
	else {
		/* actually mask it */
		mask = 0;
		for (i = 0; i < 8; i++) {
			if (FM & (1 << i))
				mask |= (0xf << (i * 4));
		}

		state->fpcsr &= ~mask;
		state->fpcsr |= (state->fpr[frB].u32[0] & mask);
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
