/*
 * libppcemu - Bus snooping
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_SNOOP_H
#define _LIBPPCEMU_SNOOP_H

#include <stdint.h>
#include <ppcemu/state.h>

enum ppcemu_snoop_op {
	/*
	 * Another bus master is reading the range.  A modified line is pushed to the
	 * bus then invalidated.  A clean line is simply invalidated.
	 */
	PPCEMU_SNOOP_READ,

	/*
	 * Another bus master is writing the range. A modified line is pushed to the
	 * bus first and then invalidated.
	 */
	PPCEMU_SNOOP_WRITE,

	/*
	 * Another bus master is overwriting the entire range.  Matching lines are
	 * invalidated and any modified data is discarded.
	 */
	PPCEMU_SNOOP_KILL
};

/*
 * Report an external bus transaction to the CPU.
 *
 * May invoke the bus hook (to push modified lines) before returning, so call it
 * from the same context you would call ppcemu_step() from.
 *
 * Parameters:
 * - 'emu': ppcemu instance
 * - 'op': kind of transaction
 * - 'phys': physical start address of the transaction
 * - 'size': length in bytes; every cache line overlapping [phys, phys + size) is snooped.
 *
 * Returns the number of modified lines that were pushed to the bus.
 */
extern unsigned int ppcemu_snoop(struct ppcemu_state *emu, enum ppcemu_snoop_op op, uint32_t phys, uint32_t size);

#endif /* _LIBPPCEMU_SNOOP_H */
