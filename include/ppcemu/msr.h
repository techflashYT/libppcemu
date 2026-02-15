/*
 * libppcemu - MSR
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_MSR_H
#define _LIBPPCEMU_MSR_H

#define PPCEMU_MSR_LE     (1 << 0)
#define PPCEMU_MSR_RI     (1 << 1)
#define PPCEMU_MSR_PM     (1 << 2)
#define PPCEMU_MSR_RSRVD1 (1 << 3)
#define PPCEMU_MSR_DR     (1 << 4)
#define PPCEMU_MSR_IR     (1 << 5)
#define PPCEMU_MSR_IP     (1 << 6)
#define PPCEMU_MSR_RSRVD2 (1 << 7)
#define PPCEMU_MSR_FE1    (1 << 8)
#define PPCEMU_MSR_BE     (1 << 9)
#define PPCEMU_MSR_SE     (1 << 10)
#define PPCEMU_MSR_FE0    (1 << 11)
#define PPCEMU_MSR_ME     (1 << 12)
#define PPCEMU_MSR_FP     (1 << 13)
#define PPCEMU_MSR_PR     (1 << 14)
#define PPCEMU_MSR_EE     (1 << 15)
#define PPCEMU_MSR_ILE    (1 << 16)
#define PPCEMU_MSR_RSRVD3 (1 << 17)
#define PPCEMU_MSR_POW    (1 << 18)
#define PPCEMU_MSR_RSRVD4 (7 << 19)
#define PPCEMU_MSR_RSRVD5 (15 << 22)
#define PPCEMU_MSR_RSRVD6 (15 << 27)
#define PPCEMU_MSR_RSRVD7 (1 << 31)
#define PPCEMU_MSR_RSRVD_FF (PPCEMU_MSR_RSRVD7 | PPCEMU_MSR_RSRVD5 | PPCEMU_RSRVD3 | PPCEMU_RSRVD2 | PPCEMU_RSRVD1)
#define PPCEMU_MSR_RSRVD_PF (PPCEMU_MSR_RSRVD6 | PPCEMU_MSR_RSRVD4)

#endif /* _LIBPPCEMU_MSR_H */
