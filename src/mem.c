/*
 * libppcemu - Memory
 *
 * Copyright (C) 2026 Techflash
 *
 * BAT handling code based on code from dol-tools' dol-run:
 * Copyright (C) 2025 Techflash
 */

#include "ppcemu/state.h"
#include "ppcemu/types.h"
#define LOG_LEVEL virt2phys_loglevel
#include <stdio.h>
#include <string.h>
#include <ppcemu/endian.h>
#include <ppcemu/msr.h>
#include "cache.h"
#include "caps.h"
#include "exception.h"
#include "log.h"
#include "mem.h"
#include "ppcemu/spr.h"
#include "spr.h"
#include "state.h"
#include "types.h"

#ifdef DEBUG_VIRT2PHYS
#define mem_debug debug
#else
static void mem_debug(const char *fmt, ...) {
	(void)fmt;
}
#endif

/*
 * Equivalent to ppcemu_sprn_to_idx(IBAT0U/IBAT4U/... + off), but computed
 * directly instead of going through ppcemu_sprn_to_idx_raw()'s switch for
 * performance.
 */
static uint bat_to_spr_idx(uint batnum, bool ibat, bool upper) {
	uint idx;

	if (batnum <= 3) idx = 13 + 2 * batnum;
	else if (batnum >= 4 && batnum <= 7) idx = 29 + 2 * (batnum - 4);
	else { assert(!"Unreachable"); return 0; }

	if (!ibat) idx += 8;
	if (!upper) idx += 1;

	return idx;
}

#define PTEG_SIZE          64 /* 8 PTEs of 8 bytes each */
#define PTES_PER_PTEG       8
#define PTE_SIZE            8

/* whether a page access is allowed, given the SR key, PTE PP bits, and access type */
static bool pte_access_allowed(bool key, u32 pp, bool write) {
	if (!key)
		return (pp == PPCEMU_PTE_PP_RO) ? !write : true;

	switch (pp) {
	case PPCEMU_PTE_PP_NA: return false;
	case PPCEMU_PTE_PP_RO_KEY: return !write;
	case PPCEMU_PTE_PP_RW: return true;
	case PPCEMU_PTE_PP_RO: return !write;
	default: return false;
	}
}

static bool real_mode_cacheable(struct _ppcemu_state *state, bool ifetch) {
	return (state->cache_mode != PPCEMU_CACHE_MODE_DISABLED) &&
	       (state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID0)] & (ifetch ? PPCEMU_HID0_ICE : PPCEMU_HID0_DCE));
}

static void htab_read_pte(struct _ppcemu_state *state, u32 pteg_addr, uint idx, u32 *w0, u32 *w1) {
	/* pteg_addr is already physical (derived from SDR1) */
	u32 addr = pteg_addr + idx * PTE_SIZE;
	u32 raw0, raw1;

	if (real_mode_cacheable(state, false)) {
		ppcemu_dcache_load(&state->dcache, addr, 4, &raw0);
		ppcemu_dcache_load(&state->dcache, addr + 4, 4, &raw1);
	}
	else {
		state->bus_hook((struct ppcemu_state *)state, addr, 4, &raw0, false);
		state->bus_hook((struct ppcemu_state *)state, addr + 4, 4, &raw1, false);
	}
	*w0 = ppcemu_be32_to_cpu(raw0);
	*w1 = ppcemu_be32_to_cpu(raw1);
}

static void htab_write_pte1(struct _ppcemu_state *state, u32 pteg_addr, uint idx, u32 w1) {
	u32 addr = pteg_addr + idx * PTE_SIZE + 4;
	u32 raw = ppcemu_cpu_to_be32(w1);

	if (real_mode_cacheable(state, false))
		ppcemu_dcache_store(&state->dcache, addr, 4, &raw);
	else
		state->bus_hook((struct ppcemu_state *)state, addr, 4, &raw, true);
}

/*
 * Walk the hashed page table (SDR1/PTEG) for a non-direct-store segment.
 * Called only once BAT translation has missed and the segment's T bit is
 * clear.
 */
static enum virt2phys_err htab_lookup(struct _ppcemu_state *state, u32 virt, u32 sr, u32 *phys, bool *cacheable, bool ifetch, bool write, bool *out_writable, bool *out_dirty_pending) {
	u32 sdr1, vsid, pi, api, hpt_mask, htaborg, hash, pteg_addr, w0, w1, pp, wimg, this_hash;
	bool key, emu_cache_enabled, dce, ice;
	uint h, i;

