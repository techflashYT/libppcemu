/*
 * libppcemu - PowerPC Instruction decoding
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include "state.h"
#include "types.h"
#include "exception.h"

/* mix of https://www.ibm.com/docs/en/aix/7.1.0?topic=overview-instruction-forms and the 750CL user manual */
#define INST_OPCD(inst)      (((inst) & 0xfc000000) >> 26)

/* 0..5 = Opcode; 6..29 = LI; 30 = AA; 31 = LK */
#define INST_I_LI(inst)      (((inst) & 0x03fffffc) >> 2)
#define INST_I_AA(inst)      (((inst) & 0x00000002) >> 1)
#define INST_I_LK(inst)      (((inst) & 0x00000001) >> 0)

/* 0..5 = Opcode; 6..10 = rD/rS/SR; 11..15 = rA; 16..20 = rB; 21 = OE; 22..30 = XO; 31 = Rc */
#define INST_XO_rD(inst)     (((inst) & 0x03e00000) >> 21)
#define INST_XO_rS(inst)     INST_XO_rD(inst)
#define INST_XO_SR(inst)     INST_XO_rD(inst)
#define INST_XO_rA(inst)     (((inst) & 0x001f0000) >> 16)
#define INST_XO_rB(inst)     (((inst) & 0x0000f800) >> 11)
#define INST_XO_OE(inst)     (((inst) & 0x00000400) >> 10)
#define INST_XO_XO(inst)     (((inst) & 0x000003fe) >> 1)
#define INST_XO_Rc(inst)     (((inst) & 0x00000001) >> 0)

/* 0..5 = Opcode; 6..10 = rD/rS; 11..20 = spr/I/FXM+I; 21..30 = XO; 31 = Rc */
#define INST_XFX_rD(inst)    (((inst) & 0x03e00000) >> 21)
#define INST_XFX_rS(inst)    INST_XFX_rD(inst)
#define INST_XFX_I(inst)     (((inst) & 0x001ff800) >> 11)
#define INST_XFX_XO(inst)    (((inst) & 0x000007fe) >> 1)
#define INST_XFX_Rc(inst)    (((inst) & 0x00000001) >> 0)

/* same as XFX */
#define INST_XL_rD(inst)     INST_XFX_rD(inst)
#define INST_XL_rS(inst)     INST_XFX_rS(inst)
#define INST_XL_I(inst)      INST_XFX_I(inst)
#define INST_XL_XO(inst)     INST_XFX_XO(inst)
#define INST_XL_Rc(inst)     INST_XFX_Rc(inst)

/* 0..5 = Opcode; 6..10 = rD/rS/frD/frS/TO/BF/I/L; 16..31 = D/SIMM/UIMM */
#define INST_D_rD(inst)      (((inst) & 0x03e00000) >> 21)
#define INST_D_rS(inst)      INST_D_rD(inst)
#define INST_D_frD(inst)     INST_D_rD(inst)
#define INST_D_frS(inst)     INST_D_rD(inst)
#define INST_D_TO(inst)      INST_D_rD(inst)
#define INST_D_BF(inst)      INST_D_rD(inst)
#define INST_D_I(inst)       INST_D_rD(inst)
#define INST_D_L(inst)       INST_D_rD(inst)
#define INST_D_rA(inst)      (((inst) & 0x001f0000) >> 16)
#define INST_D_D(inst)       (((inst) & 0x0000ffff) >> 0)
#define INST_D_SIMM(inst)    INST_D_D(inst)
#define INST_D_UIMM(inst)    INST_D_D(inst)

#define NO_RC() if (INST_XFX_Rc(inst)) { exception_fire(state, EXCEPTION_PROGRAM); return; }

#include "instr/add.c"
#include "instr/bitwise.c"
#include "instr/spr.c"
#include "instr/sync.c"
#include "instr/branch.c"
#include "instr/msr.c"
#include "instr/segmentreg.c"
#include "instr/loadstore.c"

static void do_illegal(struct _ppcemu_state *state, u32 inst) {
	(void)inst;
	exception_fire(state, EXCEPTION_PROGRAM);
}
static void _do_xor(struct _ppcemu_state *state, u32 inst) { do_xor(state, INST_XO_rS(inst), INST_XO_rA(inst), INST_XO_rB(inst), INST_XO_Rc(inst)); }
static void _do_mtspr(struct _ppcemu_state *state, u32 inst) { /* fucking IBM.... */
	u32 sprn = ((INST_XFX_I(inst) & 0b0000011111) << 5) | ((INST_XFX_I(inst) & 0b1111100000) >> 5);
	NO_RC();
	do_mtspr(state, INST_XFX_rS(inst), sprn);
}
static void _do_mtmsr(struct _ppcemu_state *state, u32 inst) { NO_RC(); do_mtmsr(state, INST_XO_rS(inst)); }
static void _do_mfmsr(struct _ppcemu_state *state, u32 inst) { NO_RC(); do_mfmsr(state, INST_XO_rD(inst)); }
static void _do_mtsr(struct _ppcemu_state *state, u32 inst) { NO_RC(); do_mtsr(state, INST_XO_SR(inst), INST_XO_rS(inst)); }
static void _do_mfsr(struct _ppcemu_state *state, u32 inst) { NO_RC(); do_mfsr(state, INST_XO_SR(inst), INST_XO_rD(inst)); }

