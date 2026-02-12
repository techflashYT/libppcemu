/*
 * libppcemu - PowerPC Instruction handling - Bitwise operations
 *
 * Copyright (C) 2026 Techflash
 */

#include <assert.h>
#include <stdio.h>
#include <ppcemu/spr.h>
#include "../caps.h"
#include "../exception.h"
#include "../spr.h"
#include "../state.h"

static void do_mtspr(struct _ppcemu_state *state, uint rS, uint sprn, uint Rc) {
	switch (sprn) {
	case PPCEMU_SPRN_LR:
	case PPCEMU_SPRN_CTR:
	case PPCEMU_SPRN_DEC:
	case PPCEMU_SPRN_TBL_OEA:
	case PPCEMU_SPRN_TBU_OEA:
	case PPCEMU_SPRN_SRR0:
	case PPCEMU_SPRN_SRR1:
	case PPCEMU_SPRN_SPRG0:
	case PPCEMU_SPRN_SPRG1:
	case PPCEMU_SPRN_SPRG2:
	case PPCEMU_SPRN_SPRG3: {
		state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];
		break;
	}
	case PPCEMU_SPRN_HID2_GEKKO: {
		if (!(state->caps & CAPS_HID2_GEKKO)) {
			puts("Attempted to write to HID2-Gekko on unsupported CPU");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	case PPCEMU_SPRN_HID4: {
		if (!(state->caps & CAPS_HID4)) {
			puts("Attempted to write to HID4 on unsupported CPU");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	default:
		assert(!"Unknown MSR");
	}
}

static void do_mfspr(struct _ppcemu_state *state, uint rS, uint sprn, uint Rc) {
	switch (sprn) {
	case PPCEMU_SPRN_LR:
	case PPCEMU_SPRN_CTR:
	case PPCEMU_SPRN_DEC:
	case PPCEMU_SPRN_TBL_OEA:
	case PPCEMU_SPRN_TBU_OEA:
	case PPCEMU_SPRN_SRR0:
	case PPCEMU_SPRN_SRR1:
	case PPCEMU_SPRN_SPRG0:
	case PPCEMU_SPRN_SPRG1:
	case PPCEMU_SPRN_SPRG2:
	case PPCEMU_SPRN_SPRG3: {
		state->gpr[rS] = state->sprs[ppcemu_sprn_to_idx(sprn)];
		break;
	}
	case PPCEMU_SPRN_HID2_GEKKO: {
		if (!(state->caps & CAPS_HID2_GEKKO)) {
			puts("Attempted to read HID2-Gekko on unsupported CPU");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rS] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	case PPCEMU_SPRN_HID4: {
		if (!(state->caps & CAPS_HID4)) {
			puts("Attempted to read HID4 on unsupported CPU");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rS] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	default:
		assert(!"Unknown MSR");
	}
}
