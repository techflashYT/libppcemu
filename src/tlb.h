/*
 * libppcemu - Internal Headers - Micro-TLB (virt2phys translation cache)
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_TLB_H
#define _LIBPPCEMU_INTERNAL_TLB_H

#include "types.h"

/* direct-mapped by the low bits of the virtual page number */
#define MICRO_TLB_ENTRIES 64

#define TLB_KEY(vpn, pr) ((((u32)(vpn)) << 2) | ((pr) ? 2u : 0u) | 1u)
#define TLB_KEY_VPN(key) ((key) >> 2)

struct micro_tlb_entry {
	u32 key;             /* TLB_KEY() of the page, or 0 if the entry is invalid */
	u32 phys_base;       /* phys of the start of this page */
	bool writable;       /* whether a store is permitted at all */
	bool dirty_pending;
	bool cacheable;
};

struct micro_tlb {
	struct micro_tlb_entry ways[MICRO_TLB_ENTRIES];
};

static inline bool tlb_lookup(const struct micro_tlb *tlb, u32 vpn, bool pr, bool write, u32 virt, u32 *phys, bool *cacheable) {
	const struct micro_tlb_entry *e = &tlb->ways[vpn & (MICRO_TLB_ENTRIES - 1)];

	if (e->key != TLB_KEY(vpn, pr))
		return false;
	if (write && (!e->writable || e->dirty_pending))
		return false;

	*phys = e->phys_base | (virt & 0xfffu);
	*cacheable = e->cacheable;
	return true;
}

#endif /* _LIBPPCEMU_INTERNAL_TLB_H */
