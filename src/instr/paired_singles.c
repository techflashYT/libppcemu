/*
 * libppcemu - PowerPC Instruction handling - Paired Singles
 *
 * Copyright (C) 2026 Techflash
 *
 * Quantization and dequantization tables adapted from Dolphin Emulator:
 * Source/Core/Core/PowerPC/Interpreter/Interpreter_LoadStorePaired.cpp
 * Copyright 2008 Dolphin Emulator Project
 */

#define LOG_LEVEL misc_loglevel
#include <math.h>
#include <limits.h>
#include <ppcemu/endian.h>
#include <ppcemu/spr.h>
#include "../caps.h"
#include "../cr.h"
#include "../exception.h"
#include "../fp.h"
#include "../log.h"
#include "../mem.h"
#include "../state.h"

/* dequantize table */
static const float dequantize_table[] = {
    1.0 / (1ULL << 0),  1.0 / (1ULL << 1),  1.0 / (1ULL << 2),  1.0 / (1ULL << 3),
    1.0 / (1ULL << 4),  1.0 / (1ULL << 5),  1.0 / (1ULL << 6),  1.0 / (1ULL << 7),
    1.0 / (1ULL << 8),  1.0 / (1ULL << 9),  1.0 / (1ULL << 10), 1.0 / (1ULL << 11),
    1.0 / (1ULL << 12), 1.0 / (1ULL << 13), 1.0 / (1ULL << 14), 1.0 / (1ULL << 15),
    1.0 / (1ULL << 16), 1.0 / (1ULL << 17), 1.0 / (1ULL << 18), 1.0 / (1ULL << 19),
    1.0 / (1ULL << 20), 1.0 / (1ULL << 21), 1.0 / (1ULL << 22), 1.0 / (1ULL << 23),
    1.0 / (1ULL << 24), 1.0 / (1ULL << 25), 1.0 / (1ULL << 26), 1.0 / (1ULL << 27),
    1.0 / (1ULL << 28), 1.0 / (1ULL << 29), 1.0 / (1ULL << 30), 1.0 / (1ULL << 31),
    (1ULL << 32),       (1ULL << 31),       (1ULL << 30),       (1ULL << 29),
    (1ULL << 28),       (1ULL << 27),       (1ULL << 26),       (1ULL << 25),
    (1ULL << 24),       (1ULL << 23),       (1ULL << 22),       (1ULL << 21),
    (1ULL << 20),       (1ULL << 19),       (1ULL << 18),       (1ULL << 17),
    (1ULL << 16),       (1ULL << 15),       (1ULL << 14),       (1ULL << 13),
    (1ULL << 12),       (1ULL << 11),       (1ULL << 10),       (1ULL << 9),
    (1ULL << 8),        (1ULL << 7),        (1ULL << 6),        (1ULL << 5),
    (1ULL << 4),        (1ULL << 3),        (1ULL << 2),        (1ULL << 1),
};

/* quantize table */
static const float quantize_table[] = {
    (1ULL << 0),        (1ULL << 1),        (1ULL << 2),        (1ULL << 3),
    (1ULL << 4),        (1ULL << 5),        (1ULL << 6),        (1ULL << 7),
    (1ULL << 8),        (1ULL << 9),        (1ULL << 10),       (1ULL << 11),
    (1ULL << 12),       (1ULL << 13),       (1ULL << 14),       (1ULL << 15),
    (1ULL << 16),       (1ULL << 17),       (1ULL << 18),       (1ULL << 19),
    (1ULL << 20),       (1ULL << 21),       (1ULL << 22),       (1ULL << 23),
    (1ULL << 24),       (1ULL << 25),       (1ULL << 26),       (1ULL << 27),
    (1ULL << 28),       (1ULL << 29),       (1ULL << 30),       (1ULL << 31),
    1.0 / (1ULL << 32), 1.0 / (1ULL << 31), 1.0 / (1ULL << 30), 1.0 / (1ULL << 29),
    1.0 / (1ULL << 28), 1.0 / (1ULL << 27), 1.0 / (1ULL << 26), 1.0 / (1ULL << 25),
    1.0 / (1ULL << 24), 1.0 / (1ULL << 23), 1.0 / (1ULL << 22), 1.0 / (1ULL << 21),
    1.0 / (1ULL << 20), 1.0 / (1ULL << 19), 1.0 / (1ULL << 18), 1.0 / (1ULL << 17),
    1.0 / (1ULL << 16), 1.0 / (1ULL << 15), 1.0 / (1ULL << 14), 1.0 / (1ULL << 13),
    1.0 / (1ULL << 12), 1.0 / (1ULL << 11), 1.0 / (1ULL << 10), 1.0 / (1ULL << 9),
    1.0 / (1ULL << 8),  1.0 / (1ULL << 7),  1.0 / (1ULL << 6),  1.0 / (1ULL << 5),
    1.0 / (1ULL << 4),  1.0 / (1ULL << 3),  1.0 / (1ULL << 2),  1.0 / (1ULL << 1),
};

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


