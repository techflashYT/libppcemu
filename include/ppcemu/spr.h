/*
 * libppcemu - SPRs
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_SPR_H
#define _LIBPPCEMU_SPR_H

enum ppcemu_sprn {
	/*
	 * Link Register
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_LR = 8,

	/*
	 * Count Register
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_CTR = 9,

	/*
	 * Decrementer
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_DEC = 22,

	/*
	 * Machine Status Save/Restore Register 0/1
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_SRR0 = 26,
	PPCEMU_SPRN_SRR1 = 27,

	/*
	 * Timebase usermode registers
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_TBL_VEA = 268,
	PPCEMU_SPRN_TBU_VEA = 269,

	/*
	 * General purpose SPRs
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_SPRG0 = 272,
	PPCEMU_SPRN_SPRG1 = 273,
	PPCEMU_SPRN_SPRG2 = 274,
	PPCEMU_SPRN_SPRG3 = 275,

	/*
	 * Timebase supervisor mode registers
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_TBL_OEA = 284,
	PPCEMU_SPRN_TBU_OEA = 285,

	/*
	 * Processor Version Register
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_PVR = 287,

	/*
	 * Hardware Implementation Dependent Register 2 - Gekko
	 *
	 * Available in: Gekko, 750CL, Broadway, Espresso
	 */
	PPCEMU_SPRN_HID2_GEKKO = 920,

	/*
	 * Hardware Implementation Dependent Register 4
	 *
	 * Available in: 750CL, Broadway, Espresso
	 */
	PPCEMU_SPRN_HID4 = 1011
};

#endif /* _LIBPPCEMU_SPR_H */