	sdr1 = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SDR1)];
	vsid = (sr & PPCEMU_SR_VSID) >> PPCEMU_SR_VSID_SHIFT;
	pi = (virt >> 12) & 0xffffu;
	api = pi >> 10;
	key = (state->msr & PPCEMU_MSR_PR) ? ((sr & PPCEMU_SR_KP) != 0) : ((sr & PPCEMU_SR_KS) != 0);

	htaborg = sdr1 & PPCEMU_SDR1_HTABORG;
	hpt_mask = ((sdr1 & PPCEMU_SDR1_HTABMASK) << 16) | 0xffffu;
	hash = vsid ^ pi;

	emu_cache_enabled = state->cache_mode != PPCEMU_CACHE_MODE_DISABLED;
	dce = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID0)] & PPCEMU_HID0_DCE;
	ice = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID0)] & PPCEMU_HID0_ICE;

	for (h = 0; h < 2; h++) {
		this_hash = h ? ~hash : hash;

		pteg_addr = htaborg | ((this_hash << 6) & hpt_mask);

		for (i = 0; i < PTES_PER_PTEG; i++) {
			htab_read_pte(state, pteg_addr, i, &w0, &w1);

			if (!(w0 & PPCEMU_PTE_V))
				continue;
			if (!!(w0 & PPCEMU_PTE_H) != (h != 0))
				continue;
			if (((w0 & PPCEMU_PTE_VSID) >> PPCEMU_PTE_VSID_SHIFT) != vsid)
				continue;
			if (((w0 & PPCEMU_PTE_API) >> PPCEMU_PTE_API_SHIFT) != api)
				continue;

			/* found the PTE */
			pp = (w1 & PPCEMU_PTE_PP) >> PPCEMU_PTE_PP_SHIFT;
			if (!pte_access_allowed(key, pp, write))
				return V2P_NO_PERMS;

			*out_writable = pte_access_allowed(key, pp, true);
			*out_dirty_pending = !write && !(w1 & PPCEMU_PTE_C);

			if (!(w1 & PPCEMU_PTE_R) || (write && !(w1 & PPCEMU_PTE_C)))
				htab_write_pte1(state, pteg_addr, i, w1 | PPCEMU_PTE_R | (write ? PPCEMU_PTE_C : 0));

			*phys = (w1 & PPCEMU_PTE_RPN) | (virt & 0xfff);

			wimg = w1 & (PPCEMU_PTE_W | PPCEMU_PTE_I | PPCEMU_PTE_M | PPCEMU_PTE_G);
			if (!(wimg & PPCEMU_PTE_I) && ((ifetch && ice) || (!ifetch && dce)) && emu_cache_enabled)
				*cacheable = true; /* already false from the caller */

			return V2P_SUCCESS;
		}
	}

	return V2P_NOT_MAPPED;
}

static uint bat_blocklen_to_bytes(u32 bl) {
	if ((bl + 1) & bl)
		return 0;

	return (128 * 1024) * (bl + 1);
}

void ppcemu_tlb_flush_all(struct _ppcemu_state *state) {
	uint i;

	for (i = 0; i < MICRO_TLB_WAYS; i++) {
		state->itlb.ways[i].valid = false;
		state->dtlb.ways[i].valid = false;
	}
	state->itlb.next_way = 0;
	state->dtlb.next_way = 0;
}

void ppcemu_tlb_flush_segment(struct _ppcemu_state *state, uint sreg) {
	uint i;

	for (i = 0; i < MICRO_TLB_WAYS; i++) {
		if (state->itlb.ways[i].valid && (state->itlb.ways[i].vpn >> 16) == sreg)
			state->itlb.ways[i].valid = false;
		if (state->dtlb.ways[i].valid && (state->dtlb.ways[i].vpn >> 16) == sreg)
			state->dtlb.ways[i].valid = false;
	}
}

