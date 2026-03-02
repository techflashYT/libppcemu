/*
 * libppcemu - Configuration
 *
 * Copyright (C) 2026 Techflash
 *
 *
 * Modify this file to change the configuration of libppcemu.
 */

#ifndef _LIBPPCEMU_CONFIG_H
#define _LIBPPCEMU_CONFIG_H

/* comment out or uncomment lines in here to disable/enable the relevant feature */

/* enable address translation debugging */
#define DEBUG_VIRT2PHYS

/* enable instruction fetch debugging */
#define DEBUG_IFETCH

/* enable instruction decode debugging */
#define DEBUG_INSTR_DECODE

/* enable branch debugging */
#define DEBUG_BRANCH

/* enable register dumps before and after an instruction executes */
#define DEBUG_REGDUMP

/* enable load/store debugging */
#define DEBUG_LOADSTORE

/* comment this out in combination with disabling the above to disable all logging, even genuine warnings and errors */
#define ALLOW_WARN_ERROR


/* don't touch the below unless you know what you're doing */
#ifdef ALLOW_WARN_ERROR
#define warn printf
#define error printf
#else
static inline void warn(const char *fmt, ...) { (void)fmt; }
static inline void error(const char *fmt, ...) { (void)fmt; }
#endif

#endif /* _LIBPPC_CONFIG_H */
