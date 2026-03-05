/*
 * libppcemu - PowerPC Instruction handling - Bitwise operations
 *
 * Copyright (C) 2026 Techflash
 */

#include "../cr.h"
#include "../state.h"

static u32 rotl32(u32 x, uint rot) {
	return (x << rot) | (x >> (32 - rot));
}

static void xori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] ^ uimm;
}

#define do_xori(s, rS, rA, uimm) xori_common(s, rS, rA, uimm)
#define do_xoris(s, rS, rA, uimm) xori_common(s, rS, rA, (uimm << 16))

static void do_xor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] ^ state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void ori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] | uimm;
}

#define do_ori(s, rS, rA, uimm) ori_common(s, rS, rA, uimm)
#define do_oris(s, rS, rA, uimm) ori_common(s, rS, rA, (uimm << 16))

static void do_or(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc)  {
	state->gpr[rA] = state->gpr[rS] | state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void andi_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] & uimm;

	update_cr0(state, state->gpr[rA]); /* andi(s) always updates CR0 */
}

#define do_andi(s, rS, rA, uimm) andi_common(s, rS, rA, uimm)
#define do_andis(s, rS, rA, uimm) andi_common(s, rS, rA, (uimm << 16))

static void do_and(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc)  {
	state->gpr[rA] = state->gpr[rS] & state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void do_rlwinm(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint MB, uint ME, uint Rc) {
	u32 r, m;
	uint b;

	r = rotl32(state->gpr[rS], SH);
	m = 0;
	/* horrid mask generation */
	if (MB <= ME) {
		for (b = MB; b <= ME; b++)
			m |= 1u << (31 - b);
	} else { /* wraparound */
		for (b = 0; b <= ME; b++)
			m |= 1u << (31 - b);

		for (b = MB; b <= 31; b++)
			m |= 1u << (31 - b);
	}

	state->gpr[rA] = r & m;
	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void do_nand(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = ~(state->gpr[rS] & state->gpr[rB]);

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void do_neg(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc) {
	state->gpr[rA] = ~state->gpr[rD];

	/* TODO: update XER if OE */

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void do_nor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = ~(state->gpr[rS] | state->gpr[rB]);

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

static void do_cntlzw(struct _ppcemu_state *state, uint rS, uint rA, uint Rc) {
	u32 n = 0;

	while (n <= 31) {
		if (n & (1 << n))
			break;

		n++;
	}

	state->gpr[rA] = n;

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}
