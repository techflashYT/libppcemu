/*
 * libppcemu - Memory caching
 *
 * Copyright (C) 2026 Techflash
 */

#include "state.h"
#define LOG_LEVEL cache_loglevel

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "log.h"
#include "mem.h"
#include "types.h"

/*
 * Utility funcs
 */
static int cache_is_power_of_two(size_t x) {
	return x != 0 && (x & (x - 1)) == 0;
}

static u32 cache_line_base(u32 addr) {
	return addr & ~(u32)CACHE_LINE_MASK;
}

static uint cache_line_offset(u32 addr) {
	return (uint)(addr & CACHE_LINE_MASK);
}

static uint cache_index_for_line(const struct cache *c, u32 line_base) {
	return (uint)((line_base >> 5) & (u32)(c->line_count - 1));
}

static struct cacheline *cache_lookup_slot(struct cache *c, u32 line_base) {
	return &c->lines[cache_index_for_line(c, line_base)];
}

/*
 * Bus helpers
 */
static enum virt2phys_err bus_write_line(struct _ppcemu_state *state, u32 addr, void *data) {
	u32 phys;
	enum virt2phys_err err;
	bool cacheable;

	/* TODO: could probably run off the end of mapping, v2p doesn't account for this */
	err = ppcemu_virt2phys(state, addr, &phys, &cacheable, false, true);
	if (err != V2P_SUCCESS)
		return err;

	state->bus_hook((struct ppcemu_state *)state, phys, CACHE_LINE_SIZE, data, true);

	return err;
}

static enum virt2phys_err bus_read_line(struct _ppcemu_state *state, u32 addr, void *data) {
	u32 phys;
	enum virt2phys_err err;
	bool cacheable;

	/* TODO: could probably run off the end of mapping, v2p doesn't account for this */
	err = ppcemu_virt2phys(state, addr, &phys, &cacheable, false, false);
	if (err != V2P_SUCCESS)
		return err;

	state->bus_hook((struct ppcemu_state *)state, phys, CACHE_LINE_SIZE, data, false);

	return err;
}


/* Initialize a cache */
static int cache_init(struct cache *c, uint cache_size, bool is_data_cache) {
	uint line_count;
	struct cacheline *lines;

	assert(c);
	assert(cache_size);
	assert(!(cache_size % CACHE_LINE_SIZE));
	line_count = cache_size / CACHE_LINE_SIZE;
	assert(cache_is_power_of_two(line_count));

	lines = malloc(line_count * sizeof(struct cacheline));
	if (!lines)
		return -1;

	c->lines = lines;
	c->line_count = line_count;
	c->is_data_cache = is_data_cache;
	return 0;
}

static void cache_destroy(struct cache *c) {
	if (!c) return;
	free(c->lines);
	c->lines = NULL;
	c->line_count = 0;
	c->is_data_cache = 0;
}

int ppcemu_cache_init(struct _ppcemu_state *state, uint icache_size, uint dcache_size) {
	memset(&state->icache, 0, sizeof(struct cache));
	memset(&state->dcache, 0, sizeof(struct cache));

	if (cache_init(&state->icache, icache_size, false))
		return -1;

	if (cache_init(&state->dcache, dcache_size, true)) {
		cache_destroy(&state->icache);
		return -1;
	}

	state->icache.ppcemu_state = state;
	state->dcache.ppcemu_state = state;

	return 0;
}

void ppcemu_cache_destroy(struct _ppcemu_state *state) {
	cache_destroy(&state->icache);
	cache_destroy(&state->dcache);
}

/* Internal cacheline management */
static void cache_writeback_slot_if_needed(struct cache *c, struct cacheline *line) {
	if (!c->is_data_cache) return;
	if (!line->valid || !line->dirty) return;
	bus_write_line(c->ppcemu_state, line->tag, line->data);
	line->dirty = 0;
}

static struct cacheline *cache_get_line(struct cache *c, u64 line_base) {
	struct cacheline *line = cache_lookup_slot(c, line_base);

	if (line->valid && line->tag == line_base)
		return line; /* hit */

	/* miss: evict old line if needed */
	if (line->valid)
		cache_writeback_slot_if_needed(c, line);

	/* fill new line */
	bus_read_line(c->ppcemu_state, line_base, line->data);
	line->tag = line_base;
	line->valid = 1;
	line->dirty = 0;

	return line;
}

/* I$ operations */
void ppcemu_icache_fetch(struct cache *icache, u32 addr, u32 *out) {
	u32 line_base;
	uint offset;
	struct cacheline *line;

	assert(icache);
	assert(!icache->is_data_cache);
	assert(!(addr & 0x3)); /* 4B alignment */

	line_base = cache_line_base(addr);
	offset = cache_line_offset(addr);

	/* with 32-byte lines and 4-byte alignment, this should never cross a line */
	assert(offset + 4 <= CACHE_LINE_SIZE);

	line = cache_get_line(icache, line_base);
	*out = *(u32 *)&line->data[offset];
}

