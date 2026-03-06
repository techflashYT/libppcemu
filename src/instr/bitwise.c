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

static u32 mb_me_mask(uint MB, uint ME) {
	uint b;
	u32 m;

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

	return m;
}

void xori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] ^ uimm;
}

void do_xor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] ^ state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void ori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] | uimm;
}

void do_or(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] | state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void andi_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm) {
	state->gpr[rA] = state->gpr[rS] & uimm;

	update_cr0(state, state->gpr[rA]); /* andi(s) always updates CR0 */
}

void do_and(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] & state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_andc(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] & (~state->gpr[rB]);

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_rlwimi(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint MB, uint ME, uint Rc) {
	u32 r, m;

	r = rotl32(state->gpr[rS], SH);
	m = mb_me_mask(MB, ME);

	state->gpr[rA] = (r & m) | (state->gpr[rA] & ~m);
	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_rlwinm(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint MB, uint ME, uint Rc) {
	u32 r, m;

	r = rotl32(state->gpr[rS], SH);
	m = mb_me_mask(MB, ME);

	state->gpr[rA] = r & m;
	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_rlwnm(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint MB, uint ME, uint Rc) {
	u32 r, m;

	r = rotl32(state->gpr[rS], (state->gpr[rB] & 0x1f));
	m = mb_me_mask(MB, ME);

	state->gpr[rA] = r & m;
	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_nand(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = ~(state->gpr[rS] & state->gpr[rB]);

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_neg(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc) {
	state->gpr[rA] = ~state->gpr[rD];

	/* TODO: update XER if OE */

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_nor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = ~(state->gpr[rS] | state->gpr[rB]);

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_cntlzw(struct _ppcemu_state *state, uint rS, uint rA, uint Rc) {
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

void do_slw(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] << state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_sraw(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = (i32)state->gpr[rS] >> state->gpr[rB];

	/* TODO: some crap with XER.CA? */

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_srawi(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint Rc) {
	state->gpr[rA] = (i32)state->gpr[rS] >> SH;

	/* TODO: some crap with XER.CA? */

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}

void do_srw(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc) {
	state->gpr[rA] = state->gpr[rS] >> state->gpr[rB];

	if (Rc)
		update_cr0(state, state->gpr[rA]);
}
