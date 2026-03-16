/*
 * libppcemu - Types
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_TYPES_H
#define _LIBPPCEMU_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* can't include state.h due to recursive includes, so declare the opaque ppcemu_state here */
struct ppcemu_state;

/* The specific CPU model to emulate */
enum ppcemu_cpu_model {
	PPCEMU_CPU_MODEL_750CXE,
	PPCEMU_CPU_MODEL_750CL,
	PPCEMU_CPU_MODEL_GEKKO,
	PPCEMU_CPU_MODEL_BROADWAY,
	PPCEMU_CPU_MODEL_ESPRESSO
};

/* Cache mode */
enum ppcemu_cache_mode {
	/* caches are enabled */
	PPCEMU_CACHE_MODE_STANDARD,
	/* caches are enabled, D$ writeback on translation-off, D$+I$ invalidate on translation-on */
	PPCEMU_CACHE_MODE_PERMISSIVE,
	/* caches are disabled (direct bus access for everything) */
	PPCEMU_CACHE_MODE_DISABLED
};

/* type for bus hooks */
typedef void (*ppcemu_bus_hook)(struct ppcemu_state *emu, uint32_t address, unsigned int len, void *data, bool is_write);

/* type for the load/store hook */
typedef void (*ppcemu_loadstore_hook)(struct ppcemu_state *emu, uint32_t address, unsigned int len, void *data, bool is_write);

#endif /* _LIBPPCEMU_TYPES_H */
