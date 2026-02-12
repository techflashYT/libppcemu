/*
 * libppcemu - Initialization
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INIT_H
#define _LIBPPCEMU_INIT_H

#include <ppcemu/state.h>
#include <ppcemu/types.h>

/*
 * Initializes a ppcemu instance.
 * Can be called multiple times to initialize multiple instances.
 *
 * Returns:
 * - pointer to a 'struct ppcemu_state' (on success)
 * - NULL (on failure)
 *
 * Parameters:
 * - 'model': The CPU model to emulate
 * - 'bus_hook': Function to call when a memory access would go to the bus
 * - 'bus_speed_khz': Bus frequency of the processor in KHz (e.g. 243000)
 * - 'c2b_mult': Core-to-bus multiplier of the processor (e.g. 3)
 */
extern struct ppcemu_state *ppcemu_init(enum ppcemu_cpu_model model, ppcemu_bus_hook bus_hook, unsigned int bus_speed_khz, unsigned int c2b_mult);

/*
 * Destroys a ppcemu instance.
 *
 * Parameters:
 * - 'emu': The ppcemu instance to destroy
 */
extern void ppcemu_destory(struct ppcemu_state *emu);

#endif /* _LIBPPCEMU_INIT_H */
