/*
 * libppcemu - PowerPC Instruction handling - SPR handling
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

/* TODO: check MSR[PR] on SPR R/W */
static void do_mtspr(struct _ppcemu_state *state, uint rS, uint sprn) {
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
	case PPCEMU_SPRN_SPRG3:
	case PPCEMU_SPRN_HID0: {
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
	case PPCEMU_SPRN_IBAT0U:
	case PPCEMU_SPRN_IBAT0L:
	case PPCEMU_SPRN_IBAT1U:
	case PPCEMU_SPRN_IBAT1L:
	case PPCEMU_SPRN_IBAT2U:
	case PPCEMU_SPRN_IBAT2L:
	case PPCEMU_SPRN_IBAT3U:
	case PPCEMU_SPRN_IBAT3L:
	case PPCEMU_SPRN_DBAT0U:
	case PPCEMU_SPRN_DBAT0L:
	case PPCEMU_SPRN_DBAT1U:
	case PPCEMU_SPRN_DBAT1L:
	case PPCEMU_SPRN_DBAT2U:
	case PPCEMU_SPRN_DBAT2L:
	case PPCEMU_SPRN_DBAT3U:
	case PPCEMU_SPRN_DBAT3L: {
		state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];
		break;
	}
	case PPCEMU_SPRN_IBAT4U:
	case PPCEMU_SPRN_IBAT4L:
	case PPCEMU_SPRN_IBAT5U:
	case PPCEMU_SPRN_IBAT5L:
	case PPCEMU_SPRN_IBAT6U:
	case PPCEMU_SPRN_IBAT6L:
	case PPCEMU_SPRN_IBAT7U:
	case PPCEMU_SPRN_IBAT7L:
	case PPCEMU_SPRN_DBAT4U:
	case PPCEMU_SPRN_DBAT4L:
	case PPCEMU_SPRN_DBAT5U:
	case PPCEMU_SPRN_DBAT5L:
	case PPCEMU_SPRN_DBAT6U:
	case PPCEMU_SPRN_DBAT6L:
	case PPCEMU_SPRN_DBAT7U:
	case PPCEMU_SPRN_DBAT7L: {
		if (!(state->caps & CAPS_UPPER_BATS)) {
			puts("Attempted to write to upper BATs on an unsupported CPU");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	default: {
		printf("Unknown SPR write %d\r\n", sprn);
		exception_fire(state, EXCEPTION_PROGRAM);
		break;
	}
	}
}

static void do_mfspr(struct _ppcemu_state *state, uint rS, uint sprn) {
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
	case PPCEMU_SPRN_SPRG3:
	case PPCEMU_SPRN_HID0: {
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
	case PPCEMU_SPRN_IBAT0U:
	case PPCEMU_SPRN_IBAT0L:
	case PPCEMU_SPRN_IBAT1U:
	case PPCEMU_SPRN_IBAT1L:
	case PPCEMU_SPRN_IBAT2U:
	case PPCEMU_SPRN_IBAT2L:
	case PPCEMU_SPRN_IBAT3U:
	case PPCEMU_SPRN_IBAT3L:
	case PPCEMU_SPRN_DBAT0U:
	case PPCEMU_SPRN_DBAT0L:
	case PPCEMU_SPRN_DBAT1U:
	case PPCEMU_SPRN_DBAT1L:
	case PPCEMU_SPRN_DBAT2U:
	case PPCEMU_SPRN_DBAT2L:
	case PPCEMU_SPRN_DBAT3U:
	case PPCEMU_SPRN_DBAT3L: {
		state->gpr[rS] = state->sprs[ppcemu_sprn_to_idx(sprn)];
		break;
	}
	case PPCEMU_SPRN_IBAT4U:
	case PPCEMU_SPRN_IBAT4L:
	case PPCEMU_SPRN_IBAT5U:
	case PPCEMU_SPRN_IBAT5L:
	case PPCEMU_SPRN_IBAT6U:
	case PPCEMU_SPRN_IBAT6L:
	case PPCEMU_SPRN_IBAT7U:
	case PPCEMU_SPRN_IBAT7L:
	case PPCEMU_SPRN_DBAT4U:
	case PPCEMU_SPRN_DBAT4L:
	case PPCEMU_SPRN_DBAT5U:
	case PPCEMU_SPRN_DBAT5L:
	case PPCEMU_SPRN_DBAT6U:
	case PPCEMU_SPRN_DBAT6L:
	case PPCEMU_SPRN_DBAT7U:
	case PPCEMU_SPRN_DBAT7L: {
		if (!(state->caps & CAPS_UPPER_BATS)) {
			puts("Attempted to read the upper BATs on an unsupported CPU");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rS] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	default: {
		printf("Unknown SPR read %d\r\n", sprn);
		exception_fire(state, EXCEPTION_PROGRAM);
		break;
	}
	}
}