/* As ppcemu_tlb_flush_segment(), but for tlbie: invalidates just the one page named by virt. */
void ppcemu_tlb_flush_page(struct _ppcemu_state *state, u32 virt) {
	u32 vpn = virt >> 12;
	uint i;

	for (i = 0; i < MICRO_TLB_WAYS; i++) {
		if (state->itlb.ways[i].valid && state->itlb.ways[i].vpn == vpn)
			state->itlb.ways[i].valid = false;
		if (state->dtlb.ways[i].valid && state->dtlb.ways[i].vpn == vpn)
			state->dtlb.ways[i].valid = false;
	}
}

static bool tlb_lookup(struct micro_tlb *tlb, u32 vpn, bool pr, bool write, u32 virt, u32 *phys, bool *cacheable) {
	uint i;

	for (i = 0; i < MICRO_TLB_WAYS; i++) {
		struct micro_tlb_entry *e = &tlb->ways[i];

		if (!e->valid || e->vpn != vpn || e->pr != pr)
			continue;
		if (write && (!e->writable || e->dirty_pending))
			return false;

		*phys = e->phys_base | (virt & 0xfffu);
		*cacheable = e->cacheable;
		return true;
	}

	return false;
}

static void tlb_insert(struct micro_tlb *tlb, u32 vpn, bool pr, u32 phys_base, bool cacheable, bool writable, bool dirty_pending) {
	struct micro_tlb_entry *e;
	uint i;

	for (i = 0; i < MICRO_TLB_WAYS; i++) {
		if (tlb->ways[i].valid && tlb->ways[i].vpn == vpn && tlb->ways[i].pr == pr) {
			e = &tlb->ways[i];
			goto fill;
		}
	}

	e = &tlb->ways[tlb->next_way];
	tlb->next_way = (tlb->next_way + 1) % MICRO_TLB_WAYS;

fill:
	e->valid = true;
	e->vpn = vpn;
	e->pr = pr;
	e->phys_base = phys_base;
	e->cacheable = cacheable;
	e->writable = writable;
	e->dirty_pending = dirty_pending;
}

enum virt2phys_err HIDDEN ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool *cacheable, bool ifetch, bool write) {
	u32 batu, batl, size, bepi, pp, sr, wimg, vpn, phys_base;
	uint i, max_bat;
	bool vs, vp, pr, writable, dirty_pending, emu_cache_enabled, dce, ice;
	struct micro_tlb *tlb;
	enum virt2phys_err err;

	*cacheable = false;
	if (ifetch && !(state->msr & PPCEMU_MSR_IR)) {
		*phys = virt;
		*cacheable = real_mode_cacheable(state, ifetch);
		return V2P_SUCCESS;
	}
	else if (!ifetch && !(state->msr & PPCEMU_MSR_DR)) {
		*phys = virt;
		*cacheable = real_mode_cacheable(state, ifetch);
		return V2P_SUCCESS;
	}

	tlb = ifetch ? &state->itlb : &state->dtlb;
	vpn = virt >> 12;
	pr = (state->msr & PPCEMU_MSR_PR) != 0;

	if (tlb_lookup(tlb, vpn, pr, write, virt, phys, cacheable))
		return V2P_SUCCESS;

