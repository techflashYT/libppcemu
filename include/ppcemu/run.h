/*
 * libppcemu - Running
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_RUN_H
#define _LIBPPCEMU_RUN_H

#include <ppcemu/state.h>

/*
 * Run one instruction of emulation.
 *
 * Will trigger any relevant hooks as needed.
 *
 * Parameters:
 * - 'emu': ppcemu instance to step
 */
extern void ppcemu_step(struct ppcemu_state *emu);

#endif /* _LIBPPCEMU_RUN_H */
