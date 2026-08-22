/*
 * libppcemu - PowerPC Instruction handling - Paired Singles
 *
 * Copyright (C) 2026 Techflash
 */

#define LOG_LEVEL misc_loglevel
#include <assert.h>
#include <ppcemu/endian.h>
#include <ppcemu/spr.h>
#include "../caps.h"
#include "../exception.h"
#include "../log.h"
#include "../mem.h"
#include "../state.h"

/*
 * Paired Single lane access.  ps0 lives in the high 32 bits of the FPR's
 * u64, ps1 in the low 32 bits, per the PPC bit-numbering.
 */
union ps_bits {
	u32 u;
	float f;
};

enum ps_lane {
	PS_LANE_0 = 0,
	PS_LANE_1 = 1,
};

static inline u32 ps_get_u32(struct _ppcemu_state *state, uint fr, enum ps_lane lane) {
	if (lane == PS_LANE_0)
		return (u32)(state->fpr[fr].u64 >> 32);

	return (u32)state->fpr[fr].u64;
}

static inline float ps_get_f32(struct _ppcemu_state *state, uint fr, enum ps_lane lane) {
	union ps_bits b;

	b.u = ps_get_u32(state, fr, lane);
	return b.f;
}

static inline void ps_set_u32(struct _ppcemu_state *state, uint fr, enum ps_lane lane, u32 val) {
	if (lane == PS_LANE_0)
		state->fpr[fr].u64 = ((u64)val << 32) | (u32)state->fpr[fr].u64;
	else
		state->fpr[fr].u64 = (state->fpr[fr].u64 & 0xffffffff00000000ULL) | val;
}

static inline void ps_set_f32(struct _ppcemu_state *state, uint fr, enum ps_lane lane, float val) {
	union ps_bits b;

	b.f = val;
	ps_set_u32(state, fr, lane, b.u);
}

#define PS_ENFORCE_CAP_LS(instr) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_FP_UNAV); \
		return; \
	} \
	if (!(state->caps & CAPS_PS_LD_ST)) { \
		warn(instr " on CPU w/o PS Load/Store support"); \
		exception_fire(state, EXCEPTION_PROGRAM); \
		return; \
	} \
	if (state->caps & CAPS_HID2_GEKKO) { \
		hid2 = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)]; \
		if ((u32)(hid2 & (PPCEMU_HID2_PSE | PPCEMU_HID2_BW_LSQE)) != (u32)(PPCEMU_HID2_PSE | PPCEMU_HID2_BW_LSQE)) { \
			exception_fire(state, EXCEPTION_PROGRAM); \
			return; \
		} \
	}

#define PS_ENFORCE_CAP_IDX(instr) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_FP_UNAV); \
		return; \
	} \
	if (!(state->caps & CAPS_PS_IDX)) { \
		warn(instr " on CPU w/o PS Indexed support"); \
		exception_fire(state, EXCEPTION_PROGRAM); \
		return; \
	} \
	if (state->caps & CAPS_HID2_GEKKO) { \
		hid2 = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)]; \
		if (!(hid2 & PPCEMU_HID2_PSE)) { \
			exception_fire(state, EXCEPTION_PROGRAM); \
			return; \
		} \
	}


void do_psq_l(struct _ppcemu_state *state, uint frD, uint rA, uint W, uint PSQ, u16 d) {
	u32 hid2, b, ea, gqr, val_hi, val_lo;
	i16 di16;
	i32 di32;
	enum ppcemu_gqr_quantization_type ld_type;
	enum virt2phys_err v2p_err;

	PS_ENFORCE_CAP_LS("psq_l");
	di16 = (i16)(d << 4);
	di16 >>= 4; /* sign extend 12->16 bit */
	di32 = (i32)di16; /* sign-extend 16->32 bit */

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + di32;

	/* determine load quantization type */
	gqr = state->sprs[ppcemu_gqrn_to_spr_idx(PSQ)];
	ld_type = (enum ppcemu_gqr_quantization_type)((gqr & PPCEMU_GQR_LD_TYPE) >> PPCEMU_GQR_LD_TYPE_SHIFT);

	if (W) { /* read unpaired */
		assert(!"Unimplemented");
	}
	else { /* read 2 paired single precision floats */
		switch (ld_type) {
		case PPCEMU_GQR_QUANTIZATION_SINGLE: {
			v2p_err = _do_basic_load(state, 4, ea, &val_hi);
			if (v2p_err != V2P_SUCCESS)
				return;
			v2p_err = _do_basic_load(state, 4, ea + 4, &val_lo);
			if (v2p_err != V2P_SUCCESS)
				return;

			ps_set_u32(state, frD, PS_LANE_0, ppcemu_be32_to_cpu(val_hi));
			ps_set_u32(state, frD, PS_LANE_1, ppcemu_be32_to_cpu(val_lo));
			state->fpr_is_ps[frD] = true;
			break;
		}
		default:
			assert(!"Unimplemented");
			break;
		}
	}
}