	emu_cache_enabled = state->cache_mode != PPCEMU_CACHE_MODE_DISABLED;
	dce = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID0)] & PPCEMU_HID0_DCE;
	ice = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID0)] & PPCEMU_HID0_ICE;

	*cacheable = false;
	if (state->caps & CAPS_UPPER_BATS &&
	    state->caps & CAPS_HID4 &&
	    state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID4)] & PPCEMU_HID4_SBE)
		max_bat = 8;
	else
		max_bat = 4;

	for (i = 0; i < max_bat; i++) {
		batu = state->sprs[bat_to_spr_idx(i, ifetch, true)];
		batl = state->sprs[bat_to_spr_idx(i, ifetch, false)];

		/*
		 * Skip BATs that do not apply to the current privilege level: Vs
		 * validates a BAT for supervisor mode, Vp for user (problem) mode.
		 * A BAT with both clear is simply inactive.
		 */
		vs = (batu & PPCEMU_BATU_VS) != 0;
		vp = (batu & PPCEMU_BATU_VP) != 0;
		if (state->msr & PPCEMU_MSR_PR) {
			if (!vp)
				continue;
		}
		else {
			if (!vs)
				continue;
		}

		/* Compute BAT block size */
		size = bat_blocklen_to_bytes((batu & PPCEMU_BATU_BL) >> PPCEMU_BATU_BL_SHIFT);

		/* Compute virtual base */
		bepi = (batu & PPCEMU_BATU_BEPI);
		mem_debug("MEM: Checking if vaddr 0x%08x lives in %cBAT%d (%d bytes @ 0x%08x)...\r\n", virt, ifetch ? 'I' : 'D', i, size, bepi);

		/* Check if VA falls in this BAT */
		if (virt >= bepi && (virt - bepi) < size) {
			mem_debug("MEM: It does!\r\n");
			/* Compute physical base */
			*phys = ((batl & PPCEMU_BATL_BPRN) & ~(size - 1)) + (virt - bepi);

			/* Determine access permissions */
			pp = (batl & PPCEMU_BATL_PP) >> PPCEMU_BATL_PP_SHIFT;
			if (write && pp != PPCEMU_BATL_PP_RW)
				return V2P_NO_PERMS;
			else if (!write && pp == PPCEMU_BATL_PP___)
				return V2P_NO_PERMS;

			/* Determine if it can hit the cache by checking the cache inhibit bit */
			wimg = (batl & PPCEMU_BATL_WIMG) >> PPCEMU_BATL_WIMG_SHIFT;
			if (!(wimg & 0x4) && ((ifetch && ice) || (!ifetch && dce)) && emu_cache_enabled)
				*cacheable = true; /* already set to false above */

			phys_base = *phys & ~0xfffu;
			writable = (pp == PPCEMU_BATL_PP_RW);
			tlb_insert(tlb, vpn, pr, phys_base, *cacheable, writable, false);

			return V2P_SUCCESS;
		}
		mem_debug("MEM: It does not, continuing...\r\n");
	}

	/* BAT translation takes precedence over segment/page translation */
	sr = state->sr[virt >> 28];
	if (sr & PPCEMU_SR_T)
		return V2P_DIRECT_STORE;

	err = htab_lookup(state, virt, sr, phys, cacheable, ifetch, write, &writable, &dirty_pending);
	if (err == V2P_SUCCESS) {
		phys_base = *phys & ~0xfffu;
		tlb_insert(tlb, vpn, pr, phys_base, *cacheable, writable, dirty_pending);
	}

	return err;
}

const char *v2p_strerror(enum virt2phys_err err) {
	switch (err) {
	case V2P_SUCCESS: return "Success";
	case V2P_NOT_MAPPED: return "Not Mapped";
	case V2P_NO_PERMS: return "Invalid Permissions";
	case V2P_DIRECT_STORE: return "Direct Store";
	default: return "???";
	}
}

void HIDDEN ppcemu_set_dsi_info(struct _ppcemu_state *state, u32 ea, enum virt2phys_err err, bool write) {
	u32 dsisr = write ? PPCEMU_DSISR_STORE : 0;

	switch (err) {
	case V2P_NOT_MAPPED:
		dsisr |= PPCEMU_DSISR_NOT_FOUND;
		break;
	case V2P_NO_PERMS:
		dsisr |= PPCEMU_DSISR_PROTECTION;
		break;
	case V2P_DIRECT_STORE:
		dsisr |= PPCEMU_DSISR_DIRECT_STORE;
		break;
	case V2P_SUCCESS:
		break;
	}

	state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DSISR)] = dsisr;
	state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_DAR)] = ea;
}

void HIDDEN ppcemu_set_isi_info(struct _ppcemu_state *state, enum virt2phys_err err) {
	u32 extra;

	switch (err) {
	case V2P_NOT_MAPPED:
	case V2P_DIRECT_STORE: /* no dedicated SRR1 bit; NOPT is the closest fit */
		extra = PPCEMU_SRR1_ISI_NOPT;
		break;
	case V2P_NO_PERMS:
		extra = PPCEMU_SRR1_ISI_PROT;
		break;
	case V2P_SUCCESS:
	default:
		extra = 0;
		break;
	}

	state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_SRR1)] |= extra;
}

static void do_wgp_flush(struct _ppcemu_state *state) {
	u32 wpar = state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_WPAR)];

	state->bus_hook((struct ppcemu_state *)state, wpar, 32, state->wgp_buf, true);
	state->cur_wgp_idx = 0;
}

