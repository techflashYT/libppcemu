/*
 * libppcemu - PowerPC Instruction handling - Load/Store
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include "../state.h"
#include "../mem.h"
#include "../exception.h"

static void _do_basic_store(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;

	err = ppcemu_virt2phys(state, ea, &phys, false, true);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		printf("_do_basic_store: virt2phys error: %d\r\n", err);
		exception_fire(state, EXCEPTION_DSI);
		return;
	}

	state->bus_hook((struct ppcemu_state *)state, phys, len, val, true);
}

static u32 do_basic_store(struct _ppcemu_state *state, uint len, uint rS, uint rA, u16 d) {
	u32 b, ea;
	u16 v16;
	u8 v8;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)(i16)d;

	switch (len) {
	case 1: {
		v8 = (u8)((state->gpr[rS] & 0xff000000) >> 24);
		_do_basic_store(state, len, ea, &v8);
		break;
	}
	case 2: {
		v16 = (u16)((state->gpr[rS] & 0xffff0000) >> 16);
		_do_basic_store(state, len, ea, &v16);
		break;
	}
	case 4: {
		_do_basic_store(state, len, ea, &state->gpr[rS]);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}

	return ea;
}

static void do_store_update(struct _ppcemu_state *state, uint len, uint rS, uint rA, uint d) {
	state->gpr[rA] = do_basic_store(state, len, rS, rA, d);
}

static void do_stmw(struct _ppcemu_state *state, uint rS, uint rA, u16 d) {
	u32 ea, b;
	int r;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)(i16)d;

	for (r = rS; r <= 31; r++) {
		_do_basic_store(state, 4, ea, &state->gpr[r]);
		ea += 4;
	}
}

static void do_basic_load(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint d) {
	enum virt2phys_err err;
	u32 b, ea, phys, v32;
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
		state->gpr[rD] |= (ntohl(v16) << 16);
		break;
	}
	case 4: {
		state->bus_hook((struct ppcemu_state *)state, phys, 4, &v32, false);
		state->gpr[rD] = ntohl(v32);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}
}