#define PS_ENFORCE_CAP_LS_RET(instr, ret) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_FP_UNAV); \
		ret; \
	} \
	if (!(state->caps & CAPS_PS_LD_ST)) { \
		warn(instr " on CPU w/o PS Load/Store support"); \
		exception_fire(state, EXCEPTION_PROGRAM); \
		ret; \
	} \
	if (state->caps & CAPS_HID2_GEKKO) { \
		hid2 = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)]; \
		if ((u32)(hid2 & (PPCEMU_HID2_PSE | PPCEMU_HID2_BW_LSQE)) != (u32)(PPCEMU_HID2_PSE | PPCEMU_HID2_BW_LSQE)) { \
			exception_fire(state, EXCEPTION_PROGRAM); \
			ret; \
		} \
	}

#define PS_ENFORCE_CAP_IDX_RET(instr, ret) \
	if (!(state->msr & PPCEMU_MSR_FP)) { \
		exception_fire(state, EXCEPTION_FP_UNAV); \
		ret; \
	} \
	if (!(state->caps & CAPS_PS_IDX)) { \
		warn(instr " on CPU w/o PS Indexed support"); \
		exception_fire(state, EXCEPTION_PROGRAM); \
		ret; \
	} \
	if (state->caps & CAPS_HID2_GEKKO) { \
		hid2 = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)]; \
		if (!(hid2 & PPCEMU_HID2_PSE)) { \
			exception_fire(state, EXCEPTION_PROGRAM); \
			ret; \
		} \
	}

#define PS_ENFORCE_CAP_IDX(instr) PS_ENFORCE_CAP_IDX_RET(instr, return)
#define PS_ENFORCE_CAP_LS(instr) PS_ENFORCE_CAP_LS_RET(instr, return)


