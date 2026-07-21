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
	u32 u32[2];
	float singleFloat[2];
	double doubleFloat;
};

static bool paired_single_mode(const struct _ppcemu_state *state) {
	return (state->caps & CAPS_PS_IDX) &&
	       (state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_PSE);
}

/* ps0 is the high-order word */
static float get_ps0(const struct _ppcemu_state *state, uint fr) {
	union fp_val val;
	val.u32[0] = (u32)(state->fpr[fr].u64 >> 32);
	return val.singleFloat[0];
}

static void set_ps(struct _ppcemu_state *state, uint fr, float ps0, float ps1) {
	union fp_val hi, lo;
	hi.singleFloat[0] = ps0;
	lo.singleFloat[0] = ps1;
	state->fpr[fr].u64 = ((u64)hi.u32[0] << 32) | lo.u32[0];
	state->fpr_is_ps[fr] = true;
}

static double get_double(const struct _ppcemu_state *state, uint fr) {
	/* ps0 is accepted by DP instructions */
	return state->fpr_is_ps[fr] ? (double)get_ps0(state, fr) : state->fpr[fr].dblPrec;
}

static void set_double(struct _ppcemu_state *state, uint fr, double val) {
	state->fpr[fr].dblPrec = val;
	state->fpr_is_ps[fr] = false;
}

#define ENFORCE_MSR_FP(x) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_FP_UNAV); \
		return x; \
	} \

u32 do_lf_common(struct _ppcemu_state *state, uint frD, uint rA, i32 d, uint width) {
	u32 b, ea;
	union fp_val val;
	enum virt2phys_err v2p_err;

	ENFORCE_MSR_FP(0);

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + d;

	v2p_err = _do_basic_load(state, 4, ea, &val.u32[0]);
	if (v2p_err != V2P_SUCCESS)
		return 0;
	if (width == 8) {
		v2p_err = _do_basic_load(state, 4, ea + 4, &val.u32[1]);
		if (v2p_err != V2P_SUCCESS)
			return 0;
	}

	if (width == 4) {
		if (paired_single_mode(state)) {
			/* load into both PS slots */
			val.u32[0] = ppcemu_be32_to_cpu(val.u32[0]);
			set_ps(state, frD, val.singleFloat[0], val.singleFloat[0]);
		}
		else {
			/* byteswap if needed */
			val.u32[0] = ppcemu_be32_to_cpu(val.u32[0]);

			/* reinterpret as single precision float, cast to double */
			val.doubleFloat = (double)val.singleFloat[0];

			/* store that double back into the FPR */
			state->fpr[frD].u64 = val.u64;
			state->fpr_is_ps[frD] = false;
		}
	}
	else if (width == 8) {
		state->fpr[frD].u64 = ((u64)ppcemu_be32_to_cpu(val.u32[0]) << 32) |
					ppcemu_be32_to_cpu(val.u32[1]);
		state->fpr_is_ps[frD] = false;
	}

	return ea;
}

u32 do_stf_common(struct _ppcemu_state *state, uint frS, uint rA, i32 d, uint width, bool convert) {
	u32 b, ea, word;
	union fp_val val;
	enum virt2phys_err v2p_err;

	ENFORCE_MSR_FP(0);

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + d;

	if (convert && width == 4) {
		/* In PS mode stfs stores ps0; otherwise it converts a double. */
		val.singleFloat[0] = paired_single_mode(state) ? get_ps0(state, frS) : (float)get_double(state, frS);
		word = ppcemu_cpu_to_be32(val.u32[0]);
		v2p_err = _do_basic_store(state, 4, ea, &word);
		if (v2p_err != V2P_SUCCESS)
			return 0;
	}
	else if (width == 8) {
		if (state->fpr_is_ps[frS]) {
			val.doubleFloat = get_double(state, frS);
			word = ppcemu_cpu_to_be32((u32)(val.u64 >> 32));
		} else
			word = ppcemu_cpu_to_be32((u32)(state->fpr[frS].u64 >> 32));

		/* stfd: high-order word first */
		v2p_err = _do_basic_store(state, 4, ea, &word);
		if (v2p_err != V2P_SUCCESS)
			return 0;

		word = ppcemu_cpu_to_be32((u32)(state->fpr_is_ps[frS] ? val.u64 : state->fpr[frS].u64));
		v2p_err = _do_basic_store(state, 4, ea + 4, &word);
		if (v2p_err != V2P_SUCCESS)
			return 0;
	}
	else {
		/* stfiwx: store the low-order 32 bits verbatim */
		word = ppcemu_cpu_to_be32((u32)state->fpr[frS].u64);
		v2p_err = _do_basic_store(state, 4, ea, &word);
		if (v2p_err != V2P_SUCCESS)
			return 0;
	}

	return ea;
}

