/*
 * libppcemu - Types
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_TYPES_H
#define _LIBPPCEMU_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <ppcemu/state.h>

/* The specific CPU model to emulate */
enum ppcemu_cpu_model {
	PPCEMU_CPU_MODEL_750CXE,
	PPCEMU_CPU_MODEL_750CL,
	PPCEMU_CPU_MODEL_GEKKO,
	PPCEMU_CPU_MODEL_BROADWAY,
	PPCEMU_CPU_MODEL_ESPRESSO
};

/* type for bus hooks */
typedef void (*ppcemu_bus_hook)(struct ppcemu_state *emu, uint32_t address, unsigned int len, void *data, bool is_write);

#endif /* _LIBPPCEMU_TYPES_H */
