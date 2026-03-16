/*
 * libppcemu - Internal Headers - PowerPC Instruction decoding
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_DECODE_H
#define _LIBPPCEMU_INTERNAL_DECODE_H

#include "exception.h"
#include "state.h"
#include "types.h"

/* mix of https://www.ibm.com/docs/en/aix/7.1.0?topic=overview-instruction-forms and the 750CL user manual */
#define INST_OPCD(inst)      (((inst) & 0xfc000000) >> 26)

/* 0..5 = Opcode; 6..29 = LI; 30 = AA; 31 = LK */
#define INST_I_LI(inst)      (((inst) & 0x03fffffc) >> 2)
#define INST_I_AA(inst)      (((inst) & 0x00000002) >> 1)
#define INST_I_LK(inst)      (((inst) & 0x00000001) >> 0)

/* 0..5 = Opcode; 6..10 = rD/rS/SR; 11..15 = rA; 16..20 = rB; 21 = OE; 22..30 = XO; 31 = Rc */
#define INST_XO_rD(inst)     (((inst) & 0x03e00000) >> 21)
#define INST_XO_rS(inst)     INST_XO_rD(inst)
#define INST_XO_frD(inst)    INST_XO_rD(inst)
#define INST_XO_crbD(inst)   INST_XO_rD(inst)
#define INST_XO_frS(inst)    INST_XO_rD(inst)
#define INST_XO_SR(inst)     INST_XO_rD(inst)
#define INST_XO_rA(inst)     (((inst) & 0x001f0000) >> 16)
#define INST_XO_rB(inst)     (((inst) & 0x0000f800) >> 11)
#define INST_XO_frB(inst)    INST_XO_rB(inst)
#define INST_XO_SH(inst)     INST_XO_rB(inst)
#define INST_XO_OE(inst)     (((inst) & 0x00000400) >> 10)
#define INST_XO_XO(inst)     (((inst) & 0x000003fe) >> 1)
#define INST_XO_Rc(inst)     (((inst) & 0x00000001) >> 0)

/* 0..5 = Opcode; 6..10 = rD/rS; 11..20 = spr/I/FXM+I/0+CRM+0/rA+rB; 21..30 = XO; 31 = Rc */
#define INST_XFX_rD(inst)    (((inst) & 0x03e00000) >> 21)
#define INST_XFX_rS(inst)    INST_XFX_rD(inst)
#define INST_XFX_CRM(inst)   (((inst) & 0x000ff000) >> 12)
#define INST_XFX_I(inst)     (((inst) & 0x001ff800) >> 11)
#define INST_XFX_XO(inst)    (((inst) & 0x000007fe) >> 1)
#define INST_XFX_Rc(inst)    (((inst) & 0x00000001) >> 0)

/* 0..5 = Opcode; 6..15 = I/FM; 16..20 = frB; 21..30 = XO; 31 = Rc */
#define INST_XFL_FM(inst)    (((inst) & 0x01fe0000) >> 17)
#define INST_XFL_I(inst)     (((inst) & 0x03ff0000) >> 16)
#define INST_XFL_frB(inst)   (((inst) & 0x0000f800) >> 11)
#define INST_XFL_XO(inst)    (((inst) & 0x000007fe) >> 1)
#define INST_XFL_Rc(inst)    (((inst) & 0x00000001) >> 0)

/* same as XFX */
#define INST_XL_rD(inst)     INST_XFX_rD(inst)
#define INST_XL_rS(inst)     INST_XFX_rS(inst)
#define INST_XL_crbD(inst)   INST_XFX_rD(inst)
#define INST_XL_BO(inst)     INST_XL_rS(inst)
#define INST_XL_crbA(inst)   (((inst) & 0x001f0000) >> 16)
#define INST_XL_crbB(inst)   (((inst) & 0x0000f800) >> 11)
#define INST_XL_I(inst)      INST_XFX_I(inst)
#define INST_XL_BI(inst)     (INST_XL_I(inst) >> 5)
#define INST_XL_XO(inst)     INST_XFX_XO(inst)
#define INST_XL_Rc(inst)     INST_XFX_Rc(inst)
#define INST_XL_LK(inst)     INST_XL_Rc(inst)

/* 0..5 = Opcode; 6..10 = rD/rS/frD/frS/TO/BF/I/L/BI; 11..15 = rA/BO; 16..31 = D/SIMM/UIMM/BD+AA+LK */
#define INST_D_rD(inst)      (((inst) & 0x03e00000) >> 21)
#define INST_D_rS(inst)      INST_D_rD(inst)
#define INST_D_frD(inst)     INST_D_rD(inst)
#define INST_D_frS(inst)     INST_D_rD(inst)
#define INST_D_TO(inst)      INST_D_rD(inst)
#define INST_D_BF(inst)      INST_D_rD(inst)
#define INST_D_I(inst)       INST_D_rD(inst)
#define INST_D_L(inst)       INST_D_rD(inst)
#define INST_D_BO(inst)      INST_D_rD(inst)
#define INST_D_rA(inst)      (((inst) & 0x001f0000) >> 16)
#define INST_D_BI(inst)      INST_D_rA(inst)
#define INST_D_D(inst)       (((inst) & 0x0000ffff) >> 0)
#define INST_D_SIMM(inst)    INST_D_D(inst)
#define INST_D_UIMM(inst)    INST_D_D(inst)
#define INST_D_BD(inst)      (((inst) & 0x0000fffc) >> 2)
#define INST_D_AA(inst)      (((inst) & 0x00000002) >> 1)
#define INST_D_LK(inst)      (((inst) & 0x00000001) >> 0)

/* 0..5 = Opcode; 6..10 = rS; 11..15 = rA; 16..20 = rB/SH; 21..25 = MB; 26..30 = ME; 31 = Rc */
#define INST_M_rS(inst)      (((inst) & 0x03e00000) >> 21)
#define INST_M_rA(inst)      (((inst) & 0x001f0000) >> 16)
#define INST_M_rB(inst)      (((inst) & 0x0000f800) >> 11)
#define INST_M_SH(inst)      INST_M_rB(inst)
#define INST_M_MB(inst)      (((inst) & 0x000007c0) >> 6)
#define INST_M_ME(inst)      (((inst) & 0x0000003e) >> 1)
#define INST_M_Rc(inst)      (((inst) & 0x00000001) >> 0)

/* TODO: guessing based on what binutils does; this form isn't public */
/* 0..5 = Opcode; 6..10 = frS/frD; 11..15 = rA; 16 = W; 17..19 = PSQ; 20..31 = D */
#define INST_PS_frS(inst)    (((inst) & 0x03e00000) >> 21)
#define INST_PS_frD(inst)    INST_PS_frS(inst)
#define INST_PS_rA(inst)     (((inst) & 0x001f0000) >> 16)
#define INST_PS_W(inst)      (((inst) & 0x00008000) >> 15)
#define INST_PS_PSQ(inst)    (((inst) & 0x00007000) >> 12)
#define INST_PS_D(inst)      (((inst) & 0x00000fff) >> 0)

#define NO_RC() if (INST_XFX_Rc(inst)) { exception_fire(state, EXCEPTION_PROGRAM); return; }

void _ppcemu_decode_exec(struct _ppcemu_state *state, u32 inst);

#endif /* _LIBPPCEMU_INTERNAL_DECODE_H */
