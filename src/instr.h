/*
 * libppcemu - Internal Headers - Instructions
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_INSTR_H
#define _LIBPPCEMU_INTERNAL_INSTR_H

#include "state.h"
#include "types.h"

/* arithmetic */
extern void generic_addi(struct _ppcemu_state *state, uint rD, uint rA, i32 simm);
#define do_addi(s, rD, rA, simm) generic_addi(s, rD, rA, (i32)(i16)simm)
#define do_addis(s, rD, rA, simm) generic_addi(s, rD, rA, (i32)((i16)simm << 16))
extern void do_add(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_addc(struct _ppcemu_state *state, uint rD, uint rA, u16 rB, uint OE, uint Rc);
extern void do_adde(struct _ppcemu_state *state, uint rD, uint rA, u16 rB, uint OE, uint Rc);
extern void do_addze(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc);
extern void do_addme(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc);
extern void do_addic(struct _ppcemu_state *state, uint rD, uint rA, u16 simm, uint Rc);
extern void do_subf(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_subfic(struct _ppcemu_state *state, uint rD, uint rA, u16 simm);
extern void do_subfc(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_subfe(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_divw(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_divwu(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_mulhw(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint Rc);
extern void do_mulhwu(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint Rc);
extern void do_mullw(struct _ppcemu_state *state, uint rD, uint rA, uint rB, uint OE, uint Rc);
extern void do_mulli(struct _ppcemu_state *state, uint rD, uint rA, u16 simm);

/* bitwise operations */
extern void xori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm);
#define do_xori(s, rS, rA, uimm) xori_common(s, rS, rA, uimm)
#define do_xoris(s, rS, rA, uimm) xori_common(s, rS, rA, (uimm << 16))
extern void do_xor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void ori_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm);
#define do_ori(s, rS, rA, uimm) ori_common(s, rS, rA, uimm)
#define do_oris(s, rS, rA, uimm) ori_common(s, rS, rA, (uimm << 16))
extern void do_or(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void andi_common(struct _ppcemu_state *state, uint rS, uint rA, u32 uimm);
#define do_andi(s, rS, rA, uimm) andi_common(s, rS, rA, uimm)
#define do_andis(s, rS, rA, uimm) andi_common(s, rS, rA, (uimm << 16))
extern void do_and(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_andc(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_rlwimi(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint MB, uint ME, uint Rc);
extern void do_rlwinm(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint MB, uint ME, uint Rc);
extern void do_rlwnm(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint MB, uint ME, uint Rc);
extern void do_nand(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_neg(struct _ppcemu_state *state, uint rD, uint rA, uint OE, uint Rc);
extern void do_nor(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_cntlzw(struct _ppcemu_state *state, uint rS, uint rA, uint Rc);
extern void do_slw(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_sraw(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_srawi(struct _ppcemu_state *state, uint rS, uint rA, uint SH, uint Rc);
extern void do_srw(struct _ppcemu_state *state, uint rS, uint rA, uint rB, uint Rc);
extern void do_extsh(struct _ppcemu_state *state, uint rS, uint rA, uint Rc);
extern void do_extsb(struct _ppcemu_state *state, uint rS, uint rA, uint Rc);

/* branches */
extern void do_rfi(struct _ppcemu_state *state, u32 inst);
extern void do_branch(struct _ppcemu_state *state, u32 li, uint aa, uint lk);
extern void _do_cond_branch(struct _ppcemu_state *state, uint bo, uint bi, uint lk, u32 target_addr);
#define do_bclr(s, bo, bi, lk) _do_cond_branch(s, bo, bi, lk, s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_LR)])
#define do_bcctr(s, bo, bi, lk) _do_cond_branch(s, bo, bi, lk, s->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_CTR)])
extern void do_bc(struct _ppcemu_state *state, uint bo, uint bi, uint bd, uint aa, uint lk);

/* cache / syncronization */
extern void do_isync(struct _ppcemu_state *state, u32 inst);
extern void do_sync(struct _ppcemu_state *state, u32 inst);
extern void do_sync(struct _ppcemu_state *state, u32 inst);
extern void do_dcbf(struct _ppcemu_state *state, uint rA, uint rB);
extern void do_dcbst(struct _ppcemu_state *state, uint rA, uint rB);
extern void do_dcbi(struct _ppcemu_state *state, uint rA, uint rB);
extern void do_icbi(struct _ppcemu_state *state, uint rA, uint rB);

/* conditionals */
extern void do_cmpi(struct _ppcemu_state *state, uint crfD, uint rA, u16 simm);
extern void do_cmpli(struct _ppcemu_state *state, uint crfD, uint rA, u16 uimm);
extern void do_cmp(struct _ppcemu_state *state, uint crfD, uint rA, uint rB);
extern void do_cmpl(struct _ppcemu_state *state, uint crfD, uint rA, uint rB);
extern void do_mfcr(struct _ppcemu_state *state, uint rD);
extern void do_mtcrf(struct _ppcemu_state *state, uint rS, uint crm);
extern void do_crxor(struct _ppcemu_state *state, uint crbD, uint crbA, uint crbB);

/* Floating Point */
extern void do_lfd(struct _ppcemu_state *state, uint frD, uint rA, u16 d);
extern void do_fmr(struct _ppcemu_state *state, uint frD, uint frB, uint Rc);
extern void do_mtfsf(struct _ppcemu_state *state, uint FM, uint frB, uint Rc);
extern void do_mtfsbN(struct _ppcemu_state *state, uint crbD, uint set, uint Rc);
#define do_mtfsb0(state, crbD, Rc) do_mtfsbN(state, crbD, 0, Rc)
#define do_mtfsb1(state, crbD, Rc) do_mtfsbN(state, crbD, 1, Rc)

/* memory */
extern u32 do_basic_store(struct _ppcemu_state *state, uint len, uint rS, uint rA, u16 d);
extern u32 do_indexed_store(struct _ppcemu_state *state, uint len, uint rS, uint rA, u16 rB);
#define do_basic_store_update(s, len, rS, rA, d) s->gpr[rA] = do_basic_store(s, len, rS, rA, d);
#define do_indexed_store_update(s, len, rS, rA, d) s->gpr[rA] = do_indexed_store(s, len, rS, rA, d);
extern void do_stmw(struct _ppcemu_state *state, uint rS, uint rA, u16 d);
extern u32 do_basic_load(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint d);
extern u32 do_indexed_load(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint rB);
#define do_basic_load_update(s, len, rD, rA, d) s->gpr[rA] = do_basic_load(s, len, rD, rA, d);
static inline u32 do_basic_load_signext(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint d) {
	u32 ret = do_basic_load(state, len, rD, rA, d);
	if (len == 2)
		state->gpr[rD] = (i32)(i16)state->gpr[rD];
	else if (len == 1)
		state->gpr[rD] = (i32)(i16)(i8)state->gpr[rD];

	return ret;
}
#define do_basic_load_signext_update(s, len, rD, rA, d) s->gpr[rA] = do_basic_load_signext(s, len, rD, rA, d)
#define do_indexed_load_update(s, len, rD, rA, d) s->gpr[rA] = do_indexed_load(s, len, rD, rA, d);
static inline u32 do_indexed_load_signext(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint d) {
	u32 ret = do_indexed_load(state, len, rD, rA, d);
	if (len == 2)
		state->gpr[rD] = (i32)(i16)state->gpr[rD];
	else if (len == 1)
		state->gpr[rD] = (i32)(i16)(i8)state->gpr[rD];

	return ret;
}
#define do_indexed_load_signext_update(s, len, rD, rA, d) s->gpr[rA] = do_indexed_load_signext(s, len, rD, rA, d)
extern void do_lmw(struct _ppcemu_state *state, uint rD, uint rA, u16 d);

/* MSR */
extern void do_mtmsr(struct _ppcemu_state *state, uint rS);
extern void do_mfmsr(struct _ppcemu_state *state, uint rD);

/* Paired Singles */
extern void do_psq_l(struct _ppcemu_state *state, uint frD, uint rA, uint W, uint PSQ, u16 d);
extern void do_ps_mr(struct _ppcemu_state *state, uint frD, uint frB, uint Rc);

/* Segment Registers */
extern void do_mtsr(struct _ppcemu_state *state, uint sreg, uint rS);
extern void do_mfsr(struct _ppcemu_state *state, uint sreg, uint rD);

/* SPRs */
extern void do_mtspr(struct _ppcemu_state *state, uint rS, uint sprn);
extern void do_mfspr(struct _ppcemu_state *state, uint rD, uint sprn);

#endif /* _LIBPPCEMU_INTERNAL_INSTR_H */
