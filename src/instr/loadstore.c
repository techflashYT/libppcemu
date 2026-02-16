/*
 * libppcemu - PowerPC Instruction handling - Load/Store
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"
#include "../mem.h"
#include "../exception.h"

static void do_basic_store(struct _ppcemu_state *state, uint len, uint rS, uint rA, uint d) {
	enum virt2phys_err err;
	u32 b, ea, phys;
	u8 v8;
	u16 v16;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	/* TODO: sign-extend */
	ea = b + d;

	err = ppcemu_virt2phys(state, ea, &phys, false, true);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		exception_fire(state, EXCEPTION_DSI);
		return;
	}

	switch (len) {
	case 1: {
		v8 = (u8)((state->gpr[rS] & 0xff000000) >> 24);
		state->bus_hook((struct ppcemu_state *)state, phys, 1, &v8, true);
		break;
	}
	case 2: {
		v16 = (u16)((state->gpr[rS] & 0xffff0000) >> 16);
		state->bus_hook((struct ppcemu_state *)state, phys, 2, &v16, true);
		break;
	}
	case 4: {
		state->bus_hook((struct ppcemu_state *)state, phys, 4, &state->gpr[rS], true);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}
}

static void do_basic_load(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint d) {
	enum virt2phys_err err;
	u32 b, ea, phys;
	u8 v8;
	u16 v16;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	/* TODO: sign-extend */
	ea = b + d;

	err = ppcemu_virt2phys(state, ea, &phys, false, false);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		exception_fire(state, EXCEPTION_DSI);
		return;
	}

	switch (len) {
	case 1: {
		state->bus_hook((struct ppcemu_state *)state, phys, 1, &v8, false);
		state->gpr[rD] &= 0x00ffffff;
		state->gpr[rD] |= (v8 << 24);
		break;
	}
	case 2: {
		state->bus_hook((struct ppcemu_state *)state, phys, 2, &v16, false);
		state->gpr[rD] &= 0x0000ffff;
		state->gpr[rD] |= (v16 << 16);
		break;
	}
	case 4: {
		state->bus_hook((struct ppcemu_state *)state, phys, 4, &state->gpr[rD], false);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}
}
