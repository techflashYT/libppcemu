/*
 * libppcemu - Internal Headers - Micro-TLB (virt2phys translation cache)
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_TLB_H
#define _LIBPPCEMU_INTERNAL_TLB_H

#include "types.h"

#define MICRO_TLB_WAYS 4

struct micro_tlb_entry {
	bool valid;
	bool pr;             /* MSR[PR] at the time this entry was made */
	bool writable;       /* whether a store is permitted at all */
	bool dirty_pending;
	bool cacheable;
	u32 vpn;             /* virt >> 12 */
	u32 phys_base;       /* phys of the start of this page */
};

struct micro_tlb {
	struct micro_tlb_entry ways[MICRO_TLB_WAYS];
	uint next_way;
};

#endif /* _LIBPPCEMU_INTERNAL_TLB_H */
