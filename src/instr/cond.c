/*
 * libppcemu - PowerPC Instruction handling - Conditionals
 *
 * Copyright (C) 2026 Techflash
 */

#define LOG_LEVEL cond_loglevel
#include <stdio.h>
#include <ppcemu/spr.h>
#include "../cr.h"
#include "../log.h"
#include "../state.h"

#ifdef DEBUG_COND
#define cond_debug debug
#else
static void cond_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif

void do_cmpi(struct _ppcemu_state *state, uint crfD, uint rA, u16 simm) {
	u32 c, a = state->gpr[rA], oldCR;

	if ((i32)a < (i32)(i16)simm)
		c = 0b1000;
	else if ((i32)a > (i32)(i16)simm)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);
	oldCR = state->cr;

	cr_set_field(state, crfD, c);
	cond_debug("cmpi: crfD=%u, rA=%u, rA(val)=0x%08x, simm=0x%04x, (i32)(i16)simm=0x%08x, c=0x%1x, oldCR=0x%08x, newCR=0x%08x\r\n", crfD, rA, a, simm, (i32)(i16)simm, c, oldCR, state->cr);
}

void do_cmpli(struct _ppcemu_state *state, uint crfD, uint rA, u16 uimm) {
	u32 c, a = state->gpr[rA], oldCR;

	if (a < (u32)uimm)
		c = 0b1000;
	else if (a > (u32)uimm)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);
	oldCR = state->cr;

	cr_set_field(state, crfD, c);
	cond_debug("cmpli: crfD=%u, rA=%u, rA(val)=0x%08x, uimm=0x%04x, (u32)uimm=0x%08x, c=0x%1x, oldCR=0x%08x, newCR=0x%08x\r\n", crfD, rA, a, uimm, (u32)uimm, c, oldCR, state->cr);
}

void do_cmp(struct _ppcemu_state *state, uint crfD, uint rA, uint rB) {
	u32 c, a = state->gpr[rA], b = state->gpr[rB], oldCR;

	if ((i32)a < (i32)b)
		c = 0b1000;
	else if ((i32)a > (i32)b)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);
	oldCR = state->cr;

	cr_set_field(state, crfD, c);
	cond_debug("cmp: crfD=%u, rA=%u, rA(val)=0x%08x, rB=%u, rB(val)=0x%08x, c=0x%1x, oldCR=0x%08x, newCR=0x%08x\r\n", crfD, rA, a, rB, b, c, oldCR, state->cr);
}

void do_cmpl(struct _ppcemu_state *state, uint crfD, uint rA, uint rB) {
	u32 c, a = state->gpr[rA], b = state->gpr[rB], oldCR;

	if (a < b)
		c = 0b1000;
	else if (a > b)
		c = 0b0100;
	else /* equal */
		c = 0b0010;

	c |= !!(state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_XER)] & PPCEMU_XER_SO);
	oldCR = state->cr;

	cr_set_field(state, crfD, c);
	cond_debug("cmpl: crfD=%u, rA=%u, rA(val)=0x%08x, rB=%u, rB(val)=0x%08x, c=0x%1x, oldCR=0x%08x, newCR=0x%08x\r\n", crfD, rA, a, rB, b, c, oldCR, state->cr);
}

void do_mfcr(struct _ppcemu_state *state, uint rD) {
	state->gpr[rD] = state->cr;
}


void do_mtcrf(struct _ppcemu_state *state, uint rS, uint crm) {
	u32 mask;
	uint i;

	if (crm == 0xff) /* fast path for "set all" */
		state->cr = state->gpr[rS];
	else {
		/* actually mask it */
		mask = 0;
		for (i = 0; i < 8; i++) {
			if (crm & (1 << i))
				mask |= (0xf << (i * 4));
		}

		state->cr &= ~mask;
		state->cr |= (state->gpr[rS] & mask);
	}
}