u32 do_psq_l(struct _ppcemu_state *state, uint frD, uint rA, uint W, uint PSQ, u16 d) {
	u32 hid2, b, ea, gqr, ld_scale;
	i32 di32;
	u16 u16Val;
	i16 di16, i16Val;
	u8 u8Val;
	i8 i8Val;
	enum ppcemu_gqr_quantization_type ld_type;
	enum virt2phys_err v2p_err;
	union ps_bits ps0, ps1;

	PS_ENFORCE_CAP_LS_RET("psq_l", return 0);
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
	ld_scale = (gqr & PPCEMU_GQR_LD_SCALE) >> PPCEMU_GQR_LD_SCALE_SHIFT;

	if (W) /* read unpaired */
		ps1.f = 1.0f;

	switch (ld_type) {
	case PPCEMU_GQR_QUANTIZATION_SINGLE: {
		v2p_err = _do_basic_load(state, 4, ea, &ps0.u);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (!W) {
			v2p_err = _do_basic_load(state, 4, ea + 4, &ps1.u);
			if (v2p_err != V2P_SUCCESS)
				return 0;
			ps1.u = ppcemu_be32_to_cpu(ps1.u);
		}

		ps_set_u32(state, frD, PS_LANE_0, ppcemu_be32_to_cpu(ps0.u));
		ps_set_u32(state, frD, PS_LANE_1, ps1.u);
		state->fpr_is_ps[frD] = true;
		break;
	}
	case PPCEMU_GQR_QUANTIZATION_U8: {
		v2p_err = _do_basic_load(state, 1, ea, &u8Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;

		ps0.f = ((float)u8Val) * dequantize_table[ld_scale];
		if (!W) {
			v2p_err = _do_basic_load(state, 1, ea + 1, &u8Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
			ps1.f = ((float)u8Val) * dequantize_table[ld_scale];
		}

		ps_set_f32(state, frD, PS_LANE_0, ps0.f);
		ps_set_f32(state, frD, PS_LANE_1, ps1.f);
		state->fpr_is_ps[frD] = true;
		break;
	}
	case PPCEMU_GQR_QUANTIZATION_U16: {
		v2p_err = _do_basic_load(state, 2, ea, &u16Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;

		u16Val = ppcemu_be16_to_cpu(u16Val);
		ps0.f = ((float)u16Val) * dequantize_table[ld_scale];
		if (!W) {
			v2p_err = _do_basic_load(state, 2, ea + 2, &u16Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
			u16Val = ppcemu_be16_to_cpu(u16Val);
			ps1.f = ((float)u16Val) * dequantize_table[ld_scale];
		}

		ps_set_f32(state, frD, PS_LANE_0, ps0.f);
		ps_set_f32(state, frD, PS_LANE_1, ps1.f);
		state->fpr_is_ps[frD] = true;
		break;
	}
	case PPCEMU_GQR_QUANTIZATION_I8: {
		v2p_err = _do_basic_load(state, 1, ea, &i8Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;

		ps0.f = ((float)i8Val) * dequantize_table[ld_scale];
		if (!W) {
			v2p_err = _do_basic_load(state, 1, ea + 1, &i8Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
			ps1.f = ((float)i8Val) * dequantize_table[ld_scale];
		}

		ps_set_f32(state, frD, PS_LANE_0, ps0.f);
		ps_set_f32(state, frD, PS_LANE_1, ps1.f);
		state->fpr_is_ps[frD] = true;
		break;
	}
	case PPCEMU_GQR_QUANTIZATION_I16: {
		v2p_err = _do_basic_load(state, 2, ea, &i16Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;

		i16Val = ppcemu_be16_to_cpu(i16Val);
		ps0.f = ((float)i16Val) * dequantize_table[ld_scale];
		if (!W) {
			v2p_err = _do_basic_load(state, 2, ea + 2, &i16Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
			i16Val = ppcemu_be16_to_cpu(i16Val);
			ps1.f = ((float)i16Val) * dequantize_table[ld_scale];
		}

		ps_set_f32(state, frD, PS_LANE_0, ps0.f);
		ps_set_f32(state, frD, PS_LANE_1, ps1.f);
		state->fpr_is_ps[frD] = true;
		break;
	}
	default: {
		error("Unimplemented psq_l quantization: %u\r\n", ld_type);
		exception_fire(state, EXCEPTION_PROGRAM);
		break;
	}
	}

	return ea;
}

u32 do_psq_st(struct _ppcemu_state *state, uint frS, uint rA, uint W, uint PSQ, u16 d) {
	u32 hid2, b, ea, gqr, st_scale;
	i32 di32;
	u16 u16Val;
	i16 di16, i16Val;
	u8 u8Val;
	i8 i8Val;
	enum ppcemu_gqr_quantization_type st_type;
	enum virt2phys_err v2p_err;
	union ps_bits ps0, ps1;

	PS_ENFORCE_CAP_LS_RET("psq_st", return 0);
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
	st_scale = (gqr & PPCEMU_GQR_ST_SCALE) >> PPCEMU_GQR_ST_SCALE_SHIFT;

	switch (st_type) {
	case PPCEMU_GQR_QUANTIZATION_SINGLE: {
		ps0.u = ppcemu_cpu_to_be32(ps_get_u32(state, frS, PS_LANE_0));
		if (!W)
			ps1.u = ppcemu_cpu_to_be32(ps_get_u32(state, frS, PS_LANE_1));

		v2p_err = _do_basic_store(state, 4, ea, &ps0.u);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (!W) {
			v2p_err = _do_basic_store(state, 4, ea + 4, &ps1.u);
			if (v2p_err != V2P_SUCCESS)
				return 0;
		}

		break;
	}
	case PPCEMU_GQR_QUANTIZATION_U8: {
		ps0.f = ps_get_f32(state, frS, PS_LANE_0);
		ps0.f *= quantize_table[st_scale];

		if (ps0.f > UCHAR_MAX)
			ps0.f = (float)UCHAR_MAX;
		if (ps0.f < 0.0f)
			ps0.f = 0.0f;

		u8Val = (u8)ps0.f;
		if (!W) {
			ps1.f = ps_get_f32(state, frS, PS_LANE_1);
			ps1.f *= quantize_table[st_scale];

			if (ps1.f > UCHAR_MAX)
				ps1.f = (float)UCHAR_MAX;
			if (ps1.f < 0.0f)
				ps1.f = 0.0f;
		}

		v2p_err = _do_basic_store(state, 1, ea, &u8Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (!W) {
			u8Val = (u8)ps1.f;
			v2p_err = _do_basic_store(state, 1, ea + 1, &u8Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
		}

		break;
	}
	case PPCEMU_GQR_QUANTIZATION_I8: {
		ps0.f = ps_get_f32(state, frS, PS_LANE_0);
		ps0.f *= quantize_table[st_scale];

		if (ps0.f > SCHAR_MAX)
			ps0.f = (float)SCHAR_MAX;
		if (ps0.f < SCHAR_MIN)
			ps0.f = SCHAR_MIN;

		i8Val = (i8)ps0.f;
		if (!W) {
			ps1.f = ps_get_f32(state, frS, PS_LANE_1);
			ps1.f *= quantize_table[st_scale];

			if (ps1.f > SCHAR_MAX)
				ps1.f = (float)SCHAR_MAX;
			if (ps1.f < SCHAR_MIN)
				ps1.f = (float)SCHAR_MIN;
		}

		v2p_err = _do_basic_store(state, 1, ea, &i8Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (!W) {
			i8Val = (i8)ps1.f;
			v2p_err = _do_basic_store(state, 1, ea + 1, &i8Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
		}

		break;
	}
	case PPCEMU_GQR_QUANTIZATION_U16: {
		ps0.f = ps_get_f32(state, frS, PS_LANE_0);
		ps0.f *= quantize_table[st_scale];

		if (ps0.f > USHRT_MAX)
			ps0.f = (float)USHRT_MAX;
		if (ps0.f < 0.0f)
			ps0.f = 0.0f;

		u16Val = ppcemu_cpu_to_be16((u16)ps0.f);
		if (!W) {
			ps1.f = ps_get_f32(state, frS, PS_LANE_1);
			ps1.f *= quantize_table[st_scale];

			if (ps1.f > USHRT_MAX)
				ps1.f = (float)USHRT_MAX;
			if (ps1.f < 0.0f)
				ps1.f = 0.0f;
		}

		v2p_err = _do_basic_store(state, 2, ea, &u16Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (!W) {
			u16Val = ppcemu_cpu_to_be16((u16)ps1.f);
			v2p_err = _do_basic_store(state, 2, ea + 2, &u16Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
		}

		break;
	}
	case PPCEMU_GQR_QUANTIZATION_I16: {
		ps0.f = ps_get_f32(state, frS, PS_LANE_0);
		ps0.f *= quantize_table[st_scale];

		if (ps0.f > SHRT_MAX)
			ps0.f = (float)SHRT_MAX;
		if (ps0.f < SHRT_MIN)
			ps0.f = SHRT_MIN;

		i16Val = ppcemu_cpu_to_be16((i16)ps0.f);
		if (!W) {
			ps1.f = ps_get_f32(state, frS, PS_LANE_1);
			ps1.f *= quantize_table[st_scale];

			if (ps1.f > SHRT_MAX)
				ps1.f = (float)SHRT_MAX;
			if (ps1.f < SHRT_MIN)
				ps1.f = (float)SHRT_MIN;
		}

		v2p_err = _do_basic_store(state, 2, ea, &i16Val);
		if (v2p_err != V2P_SUCCESS)
			return 0;
		if (!W) {
			i16Val = ppcemu_cpu_to_be16((i16)ps1.f);
			v2p_err = _do_basic_store(state, 2, ea + 2, &i16Val);
			if (v2p_err != V2P_SUCCESS)
				return 0;
		}

		break;
	}
	default: {
		error("Unimplemented psq_st quantization: %u\r\n", st_type);
		exception_fire(state, EXCEPTION_PROGRAM);
		break;
	}
	}

	return ea;
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

void do_ps_sum0(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2, c1;
	float a0, b1, d0;

	PS_ENFORCE_CAP_IDX("ps_sum1");

	/* doesn't need to be loaded into host FPR as we do no math with it */
	c1 = ps_get_u32(state, frC, PS_LANE_1);
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	d0 = a0 + b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_u32(state, frD, PS_LANE_1, c1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}


void do_ps_sum1(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2, c0;
	float a0, b1, d1;

	PS_ENFORCE_CAP_IDX("ps_sum1");

	/* doesn't need to be loaded into host FPR as we do no math with it */
	c0 = ps_get_u32(state, frC, PS_LANE_0);
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	d1 = a0 + b1;
	ps_set_u32(state, frD, PS_LANE_0, c0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_muls0(struct _ppcemu_state *state, uint frD, uint frA, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, c0, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_muls0");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	d0 = a0 * c0;
	d1 = a1 * c0;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_muls1(struct _ppcemu_state *state, uint frD, uint frA, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_muls1");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = a0 * c1;
	d1 = a1 * c1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_madds0(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, c0, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_madds0");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	d0 = (a0 * c0) + b0;
	d1 = (a1 * c0) + b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_madds1(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_madds1");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = (a0 * c1) + b0;
	d1 = (a1 * c1) + b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_mul(struct _ppcemu_state *state, uint frD, uint frA, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, c0, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_mul");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = a0 * c0;
	d1 = a1 * c1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_madd(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, c0, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_madd");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = (a0 * c0) + b0;
	d1 = (a1 * c1) + b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_msub(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, c0, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_msub");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = (a0 * c0) - b0;
	d1 = (a1 * c1) - b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_neg(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	u32 hid2;
	u32 b0, b1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_msub");
	b0 = ps_get_u32(state, frB, PS_LANE_0);
	b1 = ps_get_u32(state, frB, PS_LANE_1);
	d0 = b0 ^ (1u << 31);
	d1 = b1 ^ (1u << 31);
	ps_set_u32(state, frD, PS_LANE_0, d0);
	ps_set_u32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_sub(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_sub");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	d0 = a0 - b0;
	d1 = a1 - b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}


void do_ps_cmpo0(struct _ppcemu_state *state, uint crfD, uint frA, uint frB) {
	u32 hid2;
	u8 c;
	union ps_bits a0, b0;

	PS_ENFORCE_CAP_IDX("ps_cmpo0");
	a0.f = ps_get_f32(state, frA, PS_LANE_0);
	b0.f = ps_get_f32(state, frB, PS_LANE_0);

	if (IS_NAN(a0.u) || IS_NAN(b0.u)) {
		c = 1;

		if (IS_SNAN(a0.u) || IS_SNAN(b0.u)) {
			state->fpcsr |= FPSCR_VXSNAN;
			if ((state->fpcsr & FPSCR_VE) == 0)
				state->fpcsr |= FPSCR_VXVC;
		}
		else if (IS_QNAN(a0.u) || IS_QNAN(b0.u))
			state->fpcsr |= FPSCR_VXVC;
	}
	else if (a0.f < b0.f)
		c = 8;
	else if (a0.f > b0.f)
		c = 4;
	else
		c = 2;

	state->fpcsr &= ~0x0000f000u;
	state->fpcsr |= ((u32)c << 12);
	cr_set_field(state, crfD, c);
}

void do_ps_add(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_add");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	d0 = a0 + b0;
	d1 = a1 + b1;
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_nmsub(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, c0, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_nmsub");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = -((a0 * c0) - b0);
	d1 = -((a1 * c1) - b1);
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_nmadd(struct _ppcemu_state *state, uint frD, uint frA, uint frB, uint frC, uint Rc) {
	u32 hid2;
	float a0, a1, b0, b1, c0, c1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_nmadd");
	a0 = ps_get_f32(state, frA, PS_LANE_0);
	a1 = ps_get_f32(state, frA, PS_LANE_1);
	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	c0 = ps_get_f32(state, frC, PS_LANE_0);
	c1 = ps_get_f32(state, frC, PS_LANE_1);
	d0 = -((a0 * c0) + b0);
	d1 = -((a1 * c1) + b1);
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}

void do_ps_rsqrte(struct _ppcemu_state *state, uint frD, uint frB, uint Rc) {
	u32 hid2;
	float b0, b1, d0, d1;

	PS_ENFORCE_CAP_IDX("ps_rsqrte");

	b0 = ps_get_f32(state, frB, PS_LANE_0);
	b1 = ps_get_f32(state, frB, PS_LANE_1);
	d0 = 1.0 / sqrt(b0);
	d1 = 1.0 / sqrt(b1);
	ps_set_f32(state, frD, PS_LANE_0, d0);
	ps_set_f32(state, frD, PS_LANE_1, d1);

	/* TODO: Update CR1 if Rc */
	(void)Rc;
}