void do_psq_st(struct _ppcemu_state *state, uint frS, uint rA, uint W, uint PSQ, u16 d) {
	u32 hid2, b, ea, gqr, val_hi, val_lo;
	i16 di16;
	i32 di32;
	enum ppcemu_gqr_quantization_type st_type;
	enum virt2phys_err v2p_err;

	PS_ENFORCE_CAP_LS("psq_st");
	di16 = (i16)(d << 4);
	di16 >>= 4; /* sign extend 12->16 bit */
	di32 = (i32)di16; /* sign-extend 16->32 bit */

	if (rA)
		b = state->gpr[rA];
	else
		b = 0;

	ea = b + di32;

	/* determine load quantization type */
	gqr = state->sprs[ppcemu_gqrn_to_spr_idx(PSQ)];
	st_type = (enum ppcemu_gqr_quantization_type)((gqr & PPCEMU_GQR_ST_TYPE) >> PPCEMU_GQR_ST_TYPE_SHIFT);

	if (W) { /* read unpaired */
		assert(!"Unimplemented");
	}
	else { /* store 2 paired single precision floats */
		switch (st_type) {
		case PPCEMU_GQR_QUANTIZATION_SINGLE: {
			val_hi = ppcemu_cpu_to_be32(ps_get_u32(state, frS, PS_LANE_0));
			val_lo = ppcemu_cpu_to_be32(ps_get_u32(state, frS, PS_LANE_1));
			v2p_err = _do_basic_store(state, 4, ea, &val_hi);
			if (v2p_err != V2P_SUCCESS)
				return;
			v2p_err = _do_basic_store(state, 4, ea + 4, &val_lo);
			if (v2p_err != V2P_SUCCESS)
				return;

			break;
		}
		default:
			assert(!"Unimplemented");
			break;
		}
	}
}

void do_ps_mr(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	u32 hid2;

	PS_ENFORCE_CAP_IDX("ps_mr");
	state->fpr[frD] = state->fpr[frB]; /* technically it's broken up into 2 moves but this is functionally what it does */
	state->fpr_is_ps[frD] = true;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_merge00(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc) {
	u32 hid2, a0, b0;

	PS_ENFORCE_CAP_IDX("ps_merge00");
	a0 = ps_get_u32(state, frA, PS_LANE_0);
	b0 = ps_get_u32(state, frB, PS_LANE_0);
	ps_set_u32(state, frD, PS_LANE_0, a0);
	ps_set_u32(state, frD, PS_LANE_1, b0);
	state->fpr_is_ps[frD] = true;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_merge01(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc) {
	u32 hid2, a0, b1;

	PS_ENFORCE_CAP_IDX("ps_merge01");
	a0 = ps_get_u32(state, frA, PS_LANE_0);
	b1 = ps_get_u32(state, frB, PS_LANE_1);
	ps_set_u32(state, frD, PS_LANE_0, a0);
	ps_set_u32(state, frD, PS_LANE_1, b1);
	state->fpr_is_ps[frD] = true;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_merge10(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc) {
	u32 hid2, a1, b0;

	PS_ENFORCE_CAP_IDX("ps_merge10");
	a1 = ps_get_u32(state, frA, PS_LANE_1);
	b0 = ps_get_u32(state, frB, PS_LANE_0);
	ps_set_u32(state, frD, PS_LANE_0, a1);
	ps_set_u32(state, frD, PS_LANE_1, b0);
	state->fpr_is_ps[frD] = true;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_merge11(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc) {
	u32 hid2, a1, b1;

	PS_ENFORCE_CAP_IDX("ps_merge11");
	a1 = ps_get_u32(state, frA, PS_LANE_1);
	b1 = ps_get_u32(state, frB, PS_LANE_1);
	ps_set_u32(state, frD, PS_LANE_0, a1);
	ps_set_u32(state, frD, PS_LANE_1, b1);
	state->fpr_is_ps[frD] = true;

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}