void do_fmr(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	ENFORCE_MSR_FP();

	if (paired_single_mode(state) && state->fpr_is_ps[frB]) {
		/* in PS mode fmr copies ps0 and leaves the destination ps1 unchanged */
		state->fpr[frD].u64 = (state->fpr[frB].u64 & 0xffffffff00000000ull) |
					 (state->fpr[frD].u64 & 0xffffffffull);
		state->fpr_is_ps[frD] = true;
	} else {
		state->fpr[frD].u64 = state->fpr[frB].u64;
		state->fpr_is_ps[frD] = state->fpr_is_ps[frB];
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_mtfsf(struct _ppcemu_state *state, uint FM, uint frB, uint Rc) {
	u32 mask;
	uint i;

	ENFORCE_MSR_FP();

	if (FM == 0xff) /* fast path for "set all" */
		state->fpcsr = (u32)state->fpr[frB].u64;
	else {
		/* actually mask it */
		mask = 0;
		for (i = 0; i < 8; i++) {
			if (FM & (1 << i))
				mask |= (0xf << (i * 4));
		}

		state->fpcsr &= ~mask;
		state->fpcsr |= ((u32)state->fpr[frB].u64 & mask);
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}


void do_mtfsbN(struct _ppcemu_state *state, uint crbD, uint set, uint Rc) {
	u32 mask;

	ENFORCE_MSR_FP();

	mask = 1 << (31 - crbD);
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

	rounded = trunc(get_double(state, frB));
	asi32 = (i32)rounded;

	state->fpr[frD].u64 = (state->fpr[frD].u64 & 0xffffffff00000000ull) | (u32)asi32;
	state->fpr_is_ps[frD] = false;
	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_frsp(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	float rounded;

	ENFORCE_MSR_FP();

	/* TODO: should check mode via FPSCR[RN] */
	rounded = (float)get_double(state, frB);

	if (paired_single_mode(state)) {
		/* frsp defines ps0 only; the 750CL UM explicitly leaves ps1 undefined */
		union fp_val val;
		val.singleFloat[0] = rounded;
		state->fpr[frD].u64 = ((u64)val.u32[0] << 32) | (u32)state->fpr[frD].u64;
		state->fpr_is_ps[frD] = true;
	}
	else
		set_double(state, frD, (double)rounded);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fmul_common(struct _ppcemu_state *state, uint frD, uint frA, uint frC, uint Rc, uint width) {
	ENFORCE_MSR_FP();

	if (width == 8)
		set_double(state, frD, get_double(state, frA) * get_double(state, frC));
	else if (width == 4) {
		if (paired_single_mode(state)) {
			float result = get_ps0(state, frA) * get_ps0(state, frC);
			set_ps(state, frD, result, result);
		}
		else
			set_double(state, frD, (float)(get_double(state, frA) * get_double(state, frC)));
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fadd_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc, uint width) {
	ENFORCE_MSR_FP();

	if (width == 8)
		set_double(state, frD, get_double(state, frA) + get_double(state, frB));
	else if (width == 4) {
		if (paired_single_mode(state)) {
			float result = get_ps0(state, frA) + get_ps0(state, frB);
			set_ps(state, frD, result, result);
		}
		else
			set_double(state, frD, (float)(get_double(state, frA) + get_double(state, frB)));
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fmadd_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc, uint width) {
	ENFORCE_MSR_FP();

	if (width == 8)
		set_double(state, frD, (get_double(state, frA) * get_double(state, frC)) + get_double(state, frB));
	else if (width == 4) {
		if (paired_single_mode(state)) {
			float result = (get_ps0(state, frA) * get_ps0(state, frC)) + get_ps0(state, frB);
			set_ps(state, frD, result, result);
		}
		else
			set_double(state, frD, (float)((get_double(state, frA) * get_double(state, frC)) + get_double(state, frB)));
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fmsub_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc, uint width) {
	ENFORCE_MSR_FP();

	if (width == 8)
		set_double(state, frD, (get_double(state, frA) * get_double(state, frC)) - get_double(state, frB));
	else if (width == 4) {
		if (paired_single_mode(state)) {
			float result = (get_ps0(state, frA) * get_ps0(state, frC)) - get_ps0(state, frB);
			set_ps(state, frD, result, result);
		}
		else
			set_double(state, frD, (float)((get_double(state, frA) * get_double(state, frC)) - get_double(state, frB)));
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fdiv_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc, uint width) {
	ENFORCE_MSR_FP();

	if (width == 8)
		set_double(state, frD, get_double(state, frA) / get_double(state, frB));
	else if (width == 4) {
		if (paired_single_mode(state)) {
			float result = get_ps0(state, frA) / get_ps0(state, frB);
			set_ps(state, frD, result, result);
		}
		else
			set_double(state, frD, (float)(get_double(state, frA) / get_double(state, frB)));
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fsub_common(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc, uint width) {
	ENFORCE_MSR_FP();

	if (width == 8)
		set_double(state, frD, get_double(state, frA) - get_double(state, frB));
	else if (width == 4) {
		if (paired_single_mode(state)) {
			float result = get_ps0(state, frA) - get_ps0(state, frB);
			set_ps(state, frD, result, result);
		}
		else
			set_double(state, frD, (float)(get_double(state, frA) - get_double(state, frB)));
	}

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_fcmpu(struct _ppcemu_state *state, uint crfD, uint frA, uint frB) {
	u8 c = 0;
	double a, b;

	ENFORCE_MSR_FP();

	a = get_double(state, frA);
	b = get_double(state, frB);
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

void do_fneg(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	ENFORCE_MSR_FP();

	state->fpr[frD].u64 = state->fpr[frB].u64 ^ (1ull << 63);
	state->fpr_is_ps[frD] = state->fpr_is_ps[frB];

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_mffs(struct _ppcemu_state *state, uint frD, uint Rc) {
	ENFORCE_MSR_FP();

	state->fpr[frD].u64 = (state->fpr[frD].u64 & 0xffffffff00000000ull) | state->fpcsr;
	state->fpr_is_ps[frD] = false;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}
