/*
 * libppcemu - SPRs
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_SPR_H
#define _LIBPPCEMU_SPR_H

enum ppcemu_sprn {
	/*
	 * integer eXEception Register
	 *
	 * Available in: All
	 */
	PPCEMU_SPRN_XER = 1,

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

/*
 * XER bits
 */
#define PPCEMU_XER_SO    (1 << 31)
#define PPCEMU_XER_OV    (1 << 30)
#define PPCEMU_XER_CA    (1 << 29)
#define PPCEMU_XER_COUNT (127 << 0)

/*
 * HID4 bits
 */
#define PPCEMU_HID4_L2CFI          (1 << 20)
#define PPCEMU_HID4_L2MUM          (1 << 21)
#define PPCEMU_HID4_DBP            (1 << 22)
/* this is reserved on 750CL, "LPE" (?) on Broadway/Espresso */
#define PPCEMU_HID4_BW_LPE         (1 << 23)
/* this is Paired-singles control bit 1 on 750CL, "ST0" (STore 0?) on Broadway/Espresso */
#define PPCEMU_HID4_BW_ST0         (1 << 24)
#define PPCEMU_HID4_750CL_PS1_CTL  (1 << 24)
#define PPCEMU_HID4_SBE            (1 << 25)
/* this is L2 second castout buffer enable on 750CL, reserved on Broadway/Espresso */
#define PPCEMU_HID4_750CL_BCO      (1 << 26)
#define PPCEMU_HID4_BPD            (3 << 27)
#define PPCEMU_HID4_L2FM           (3 << 29)

/* reserved bits that must be 0 on Broadway/Espresso */
#define PPCEMU_HID4_BW_RSRVD0      (0xfffff | (1 << 26))
/* reserved bits that must be 0 on 750CL */
#define PPCEMU_HID4_750CL_RSRVD0   (0x7ffff | (1 << 23))
/* reserved bits that must be 1 on all implementations */
#define PPCEMU_HID4_RSRVD1         (1 << 31)


/*
 * BATU bits
 */
#define PPCEMU_BATU_VP_SHIFT   0
#define PPCEMU_BATU_VP         (1 << PPCEMU_BATU_VP_SHIFT)
#define PPCEMU_BATU_VS_SHIFT   1
#define PPCEMU_BATU_VS         (1 << PPCEMU_BATU_VS_SHIFT)
#define PPCEMU_BATU_BL_SHIFT   2
#define PPCEMU_BATU_BL         (2047 << PPCEMU_BATU_BL_SHIFT)
#define PPCEMU_BATU_BL_128KB   0x000
#define PPCEMU_BATU_BL_256KB   0x001
#define PPCEMU_BATU_BL_512KB   0x003
#define PPCEMU_BATU_BL_1MB     0x007
#define PPCEMU_BATU_BL_2MB     0x00F
#define PPCEMU_BATU_BL_4MB     0x01F
#define PPCEMU_BATU_BL_8MB     0x03F
#define PPCEMU_BATU_BL_16MB    0x07F
#define PPCEMU_BATU_BL_32MB    0x0FF
#define PPCEMU_BATU_BL_64MB    0x1FF
#define PPCEMU_BATU_BL_128MB   0x3FF
#define PPCEMU_BATU_BL_256MB   0x7FF
#define PPCEMU_BATU_BEPI_SHIFT 17
#define PPCEMU_BATU_BEPI       (32767 << PPCEMU_BATU_BEPI_SHIFT)

/*
 * BATL bits
 */
#define PPCEMU_BATL_PP_SHIFT   0
#define PPCEMU_BATL_PP         (3 << PPCEMU_BATL_PP_SHIFT)
#define PPCEMU_BATL_PP___      0 /* No access */
#define PPCEMU_BATL_PP_R_      1 /* Read-only */
#define PPCEMU_BATL_PP_RW      2 /* Read-write */
#define PPCEMU_BATL_WIMG_SHIFT 3
#define PPCEMU_BATL_WIMG       (15 << PPCEMU_BATL_WIMG_SHIFT)
#define PPCEMU_BATL_BPRN_SHIFT 17
#define PPCEMU_BATL_BPRN       (32767 << PPCEMU_BATL_BPRN_SHIFT)


#endif /* _LIBPPCEMU_SPR_H */
