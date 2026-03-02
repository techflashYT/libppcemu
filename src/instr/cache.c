/*
 * libppcemu - PowerPC Instruction handling - Syncronization/Cache
 *
 * Copyright (C) 2026 Techflash
 */

#include "../state.h"


static void do_isync(struct _ppcemu_state *state, u32 inst) {
	NO_RC();
	/* TODO: actually syncronize something */
}

static void do_sync(struct _ppcemu_state *state, u32 inst) {
	NO_RC();
	/* TODO: actually syncronize something */
}

static void do_dcbf(struct _ppcemu_state *state, uint rA, uint rB) {
	/* TODO: actually store+invalidate the D-Cache block @ rA + (i32)rB */
}

static void do_dcbst(struct _ppcemu_state *state, uint rA, uint rB) {
	/* TODO: actually store the D-Cache block @ rA + (i32)rB */
}

static void do_dcbi(struct _ppcemu_state *state, uint rA, uint rB) {
	/* TODO: actually invalidate the D-Cache block @ rA + (i32)rB */
}

static void do_icbi(struct _ppcemu_state *state, uint rA, uint rB) {
	/* TODO: actually invalidate the I-Cache block @ rA + (i32)rB */
}
