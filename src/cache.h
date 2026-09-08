/*
 * libppcemu - Internal Headers - Memory caching
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_CACHE_H
#define _LIBPPCEMU_INTERNAL_CACHE_H

#include "types.h"

#define CACHE_LINE_SIZE 32u
#define CACHE_LINE_MASK (CACHE_LINE_SIZE - 1u)

struct cacheline {
	u32 tag; /* full aligned line base address */
	u8 valid;
	u8 dirty; /* only meaningful for D$ */
	u8 locked; /* only meaningful for D$: exempt from eviction */
	u8 _pad[5];
	u8 data[CACHE_LINE_SIZE];
};

struct cache {
	struct _ppcemu_state *ppcemu_state; /* pointer back to top-level state */
	struct cacheline *lines;
	uint line_count;   /* must be power of two */
	int is_data_cache;
};

extern int ppcemu_cache_init(struct _ppcemu_state *state, uint icache_size, uint dcache_size);
extern void ppcemu_cache_destroy(struct _ppcemu_state *state);
extern void ppcemu_icache_fetch(struct cache *icache, u32 phys, u32 *out);
extern void ppcemu_icache_invalidate_line(struct cache *icache, u32 phys);
extern void ppcemu_dcache_load(struct cache *dcache, u32 phys, uint size, void *out);
extern void ppcemu_dcache_store(struct cache *dcache, u32 phys, unsigned size, void *in);
extern void ppcemu_dcache_zero_line(struct cache *dcache, u32 phys);
extern void ppcemu_dcache_zero_line_locked(struct cache *dcache, u32 phys);
extern void ppcemu_dcache_writeback_line(struct cache *dcache, u32 phys);
extern void ppcemu_dcache_invalidate_line(struct cache *dcache, u32 phys);
extern void ppcemu_dcache_writeback_invalidate_line(struct cache *dcache, u32 phys);
extern void ppcemu_dcache_writeback_all(struct cache *dcache);
extern void ppcemu_cache_invalidate_all(struct cache *cache);

#endif /* _LIBPPCEMU_INTERNAL_CACHE_H */