void ppcemu_icache_invalidate_line(struct cache *icache, u32 addr) {
	u32 line_base;
	struct cacheline *line;

	verbose("icache inval: 0x%08x\r\n", addr);

	assert(icache != NULL);
	assert(!icache->is_data_cache);

	line_base = cache_line_base(addr);
	line = cache_lookup_slot(icache, line_base);

	if (line->valid && line->tag == line_base) {
		line->valid = 0;
		line->dirty = 0;
	}
}

/* D$ operations */
void ppcemu_dcache_load(struct cache *dcache, u32 addr, uint size, void *out) {
	u32 line_base;
	uint offset, n1, n2;
	struct cacheline *line, *line2;

	assert(dcache);
	assert(dcache->is_data_cache);
	/* enforce size and alignment */
	assert(size == 1 || (size == 2 && !(addr & 0x1)) || (size == 4 && !(addr & 0x3)) || (size == 8 && !(addr & 0x7)));

	line_base = cache_line_base(addr);
	offset = cache_line_offset(addr);

	if (offset + size <= CACHE_LINE_SIZE) {
		line = cache_get_line(dcache, line_base);
		memcpy(out, &line->data[offset], size);
	} else {
		/* Crosses a 32-byte boundary: split into two accesses */
		n1 = CACHE_LINE_SIZE - offset;
		n2 = size - n1;

		line = cache_get_line(dcache, line_base);
		line2 = cache_get_line(dcache, line_base + CACHE_LINE_SIZE);

		memcpy(out, &line->data[offset], n1);
		memcpy(((u8 *)out) + n1, &line2->data[0], n2);
	}
}

void ppcemu_dcache_store(struct cache *dcache, u32 addr, unsigned size, void *in) {
	u32 line_base;
	uint offset, n1, n2;
	struct cacheline *line, *line2;

	assert(dcache);
	assert(dcache->is_data_cache);
	/* enforce size and alignment */
	assert(size == 1 || (size == 2 && !(addr & 0x1)) || (size == 4 && !(addr & 0x3)) || (size == 8 && !(addr & 0x7)));

	line_base = cache_line_base(addr);
	offset = cache_line_offset(addr);

	if (offset + size <= CACHE_LINE_SIZE) {
		line = cache_get_line(dcache, line_base);
		memcpy(&line->data[offset], in, size);
		line->dirty = 1;
	} else {
		/* Crosses a 32-byte boundary: split into two stores */
		n1 = CACHE_LINE_SIZE - offset;
		n2 = size - n1;

		line = cache_get_line(dcache, line_base);
		line2 = cache_get_line(dcache, line_base + CACHE_LINE_SIZE);

		memcpy(&line->data[offset], in, n1);
		memcpy(&line2->data[0], ((u8 *)in) + n1, n2);

		line->dirty = 1;
		line2->dirty = 1;
	}
}

void ppcemu_dcache_writeback_line(struct cache *dcache, u32 addr) {
	u32 line_base;
	struct cacheline *line;

	verbose("dcache wb: 0x%08x\r\n", addr);

	assert(dcache);
	assert(dcache->is_data_cache);

	line_base = cache_line_base(addr);
	line = cache_lookup_slot(dcache, line_base);

	if (line->valid && line->tag == line_base && line->dirty) {
		bus_write_line(dcache->ppcemu_state, line->tag, line->data);
		line->dirty = 0;
	}
}

void ppcemu_dcache_invalidate_line(struct cache *dcache, u32 addr) {
	u32 line_base;
	struct cacheline *line;

	verbose("dcache inval: 0x%08x\r\n", addr);

	assert(dcache);
	assert(dcache->is_data_cache);

	line_base = cache_line_base(addr);
	line = cache_lookup_slot(dcache, line_base);

	if (line->valid && line->tag == line_base) {
		/* plain invalidate: discard without writeback */
		line->valid = 0;
		line->dirty = 0;
	}
}

void ppcemu_dcache_writeback_invalidate_line(struct cache *dcache, u32 addr) {
	u32 line_base;
	struct cacheline *line;

	verbose("dcache wb+inval: 0x%08x\r\n", addr);

	assert(dcache);
	assert(dcache->is_data_cache);

	line_base = cache_line_base(addr);
	line = cache_lookup_slot(dcache, line_base);

	if (line->valid && line->tag == line_base) {
		if (line->dirty)
			bus_write_line(dcache->ppcemu_state, line->tag, line->data);

		line->valid = 0;
		line->dirty = 0;
	}
}

void ppcemu_dcache_writeback_all(struct cache *dcache) {
	uint i;
	struct cacheline *line;

	assert(dcache);
	assert(dcache->is_data_cache);

	for (i = 0; i < dcache->line_count; i++) {
		line = &dcache->lines[i];

		if (line->dirty)
			bus_write_line(dcache->ppcemu_state, line->tag, line->data);

		line->dirty = 0;
	}
}

void ppcemu_cache_invalidate_all(struct cache *cache) {
	uint i;
	struct cacheline *line;

	assert(cache);

	for (i = 0; i < cache->line_count; ++i) {
		line = &cache->lines[i];
		line->valid = 0;
		line->dirty = 0;
	}
}
