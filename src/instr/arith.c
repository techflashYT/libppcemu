/*
 * libppcemu - PowerPC Instruction handling - Arithmetic
 *
 * Copyright (C) 2026 Techflash
 */

#include "../cr.h"
#include "../state.h"
#include "ppcemu/spr.h"

static inline void set_xer_ca(struct _ppcemu_state *state, bool ca) {
	u32 xer = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)];
	xer = (xer & ~PPCEMU_XER_CA) | (ca ? PPCEMU_XER_CA : 0);
	state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] = xer;
}

static inline void set_xer_ov(struct _ppcemu_state *state, uint ov) {
	u32 xer = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)];
	xer &= ~PPCEMU_XER_OV;
	if (ov)
		xer |= PPCEMU_XER_OV | PPCEMU_XER_SO;
	state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] = xer;
}

static inline u32 ppcemu_abs_s32_bits(u32 val) {
	return val & 0x80000000u ? (~val + 1) : val;
}

void generic_addi(struct _ppcemu_state *state, uint rD, uint rA, i32 simm) {
	if (rA == 0)
		state->gpr[rD] = simm;
	else
		state->gpr[rD] = simm + state->gpr[rA];
}

void do_add(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	state->gpr[rD] = state->gpr[rA] + state->gpr[rB];

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_addc(struct _ppcemu_state *state, uint rD, uint rA, u16 rB, uint OE, uint Rc) {
	u64 result = (u64)state->gpr[rA] + (u64)state->gpr[rB];
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_adde(struct _ppcemu_state *state, uint rD, uint rA, u16 rB, uint OE, uint Rc) {
	u32 ca = !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_CA);
	u64 result = (u64)state->gpr[rA] + (u64)state->gpr[rB] + ca;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_addze(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc) {
	u32 ca = !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_CA);
	u64 result = (u64)state->gpr[rA] + ca;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_addme(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc) {
	u32 ca = !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_CA);
	u64 result = (u64)state->gpr[rA] + ca + 0xffffffff;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_addic(struct _ppcemu_state *state, uint rD, uint rA, u16 simm, uint Rc) {
	u64 result = (u64)state->gpr[rA] + (u64)(u32)(i32)(i16)simm;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_subf(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	state->gpr[rD] = (~state->gpr[rA]) + state->gpr[rB] + 1;

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_subfic(struct _ppcemu_state *state, uint rD, uint rA, u16 simm) {
	u64 result = (u64)(~state->gpr[rA]) + (u64)(u32)(i32)(i16)simm + 1;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);
}

void do_subfc(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	u64 result = (u64)(~state->gpr[rA]) + (u64)state->gpr[rB] + 1;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_subfe(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	u32 ca = !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_CA);
	u64 result = (u64)(~state->gpr[rA]) + (u64)state->gpr[rB] + ca;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_subfze(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc) {
	u32 ca = !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_CA);
	u64 result = (u64)(~state->gpr[rA]) + ca;
	state->gpr[rD] = (u32)result;
	set_xer_ca(state, result >> 32);

	/* TODO: update XER if OE */
	(void)OE;

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_divw(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	u32 quotient, dividend, divisor;
	uint overflow, negative;
	dividend = state->gpr[rA];
	divisor = state->gpr[rB];
	overflow = divisor == 0 || (dividend == 0x80000000u && divisor == 0xffffffffu);

	if (!overflow) {
		negative = (dividend ^ divisor) & 0x80000000u;
		quotient = ppcemu_abs_s32_bits(dividend) / ppcemu_abs_s32_bits(divisor);

		state->gpr[rD] = negative ? (~quotient + 1) : quotient;
	}

	if (OE)
		set_xer_ov(state, overflow);

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_divwu(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	uint overflow = state->gpr[rB] == 0;

	if (!overflow)
		state->gpr[rD] = state->gpr[rA] / state->gpr[rB];

	if (OE)
		set_xer_ov(state, overflow);

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_mulhw(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint Rc) {
	i64 res = (i64)(i32)state->gpr[rA] * (i64)(i32)state->gpr[rB];

	state->gpr[rD] = (u32)(res >> 32);

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_mulhwu(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint Rc) {
	u64 res = (u64)state->gpr[rA] * (u64)state->gpr[rB];

	state->gpr[rD] = (u32)(res >> 32);

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_mullw(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc) {
	u32 xer;
	u64 res = (u64)state->gpr[rA] * (u64)state->gpr[rB];

	state->gpr[rD] = (u32)(res & 0xffffffff);

	if (OE) {
		xer = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)];
		if (res >> 32)
			xer |= PPCEMU_XER_OV;
		state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] = xer;

		/* TODO: SO bit */
	}

	if (Rc)
		update_cr0(state, state->gpr[rD]);
}

void do_mulli(struct _ppcemu_state *state, uint rD, uint rA, u16 simm) {
	u64 res = (i64)(i32)state->gpr[rA] * (i16)simm;

	state->gpr[rD] = (u32)(res & 0xffffffff);
}
