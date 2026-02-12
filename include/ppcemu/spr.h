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
	 * Lower BATs
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_IBAT0U = 528,
	PPCEMU_SPRN_IBAT0L = 529,
	PPCEMU_SPRN_IBAT1U = 530,
	PPCEMU_SPRN_IBAT1L = 531,
	PPCEMU_SPRN_IBAT2U = 532,
	PPCEMU_SPRN_IBAT2L = 533,
	PPCEMU_SPRN_IBAT3U = 534,
	PPCEMU_SPRN_IBAT3L = 535,
	PPCEMU_SPRN_DBAT0U = 536,
	PPCEMU_SPRN_DBAT0L = 537,
	PPCEMU_SPRN_DBAT1U = 538,
	PPCEMU_SPRN_DBAT1L = 539,
	PPCEMU_SPRN_DBAT2U = 540,
	PPCEMU_SPRN_DBAT2L = 541,
	PPCEMU_SPRN_DBAT3U = 542,
	PPCEMU_SPRN_DBAT3L = 543,

	/*
	 * Upper BATs
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_IBAT4U = 560,
	PPCEMU_SPRN_IBAT4L = 561,
	PPCEMU_SPRN_IBAT5U = 562,
	PPCEMU_SPRN_IBAT5L = 563,
	PPCEMU_SPRN_IBAT6U = 564,
	PPCEMU_SPRN_IBAT6L = 565,
	PPCEMU_SPRN_IBAT7U = 566,
	PPCEMU_SPRN_IBAT7L = 567,
	PPCEMU_SPRN_DBAT4U = 568,
	PPCEMU_SPRN_DBAT4L = 569,
	PPCEMU_SPRN_DBAT5U = 570,
	PPCEMU_SPRN_DBAT5L = 571,
	PPCEMU_SPRN_DBAT6U = 572,
	PPCEMU_SPRN_DBAT6L = 573,
	PPCEMU_SPRN_DBAT7U = 574,
	PPCEMU_SPRN_DBAT7L = 575,

	/*
	 * Hardware Implementation Dependent Register 2 - Gekko
	 *
	 * Available in: Gekko, 750CL, Broadway, Espresso
	 */
	PPCEMU_SPRN_HID2_GEKKO = 920,

	/*
	 * Hardware Implementation Dependent Register 0
	 *
	 * Available in: 750CXe, Gekko, 750CL, Broadway, Espresso
	 */
	PPCEMU_SPRN_HID0 = 1008,

	/*
	 * Hardware Implementation Dependent Register 4
	 *
	 * Available in: 750CL, Broadway, Espresso
	 */
	PPCEMU_SPRN_HID4 = 1011
};

#endif /* _LIBPPCEMU_SPR_H */
