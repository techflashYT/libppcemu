/*
 * libppcemu - PowerPC Instruction handling - SPR handling
 *
 * Copyright (C) 2026 Techflash
 */

#include <assert.h>
#include <ppcemu/spr.h>
#include "../caps.h"
#include "../exception.h"
#include "../spr.h"
#include "../state.h"
#include "../../config.h"

/* TODO: check MSR[PR] on SPR R/W */
void do_mtspr(struct _ppcemu_state *state, uint rS, uint sprn) {
	switch (sprn) {
	case PPCEMU_SPRN_XER:
	case PPCEMU_SPRN_LR:
	case PPCEMU_SPRN_CTR:
	case PPCEMU_SPRN_DEC:
	case PPCEMU_SPRN_SRR0:
	case PPCEMU_SPRN_SRR1:
	case PPCEMU_SPRN_SPRG0:
	case PPCEMU_SPRN_SPRG1:
	case PPCEMU_SPRN_SPRG2:
	case PPCEMU_SPRN_SPRG3:
	case PPCEMU_SPRN_HID0:
	case PPCEMU_SPRN_TBL_OEA:
	case PPCEMU_SPRN_TBL_VEA:
	case PPCEMU_SPRN_TBU_OEA:
	case PPCEMU_SPRN_TBU_VEA: {
		state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];
		break;
	}
	case PPCEMU_SPRN_HID2_GEKKO: {
		if (!(state->caps & CAPS_HID2_GEKKO)) {
			warn("Attempted to write to HID2-Gekko on unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	case PPCEMU_SPRN_HID4: {
		if (!(state->caps & CAPS_HID4)) {
			warn("Attempted to write to HID4 on unsupported CPU\r\n");
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
			warn("Attempted to write to upper BATs on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	case PPCEMU_SPRN_GQR0:
	case PPCEMU_SPRN_GQR1:
	case PPCEMU_SPRN_GQR2:
	case PPCEMU_SPRN_GQR3:
	case PPCEMU_SPRN_GQR4:
	case PPCEMU_SPRN_GQR5:
	case PPCEMU_SPRN_GQR6:
	case PPCEMU_SPRN_GQR7: {
		if (!(state->caps & CAPS_PS_IDX)) {
			warn("Attempted to write to GQRs on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	case PPCEMU_SPRN_L2CR: {
		if (!(state->caps & CAPS_L2CR)) {
			warn("Attempted to write to L2CR on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	case PPCEMU_SPRN_MMCR0:
	case PPCEMU_SPRN_MMCR1:
	case PPCEMU_SPRN_PMC1:
	case PPCEMU_SPRN_PMC2:
	case PPCEMU_SPRN_PMC3:
	case PPCEMU_SPRN_PMC4: {
		if (!(state->caps & CAPS_PERF_MON)) {
			warn("Attempted to write PMCn/MMCRn on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->sprs[ppcemu_sprn_to_idx(sprn)] = state->gpr[rS];

		break;
	}
	default: {
		warn("Unknown SPR write %d\r\n", sprn);
		exception_fire(state, EXCEPTION_PROGRAM);
		break;
	}
	}
}

void do_mfspr(struct _ppcemu_state *state, uint rD, uint sprn) {
	switch (sprn) {
	case PPCEMU_SPRN_XER:
	case PPCEMU_SPRN_LR:
	case PPCEMU_SPRN_CTR:
	case PPCEMU_SPRN_DEC:
	case PPCEMU_SPRN_SRR0:
	case PPCEMU_SPRN_SRR1:
	case PPCEMU_SPRN_SPRG0:
	case PPCEMU_SPRN_SPRG1:
	case PPCEMU_SPRN_SPRG2:
	case PPCEMU_SPRN_SPRG3:
	case PPCEMU_SPRN_HID0:
	case PPCEMU_SPRN_TBL_OEA:
	case PPCEMU_SPRN_TBL_VEA:
	case PPCEMU_SPRN_TBU_OEA:
	case PPCEMU_SPRN_TBU_VEA: {
		state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];
		break;
	}
	case PPCEMU_SPRN_HID2_GEKKO: {
		if (!(state->caps & CAPS_HID2_GEKKO)) {
			warn("Attempted to read HID2-Gekko on unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	case PPCEMU_SPRN_HID4: {
		if (!(state->caps & CAPS_HID4)) {
			warn("Attempted to read HID4 on unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];

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
		state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];
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
			warn("Attempted to read the upper BATs on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	case PPCEMU_SPRN_GQR0:
	case PPCEMU_SPRN_GQR1:
	case PPCEMU_SPRN_GQR2:
	case PPCEMU_SPRN_GQR3:
	case PPCEMU_SPRN_GQR4:
	case PPCEMU_SPRN_GQR5:
	case PPCEMU_SPRN_GQR6:
	case PPCEMU_SPRN_GQR7: {
		if (!(state->caps & CAPS_PS_IDX)) {
			warn("Attempted to read the GQRs on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	case PPCEMU_SPRN_L2CR: {
		if (!(state->caps & CAPS_L2CR)) {
			warn("Attempted to read L2CR on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	case PPCEMU_SPRN_MMCR0:
	case PPCEMU_SPRN_MMCR1:
	case PPCEMU_SPRN_PMC1:
	case PPCEMU_SPRN_PMC2:
	case PPCEMU_SPRN_PMC3:
	case PPCEMU_SPRN_PMC4: {
		if (!(state->caps & CAPS_PERF_MON)) {
			warn("Attempted to read PMCn/MMCRn on an unsupported CPU\r\n");
			exception_fire(state, EXCEPTION_PROGRAM);
		}
		else
			state->gpr[rD] = state->sprs[ppcemu_sprn_to_idx(sprn)];

		break;
	}
	default: {
		warn("Unknown SPR read %d\r\n", sprn);
		exception_fire(state, EXCEPTION_PROGRAM);
		break;
	}
	}
}
