/*
 * libppcemu - PowerPC Instruction handling - Floating Point
 *
 * Copyright (C) 2026 Techflash
 */

#include <assert.h>
#include <math.h>
#include <ppcemu/endian.h>
#include <ppcemu/spr.h>
#include "../caps.h"
#include "../cr.h"
#include "../exception.h"
#include "../mem.h"
#include "../state.h"

union fp_val {
	u64 u64;
	u32 u32;
	float singleFloat;
	double doubleFloat;
};

#define ENFORCE_MSR_FP(x) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_FP_UNAV); \
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
	union fp_val val;
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

u32 do_stf_common(struct _ppcemu_state *state, uint frS, uint rA, i32 d, uint width, bool convert) {
	u32 b, ea;
	float f32;
	enum virt2phys_err v2p_err;

	ENFORCE_MSR_FP(0);

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + d;

	if (convert && width == 4) {
		f32 = (float)state->fpr[frS].dblPrec;
		v2p_err = _do_basic_store(state, 4, ea, &f32);
		if (v2p_err != V2P_SUCCESS)
			return 0;
	}
	else {
		v2p_err = _do_basic_store(state, 4, ea, &state->fpr[frS].u32[0]);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (width == 8) {
			v2p_err = _do_basic_store(state, 4, ea + 4, &state->fpr[frS].u32[1]);
			if (v2p_err != V2P_SUCCESS)
				return 0;
		}
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

void do_fctiwz(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	i32 asi32;
	double rounded;

	ENFORCE_MSR_FP();

	rounded = trunc(state->fpr[frB].dblPrec);
	asi32 = (i32)rounded;

	state->fpr[frD].u32[0] = (u32)asi32;
	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_frsp(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	float rounded;

	ENFORCE_MSR_FP();

	/* TODO: should check mode via FPSCR[RN] */
	rounded = (float)state->fpr[frB].dblPrec;

	if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
		state->fpr[frD].ps[0] = rounded;
	else
		state->fpr[frD].dblPrec = (double)rounded;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fmul_common(struct _ppcemu_state *state, uint frD, uint frA, uint frC, uint Rc, uint width) {
	if (width == 8)
		state->fpr[frD].dblPrec = state->fpr[frA].dblPrec * state->fpr[frC].dblPrec;
	else if (width == 4) {
		if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
			state->fpr[frD].ps[0] = state->fpr[frD].ps[1] = state->fpr[frA].ps[0] * state->fpr[frC].ps[0];
		else
			state->fpr[frD].dblPrec = (float)(state->fpr[frA].dblPrec * state->fpr[frC].dblPrec);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fadd_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc, uint width) {
	if (width == 8)
		state->fpr[frD].dblPrec = state->fpr[frA].dblPrec + state->fpr[frB].dblPrec;
	else if (width == 4) {
		if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
			state->fpr[frD].ps[0] = state->fpr[frD].ps[1] = state->fpr[frA].ps[0] + state->fpr[frB].ps[0];
		else
			state->fpr[frD].dblPrec = (float)(state->fpr[frA].dblPrec + state->fpr[frB].dblPrec);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fmadd_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc, uint width) {
	if (width == 8)
		state->fpr[frD].dblPrec = (state->fpr[frA].dblPrec * state->fpr[frC].dblPrec) + state->fpr[frB].dblPrec;
	else if (width == 4) {
		if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
			state->fpr[frD].ps[0] = state->fpr[frD].ps[1] = (state->fpr[frA].ps[0] * state->fpr[frC].ps[0]) + state->fpr[frB].ps[0];
		else
			state->fpr[frD].dblPrec = (float)((state->fpr[frA].dblPrec * state->fpr[frC].dblPrec) + state->fpr[frB].dblPrec);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fmsub_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc, uint width) {
	if (width == 8)
		state->fpr[frD].dblPrec = (state->fpr[frA].dblPrec * state->fpr[frC].dblPrec) - state->fpr[frB].dblPrec;
	else if (width == 4) {
		if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
			state->fpr[frD].ps[0] = state->fpr[frD].ps[1] = (state->fpr[frA].ps[0] * state->fpr[frC].ps[0]) - state->fpr[frB].ps[0];
		else
			state->fpr[frD].dblPrec = (float)((state->fpr[frA].dblPrec * state->fpr[frC].dblPrec) - state->fpr[frB].dblPrec);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fdiv_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc, uint width) {
	if (width == 8)
		state->fpr[frD].dblPrec = state->fpr[frA].dblPrec / state->fpr[frB].dblPrec;
	else if (width == 4) {
		if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
			state->fpr[frD].ps[0] = state->fpr[frD].ps[1] = state->fpr[frA].ps[0] / state->fpr[frB].ps[0];
		else
			state->fpr[frD].dblPrec = (float)(state->fpr[frA].dblPrec / state->fpr[frB].dblPrec);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fsub_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc, uint width) {
	if (width == 8)
		state->fpr[frD].dblPrec = state->fpr[frA].dblPrec - state->fpr[frB].dblPrec;
	else if (width == 4) {
		if (state->caps & CAPS_PS_IDX && state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE)
			state->fpr[frD].ps[0] = state->fpr[frD].ps[1] = state->fpr[frA].ps[0] - state->fpr[frB].ps[0];
		else
			state->fpr[frD].dblPrec = (float)(state->fpr[frA].dblPrec - state->fpr[frB].dblPrec);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fcmpu(struct _ppcemu_state *state, uint crfD, uint frA, uint frB) {
	u8 c = 0;
	double a, b;

	a = state->fpr[frA].dblPrec;
	b = state->fpr[frB].dblPrec;
	if (a == NAN || b == NAN)
		c = 1;
	else if (a < b)
		c = 8;
	else if (a > b)
		c = 4;
	else
		c = 2;

	state->fpcsr &= 0x0000f000;
	state->fpcsr |= ((u32)c << 12);
	cr_set_field(state, crfD, c);

	/* TODO: check SNaN? */
}
