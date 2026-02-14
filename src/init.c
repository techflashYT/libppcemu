/*
 * libppcemu - Initialization
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdlib.h>
#include <string.h>
#include <ppcemu/init.h>
#include <ppcemu/msr.h>
#include <ppcemu/types.h>
#include <ppcemu/state.h>
#include "caps.h"
#include "exception.h"
#include "state.h"

struct ppcemu_state *ppcemu_init(enum ppcemu_cpu_model model, ppcemu_bus_hook bus_hook, uint bus_speed_khz, uint c2b_mult) {
	struct _ppcemu_state *state;

	if (model < 0 || model > PPCEMU_CPU_MODEL_ESPRESSO)
		return NULL;

	state = malloc(sizeof(struct _ppcemu_state));
	if (!state)
		return NULL;

	memset(state, 0, sizeof(struct _ppcemu_state));

	state->model = model;
	switch (model) {
	case PPCEMU_CPU_MODEL_750CXE: {
		state->caps = 0;
		break;
	}
	case PPCEMU_CPU_MODEL_GEKKO: {
		state->caps = CAPS_PS_IDX | CAPS_PS_LD_ST | CAPS_HID2_GEKKO;
		break;
	}
	case PPCEMU_CPU_MODEL_750CL: {
		state->caps = CAPS_PS_IDX | CAPS_HID2_GEKKO | CAPS_HID4 | CAPS_UPPER_BATS;
		break;
	}
	case PPCEMU_CPU_MODEL_BROADWAY:
		/* fallthrough */
	case PPCEMU_CPU_MODEL_ESPRESSO: {
		state->caps = CAPS_PS_IDX | CAPS_PS_LD_ST | CAPS_HID2_GEKKO | CAPS_HID4 | CAPS_UPPER_BATS | CAPS_HID4_BDWAY_LIKE;
		break;
	}
	}

	state->bus_hook = bus_hook;
	state->bus_speed_khz = bus_speed_khz;
	state->c2b_mult = c2b_mult;
	state->msr = PPCEMU_MSR_IP;
	state->ready = true;
	exception_fire(state, EXCEPTION_RESET);

	return (struct ppcemu_state *)state;
}
