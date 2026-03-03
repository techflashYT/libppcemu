/*
 * libppcemu - PowerPC Instruction handling - Paired Singles
 *
 * Copyright (C) 2026 Techflash
 */

#include <assert.h>
#include <ppcemu/endian.h>
#include <ppcemu/spr.h>
#include "../caps.h"
#include "../exception.h"
#include "../mem.h"
#include "../state.h"
#include "../../config.h"

#define PS_ENFORCE_CAP_LS(instr) \
	if (!(state->caps & CAPS_PS_LD_ST)) { \
		warn(instr " on CPU w/o PS Load/Store support"); \
		exception_fire(state, EXCEPTION_PROGRAM); \
		return; \
	} \
	if (state->caps & CAPS_HID2_GEKKO) { \
		hid2 = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)]; \
		if ((hid2 & (PPCEMU_HID2_PSE | PPCEMU_HID2_BW_LSQE)) != (PPCEMU_HID2_PSE | PPCEMU_HID2_BW_LSQE)) { \
			exception_fire(state, EXCEPTION_PROGRAM); \
			return; \
		} \
	}

#define PS_ENFORCE_CAP_IDX(instr) \
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


static void do_psq_l(struct _ppcemu_state *state, uint frD, uint rA, uint W, uint PSQ, u16 d) {
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

			state->fpr[frD] = (u64)ppcemu_be32_to_cpu(val_hi) << 32;
			state->fpr[frD] |= ppcemu_be32_to_cpu(val_lo);
			break;
		}
		default:
			assert(!"Unimplemented");
			break;
		}
	}
}