static void do_wgp_store(struct _ppcemu_state *state, uint len, void *_val) {
	int wraparound = ((int)(state->cur_wgp_idx + len)) - 32;
	u8 *val = _val;

	/* filling */
	if (wraparound < 0) {
		memcpy(&state->wgp_buf[state->cur_wgp_idx], val, len);
		state->cur_wgp_idx += len;
		return;
	}

	/* exactly fills 32B, flush */
	else if (wraparound == 0) {
		memcpy(&state->wgp_buf[state->cur_wgp_idx], val, len);
		state->cur_wgp_idx += len;
		do_wgp_flush(state);
		return;
	}

	/* wrap around */
	else if (wraparound > 0) {
		memcpy(&state->wgp_buf[state->cur_wgp_idx], val, len - wraparound);
		do_wgp_flush(state);
		val += (len - wraparound);
		len = wraparound;
		memcpy(&state->wgp_buf[state->cur_wgp_idx], val, len);
		state->cur_wgp_idx += len;
	}
}

/*
 * An unaligned access can span independently mapped pages. Translate both
 * before touching data.
 */
static enum virt2phys_err access_page_tail(struct _ppcemu_state *state, u32 ea, uint len, u32 phys, bool cacheable, void *val, bool write) {
	uint first = 0x1000u - (ea & 0xfffu), i;
	u32 next_phys, addr;
	bool next_cacheable, cached;
	u8 *byte;
	enum virt2phys_err err = ppcemu_virt2phys(state, ea + first, &next_phys, &next_cacheable, false, write);

	if (err != V2P_SUCCESS) {
		ppcemu_set_dsi_info(state, ea + first, err, write);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	for (i = 0; i < len; i++) {
		addr = i < first ? phys + i : next_phys + i - first;
		cached = i < first ? cacheable : next_cacheable;
		byte = (u8 *)val + i;

		if (cached) {
			if (write)
				ppcemu_dcache_store(&state->dcache, addr, 1, byte);
			else
				ppcemu_dcache_load(&state->dcache, addr, 1, byte);
		}
		else
			state->bus_hook((struct ppcemu_state *)state, addr, 1, byte, write);
	}

	return V2P_SUCCESS;
}

enum virt2phys_err _do_basic_store(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;
	bool cacheable;

	if (state->loadstore_hook)
		state->loadstore_hook((struct ppcemu_state *)state, ea, len, val, true);

	err = ppcemu_virt2phys(state, ea, &phys, &cacheable, false, true);
	if (err != V2P_SUCCESS) {
		warn("_do_basic_store: store %uB to 0x%08x @ PC=0x%08x: virt2phys error: %s (%d)\r\n", len, ea, state->pc, v2p_strerror(err), err);
		ppcemu_set_dsi_info(state, ea, err, true);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	if ((ea & 0xfffu) + len > 0x1000u)
		return access_page_tail(state, ea, len, phys, cacheable, val, true);

	if (cacheable)
		ppcemu_dcache_store(&state->dcache, phys, len, val);
	else {
		if ((state->caps & CAPS_WR_GATHER_PIPE) &&
		    (state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_HID2_GEKKO)] & PPCEMU_HID2_WPE) &&
		    (phys & ~31) == state->sprs[ppcemu_sprn_to_idx(PPCEMU_SPRN_WPAR)])
			do_wgp_store(state, len, val);
		else
			state->bus_hook((struct ppcemu_state *)state, phys, len, val, true);
	}
	return err;
}

enum virt2phys_err _do_basic_load(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;
	bool cacheable;

	if (state->loadstore_hook)
		state->loadstore_hook((struct ppcemu_state *)state, ea, len, val, false);

	err = ppcemu_virt2phys(state, ea, &phys, &cacheable, false, false);
	if (err != V2P_SUCCESS) {
		warn("_do_basic_load: load %uB from 0x%08x @ PC=0x%08x: virt2phys error: %s (%d)\r\n", len, ea, state->pc, v2p_strerror(err), err);
		ppcemu_set_dsi_info(state, ea, err, false);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	if ((ea & 0xfffu) + len > 0x1000u)
		return access_page_tail(state, ea, len, phys, cacheable, val, false);

	if (cacheable)
		ppcemu_dcache_load(&state->dcache, phys, len, val);
	else
		state->bus_hook((struct ppcemu_state *)state, phys, len, val, false);

	return err;
}
