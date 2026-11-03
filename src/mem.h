/*
 * libppcemu - Internal Headers - Memory handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_MEM_H
#define _LIBPPCEMU_INTERNAL_MEM_H

#include <ppcemu/msr.h>
#include "state.h"
#include "types.h"

enum virt2phys_err {
	V2P_SUCCESS,
	V2P_NOT_MAPPED,
	V2P_NO_PERMS,
	V2P_DIRECT_STORE
};

/* converts a virtual address to a physical one, returning various values if it failed */
enum virt2phys_err ppcemu_virt2phys_slow(struct _ppcemu_state *state, u32 virt, u32 *phys, bool *cacheable, bool ifetch, bool write);

/* fast path: translation on and a micro-TLB hit; everything else goes to the slow path */
static inline enum virt2phys_err ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool *cacheable, bool ifetch, bool write) {
	if (state->msr & (ifetch ? PPCEMU_MSR_IR : PPCEMU_MSR_DR)) {
		if (tlb_lookup(ifetch ? &state->itlb : &state->dtlb, virt >> 12, (state->msr & PPCEMU_MSR_PR) != 0, write, virt, phys, cacheable))
			return V2P_SUCCESS;
	}

	return ppcemu_virt2phys_slow(state, virt, phys, cacheable, ifetch, write);
}

const char *v2p_strerror(enum virt2phys_err err);
void ppcemu_set_dsi_info(struct _ppcemu_state *state, u32 ea, enum virt2phys_err err, bool write);
void ppcemu_set_isi_info(struct _ppcemu_state *state, enum virt2phys_err err);
enum virt2phys_err _do_basic_store_nohook(struct _ppcemu_state *state, uint len, u32 ea, void *val);
enum virt2phys_err _do_basic_load_nohook(struct _ppcemu_state *state, uint len, u32 ea, void *val);

static inline enum virt2phys_err _do_basic_load(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	u32 phys;
	bool cacheable;
	struct cacheline *line;
	uint off;

	if (state->loadstore_hook)
		state->loadstore_hook((struct ppcemu_state *)state, ea, len, val, false);

	if ((state->msr & PPCEMU_MSR_DR) &&
	    tlb_lookup(&state->dtlb, ea >> 12, (state->msr & PPCEMU_MSR_PR) != 0, false, ea, &phys, &cacheable) && cacheable) {
		line = &state->dcache.lines[(phys >> 5) & (state->dcache.line_count - 1)];
		off = phys & CACHE_LINE_MASK;

		if (line->valid && line->tag == (phys & ~CACHE_LINE_MASK) && off + len <= CACHE_LINE_SIZE) {
			switch (len) {
			/* FIXME: these are faster on my Mac, but are they actually portable? */
			case 1: __builtin_memcpy(val, &line->data[off], 1); return V2P_SUCCESS;
			case 2: __builtin_memcpy(val, &line->data[off], 2); return V2P_SUCCESS;
			case 4: __builtin_memcpy(val, &line->data[off], 4); return V2P_SUCCESS;
			case 8: __builtin_memcpy(val, &line->data[off], 8); return V2P_SUCCESS;
			default: break;
			}
		}
	}

	return _do_basic_load_nohook(state, len, ea, val);
}

static inline enum virt2phys_err _do_basic_store(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	u32 phys;
	bool cacheable;
	struct cacheline *line;
	uint off;

	if (state->loadstore_hook)
		state->loadstore_hook((struct ppcemu_state *)state, ea, len, val, true);

	if ((state->msr & PPCEMU_MSR_DR) &&
	    tlb_lookup(&state->dtlb, ea >> 12, (state->msr & PPCEMU_MSR_PR) != 0, true, ea, &phys, &cacheable) && cacheable) {
		line = &state->dcache.lines[(phys >> 5) & (state->dcache.line_count - 1)];
		off = phys & CACHE_LINE_MASK;

		if (line->valid && line->tag == (phys & ~CACHE_LINE_MASK) && off + len <= CACHE_LINE_SIZE) {
			switch (len) {
			/* FIXME: these are faster on my Mac, but are they actually portable? */
			case 1: __builtin_memcpy(&line->data[off], val, 1); line->dirty = 1; return V2P_SUCCESS;
			case 2: __builtin_memcpy(&line->data[off], val, 2); line->dirty = 1; return V2P_SUCCESS;
			case 4: __builtin_memcpy(&line->data[off], val, 4); line->dirty = 1; return V2P_SUCCESS;
			case 8: __builtin_memcpy(&line->data[off], val, 8); line->dirty = 1; return V2P_SUCCESS;
			default: break;
			}
		}
	}

	return _do_basic_store_nohook(state, len, ea, val);
}

static inline void ppcemu_ifetch_invalidate(struct _ppcemu_state *state) {
	state->ifetch_valid = false;
}

void ppcemu_tlb_flush_all(struct _ppcemu_state *state);
void ppcemu_tlb_flush_segment(struct _ppcemu_state *state, uint sreg);
void ppcemu_tlb_flush_page(struct _ppcemu_state *state, u32 virt);

#endif /* _LIBPPCEMU_INTERNAL_MEM_H */
