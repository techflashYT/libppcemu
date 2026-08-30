/*
 * libppcemu - Internal Headers - Floating Point
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_FP_H
#define _LIBPPCEMU_INTERNAL_FP_H


/* float32 mantissa is 23 bits wide; its MSB (bit 22) is the QNaN/SNaN marker */
#define FRAC_MSB 0x00400000u
#define IS_NAN(x) (((x >> 23) & 0xffu) == 0xffu && (x & 0x7fffffu) != 0u)
#define IS_SNAN(x) (IS_NAN(x) && (x & FRAC_MSB) == 0u)
#define IS_QNAN(x) (IS_NAN(x) && (x & FRAC_MSB) == FRAC_MSB)

/* FPSCR bits */
#define FPSCR_ZX     (1u << (31 - 5))
#define FPSCR_VXSNAN (1u << (31 - 7))
#define FPSCR_VXVC   (1u << (31 - 12))
#define FPSCR_VE     (1u << (31 - 24))

#endif /* _LIBPPCEMU_INTERNAL_FP_H */
