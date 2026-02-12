/*
 * libppcemu - Internal Headers - Exception handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_EXCEPTION_H
#define _LIBPPCEMU_INTERNAL_EXCEPTION_H

#include "state.h"
#include "types.h"
#include <ppcemu/msr.h>

#define EXCEPTION_RESET   0x0100
#define EXCEPTION_MCHK    0x0200
#define EXCEPTION_DSI     0x0300
#define EXCEPTION_ISI     0x0400
#define EXCEPTION_EXT     0x0500
#define EXCEPTION_ALIGN   0x0600
#define EXCEPTION_PROGRAM 0x0700
#define EXCEPTION_FP_UNAV 0x0800
#define EXCEPTION_DEC     0x0900
#define EXCEPTION_SYSCALL 0x0C00
#define EXCEPTION_TRACE   0x0D00
#define EXCEPTION_FP_ASS  0x0E00
#define EXCEPTION_PERFMON 0x0F00
#define EXCEPTION_INAD_BP 0x1300
#define EXCEPTION_SM_INT  0x1400
#define EXCEPTION_THRM    0x1700

/* fire an exception */
static inline void exception_fire(struct _ppcemu_state *state, uint idx) {
	u32 prefix;

	if (state->msr & PPCEMU_MSR_IP)
		prefix = 0xfff00000;
	else
		prefix = 0x00000000;

	state->pc = prefix | (u32)idx;
	state->branched = true;
}

#endif /* _LIBPPCEMU_INTERNAL_EXCEPTION_H */
