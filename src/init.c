/*
 * libppcemu - Initialization
 *
 * Copyright (C) 2026 Techflash
 */

#define LOG_LEVEL misc_loglevel
#include <stdlib.h>
#include <string.h>
#include <ppcemu/init.h>
#include <ppcemu/msr.h>
#include <ppcemu/types.h>
#include <ppcemu/state.h>
#include <ppcemu/spr.h>
#include "cache.h"
#include "caps.h"
#include "exception.h"
#include "log.h"
#include "state.h"
#include "spr.h"

struct ppcemu_state *ppcemu_init(enum ppcemu_cpu_model model, ppcemu_bus_hook bus_hook, uint bus_speed_khz, uint c2b_mult) {
	struct _ppcemu_state *state;

	if (model < 0 || model > PPCEMU_CPU_MODEL_ESPRESSO)
		return NULL;

	state = malloc(sizeof(struct _ppcemu_state));
	if (!state)
		return NULL;

	state->model = model;
	state->bus_hook = bus_hook;
	state->c2b_mult = c2b_mult;
	state->bus_speed_khz = bus_speed_khz;
	ppcemu_reset((struct ppcemu_state *)state);

	return (struct ppcemu_state *)state;
}

int ppcemu_reset(struct ppcemu_state *state) {
	struct _ppcemu_state *s;
	int ret;
	enum ppcemu_cpu_model model;
	ppcemu_bus_hook bus_hook;
	uint c2b_mult, bus_speed_khz;

	s = (struct _ppcemu_state *)state;

	/* save some state before the below memset */
	model = s->model;
	bus_hook = s->bus_hook;
	c2b_mult = s->c2b_mult;
	bus_speed_khz = s->bus_speed_khz;

	memset(s, 0, sizeof(struct _ppcemu_state));

	s->model = model;
	switch (model) {
	case PPCEMU_CPU_MODEL_750CXE: {
		s->caps = 0;
		s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_PVR)] = PPCEMU_PVR_750CXE_DEFAULT;
		break;
	}
	case PPCEMU_CPU_MODEL_GEKKO: {
		s->caps = CAPS_PS_IDX | CAPS_PS_LD_ST | CAPS_HID2_GEKKO | CAPS_L2CR | CAPS_PERF_MON | CAPS_WR_GATHER_PIPE;
		s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_PVR)] = PPCEMU_PVR_GEKKO_DEFAULT;
		break;
	}
	case PPCEMU_CPU_MODEL_750CL: {
		s->caps = CAPS_PS_IDX | CAPS_HID2_GEKKO | CAPS_HID4 | CAPS_UPPER_BATS | CAPS_L2CR | CAPS_PERF_MON | CAPS_WR_GATHER_PIPE;
		s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_PVR)] = PPCEMU_PVR_750CL_DEFAULT;
		break;
	}
	case PPCEMU_CPU_MODEL_BROADWAY: {
		s->caps = CAPS_PS_IDX | CAPS_PS_LD_ST | CAPS_HID2_GEKKO | CAPS_HID4 | CAPS_UPPER_BATS | CAPS_HID4_BDWAY_LIKE | CAPS_L2CR | CAPS_PERF_MON | CAPS_WR_GATHER_PIPE;
		s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_PVR)] = PPCEMU_PVR_BROADWAY_DEFAULT;
		break;
	}
	case PPCEMU_CPU_MODEL_ESPRESSO: {
		s->caps = CAPS_PS_IDX | CAPS_PS_LD_ST | CAPS_HID2_GEKKO | CAPS_HID4 | CAPS_UPPER_BATS | CAPS_HID4_BDWAY_LIKE | CAPS_L2CR | CAPS_PERF_MON | CAPS_WR_GATHER_PIPE;
		s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_PVR)] = PPCEMU_PVR_ESPRESSO_DEFAULT;
		break;
	}
	}

	s->cache_mode = PPCEMU_CACHE_MODE_STANDARD;
	/* TODO: don't hardcode */
	ret = ppcemu_cache_init(s, 16 * 1024, 16 * 1024);
	if (ret) {
		error("ppcemu_cache_init failed: %d\r\n", ret);
		return ret;
	}

	s->sync_rt = false;
	s->bus_hook = bus_hook;
	s->bus_speed_khz = bus_speed_khz;
	s->c2b_mult = c2b_mult;
	s->msr = PPCEMU_MSR_IP;
	s->ready = true;
	exception_fire(s, EXCEPTION_RESET);

	return 0;
}
