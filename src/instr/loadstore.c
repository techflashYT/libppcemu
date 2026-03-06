/*
 * libppcemu - PowerPC Instruction handling - Load/Store
 *
 * Copyright (C) 2026 Techflash
 */

#include <stdio.h>
#include <ppcemu/endian.h>
#include "../exception.h"
#include "../mem.h"
#include "../state.h"
#include "../../config.h"

#ifdef DEBUG_LOADSTORE
#define mem_debug printf
#else
static void mem_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif

u32 do_basic_store(struct _ppcemu_state *state, uint len, uint rS, uint rA, u16 d) {
	u32 b, ea, v32;
	u16 v16;
	u8 v8;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)(i16)d;
	mem_debug("basic store: len=%u, rS=%u, rA=%u, b=0x%08x, d=%d, ea=0x%08x\r\n", len, rS, rA, b, (i32)(i16)d, ea);

	switch (len) {
	case 1: {
		v8 = (u8)state->gpr[rS];
		_do_basic_store(state, len, ea, &v8);
		break;
	}
	case 2: {
		v16 = ppcemu_cpu_to_be16((u16)state->gpr[rS]);
		_do_basic_store(state, len, ea, &v16);
		break;
	}
	case 4: {
		v32 = ppcemu_cpu_to_be32(state->gpr[rS]);
		_do_basic_store(state, len, ea, &v32);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}

	return ea;
}

u32 do_indexed_store(struct _ppcemu_state *state, uint len, uint rS, uint rA, u16 rB) {
	u32 b, ea, v32;
	u16 v16;
	u8 v8;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)state->gpr[rB];
	mem_debug("indexed store: len=%u, rS=%u, rA=%u, rB=%u, rB(val)=%d, b=0x%08x, ea=0x%08x\r\n", len, rS, rA, b, rB, (i32)state->gpr[rB], ea);

	switch (len) {
	case 1: {
		v8 = (u8)state->gpr[rS];
		_do_basic_store(state, len, ea, &v8);
		break;
	}
	case 2: {
		v16 = ppcemu_cpu_to_be16((u16)state->gpr[rS]);
		_do_basic_store(state, len, ea, &v16);
		break;
	}
	case 4: {
		v32 = ppcemu_cpu_to_be32(state->gpr[rS]);
		_do_basic_store(state, len, ea, &v32);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}

	return ea;
}

void do_stmw(struct _ppcemu_state *state, uint rS, uint rA, u16 d) {
	u32 ea, b, val;
	int r;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)(i16)d;

	for (r = rS; r <= 31; r++) {
		val = ppcemu_cpu_to_be32(state->gpr[r]);
		_do_basic_store(state, 4, ea, &val);
		ea += 4;
	}
}

u32 do_basic_load(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint d) {
	u32 b, ea, v32;
	u8 v8;
	u16 v16;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)(i16)d;

	switch (len) {
	case 1: {
		_do_basic_load(state, len, ea, &v8);
		state->gpr[rD] = (u32)v8;
		break;
	}
	case 2: {
		_do_basic_load(state, len, ea, &v16);
		state->gpr[rD] = (u32)ppcemu_be16_to_cpu(v16);
		break;
	}
	case 4: {
		_do_basic_load(state, len, ea, &v32);
		state->gpr[rD] = ppcemu_be32_to_cpu(v32);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}
	mem_debug("basic load: len=%u, rD=%u, rA=%u, b=0x%08x, d=%d, ea=0x%08x, result=0x%08x\r\n", len, rD, rA, b, (i32)(i16)d, ea, state->gpr[rD]);

	return ea;
}

u32 do_indexed_load(struct _ppcemu_state *state, uint len, uint rD, uint rA, uint rB) {
	u32 b, ea, v32;
	u8 v8;
	u16 v16;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)state->gpr[rB];

	switch (len) {
	case 1: {
		_do_basic_load(state, len, ea, &v8);
		state->gpr[rD] = (u32)v8;
		break;
	}
	case 2: {
		_do_basic_load(state, len, ea, &v16);
		state->gpr[rD] = (u32)ppcemu_be16_to_cpu(v16);
		break;
	}
	case 4: {
		_do_basic_load(state, len, ea, &v32);
		state->gpr[rD] = ppcemu_be32_to_cpu(v32);
		break;
	}
	default: {
		assert(!"Unreachable");
		break;
	}
	}
	mem_debug("indexed load: len=%u, rD=%u, rA=%u, rB=%u, rB(val)=%d, b=0x%08x, ea=0x%08x, result=0x%08x\r\n", len, rD, rA, rB, (i32)(i16)state->gpr[rB], b, ea, state->gpr[rD]);

	return ea;
}

void do_lmw(struct _ppcemu_state *state, uint rD, uint rA, u16 d) {
	u32 ea, b, val;
	int r;

	if (rA == 0)
		b = 0;
	else
		b = state->gpr[rA];

	ea = b + (i32)(i16)d;

	for (r = rD; r <= 31; r++) {
		_do_basic_load(state, 4, ea, &val);
		state->gpr[r] = ppcemu_be32_to_cpu(val);
		ea += 4;
	}
}