static void (*opc31_handlers[1024])(struct _ppcemu_state *state, u32 inst) = {
	/* 0  */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 16 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 32 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 48 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 64 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 80 */   do_illegal, do_illegal, do_illegal, _do_mfmsr,  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 96 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 112 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 128 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 144 */  do_illegal, do_illegal, _do_mtmsr,  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 160 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 176 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 192 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 208 */  do_illegal, do_illegal, _do_mtsr,   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 224 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 240 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 256 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 272 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 288 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 304 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, _do_xor,    do_illegal, do_illegal, do_illegal,
	/* 320 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 336 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 352 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 368 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 384 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 400 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 416 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 432 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 448 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 464 */  do_illegal, do_illegal, do_illegal, _do_mtspr,  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 480 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 496 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 512 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 528 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 544 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 560 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 576 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 592 */  do_illegal, do_illegal, do_illegal, _do_mfsr,   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 608 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 624 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 640 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 656 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 672 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 688 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 704 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 720 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 736 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 752 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 768 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 784 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 800 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 816 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 832 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 848 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 864 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 880 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 896 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 912 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 928 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 944 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 960 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 976 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 992 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 1008 */ do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
};

static void (*opc19_handlers[1024])(struct _ppcemu_state *state, u32 inst) = {
	/* 0  */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 16 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 32 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 48 */   do_illegal, do_illegal, do_rfi,     do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 64 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 80 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 96 */   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 112 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 128 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 144 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_isync,   do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 160 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 176 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 192 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 208 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 224 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 240 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 256 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 272 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 288 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 304 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 320 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 336 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 352 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 368 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 384 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 400 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 416 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 432 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 448 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 464 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 480 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 496 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 512 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 528 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 544 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 560 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 576 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 592 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_sync,    do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 608 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 624 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 640 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 656 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 672 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 688 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 704 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 720 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 736 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 752 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 768 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 784 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 800 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 816 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 832 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 848 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 864 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 880 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 896 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 912 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 928 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 944 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 960 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 976 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 992 */  do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
	/* 1008 */ do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal, do_illegal,
};

static void _ppcemu_decode_exec(struct _ppcemu_state *state, u32 inst) {
	state->branched = false;

	printf("top-level opcode: %d\r\n", INST_OPCD(inst));
	switch (INST_OPCD(inst)) {
	#if 0
	case 12: { /* addic */
		do_addic(state, INST_D_rD(inst), INST_D_rA(inst), INST_D_SIMM(inst));
		break;
	}
	case 13: { /* addic. */
		do_addic_rec(state, INST_D_rD(inst), INST_D_rA(inst), INST_D_SIMM(inst));
		break;
	}
	#endif
	case 14: { /* addi */
		do_addi(state, INST_D_rD(inst), INST_D_rA(inst), INST_D_SIMM(inst));
		break;
	}
	case 15: { /* addis */
		do_addis(state, INST_D_rD(inst), INST_D_rA(inst), INST_D_SIMM(inst));
		break;
	}
	case 18: { /* branch */
		do_branch(state, INST_I_LI(inst), INST_I_AA(inst), INST_I_LK(inst));
		break;
	}
	case 19: { /* X form instructions */
		printf("XO opcode: %d\r\n", INST_XL_XO(inst));
		opc19_handlers[INST_XL_XO(inst)](state, inst);
		break;
	}
	case 24: { /* ori */
		do_ori(state, INST_D_rS(inst), INST_D_rA(inst), INST_D_UIMM(inst));
		break;
	}
	case 25: { /* oris */
		do_oris(state, INST_D_rS(inst), INST_D_rA(inst), INST_D_UIMM(inst));
		break;
	}
	#if 0
	case 28: { /* andi. */
		do_andi(state, INST_D_rS(inst), INST_D_rA(inst), INST_D_UIMM(inst));
		break;
	}
	#endif
	case 31: { /* X form instructions */
		printf("XO opcode: %d\r\n", INST_XO_XO(inst));
		opc31_handlers[INST_XO_XO(inst)](state, inst);
		break;
	}
	case 32: { /* lwz */
		do_basic_load(state, 4, INST_D_rD(inst), INST_D_rA(inst), INST_D_D(inst));
		break;
	}
	case 34: { /* lbz */
		do_basic_load(state, 1, INST_D_rD(inst), INST_D_rA(inst), INST_D_D(inst));
		break;
	}
	case 36: { /* stw */
		do_basic_store(state, 4, INST_D_rS(inst), INST_D_rA(inst), INST_D_D(inst));
		break;
	}
	case 38: { /* stb */
		do_basic_store(state, 1, INST_D_rS(inst), INST_D_rA(inst), INST_D_D(inst));
		break;
	}
	case 40: { /* lhz */
		do_basic_load(state, 2, INST_D_rD(inst), INST_D_rA(inst), INST_D_D(inst));
		break;
	}
	case 44: { /* sth */
		do_basic_store(state, 2, INST_D_rS(inst), INST_D_rA(inst), INST_D_D(inst));
		break;
	}
	default: {
		printf("Illegal instruction: 0x%08x\r\n", inst);
		do_illegal(state, inst);
		break;
	}
	}

	if (!state->branched)
		state->pc += 4;
}
