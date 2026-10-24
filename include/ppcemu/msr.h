/*
 * libppcemu - MSR
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_MSR_H
#define _LIBPPCEMU_MSR_H

#define PPCEMU_MSR_LE     (1u << 0)
#define PPCEMU_MSR_RI     (1u << 1)
#define PPCEMU_MSR_PM     (1u << 2)
#define PPCEMU_MSR_RSRVD1 (1u << 3)
#define PPCEMU_MSR_DR     (1u << 4)
#define PPCEMU_MSR_IR     (1u << 5)
#define PPCEMU_MSR_IP     (1u << 6)
#define PPCEMU_MSR_RSRVD2 (1u << 7)
#define PPCEMU_MSR_FE1    (1u << 8)
#define PPCEMU_MSR_BE     (1u << 9)
#define PPCEMU_MSR_SE     (1u << 10)
#define PPCEMU_MSR_FE0    (1u << 11)
#define PPCEMU_MSR_ME     (1u << 12)
#define PPCEMU_MSR_FP     (1u << 13)
#define PPCEMU_MSR_PR     (1u << 14)
#define PPCEMU_MSR_EE     (1u << 15)
#define PPCEMU_MSR_ILE    (1u << 16)
#define PPCEMU_MSR_RSRVD3 (1u << 17)
#define PPCEMU_MSR_POW    (1u << 18)
#define PPCEMU_MSR_RSRVD4 (7 << 19)
#define PPCEMU_MSR_RSRVD5 (15 << 22)
#define PPCEMU_MSR_RSRVD6 (15 << 27)
#define PPCEMU_MSR_RSRVD7 (1u << 31)
#define PPCEMU_MSR_RSRVD_FF (PPCEMU_MSR_RSRVD7 | PPCEMU_MSR_RSRVD5 | PPCEMU_RSRVD3 | PPCEMU_RSRVD2 | PPCEMU_RSRVD1)
#define PPCEMU_MSR_RSRVD_PF (PPCEMU_MSR_RSRVD6 | PPCEMU_MSR_RSRVD4)

#endif /* _LIBPPCEMU_MSR_H */
