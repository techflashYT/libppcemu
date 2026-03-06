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

/*
 * comment out or uncomment lines in here to disable/enable the relevant feature
 * DEBUG_[foo] still needs to be manually enabled
 */

/* enable address translation debugging */
#define DEBUG_VIRT2PHYS

/* enable instruction fetch debugging */
#define DEBUG_IFETCH

/* enable instruction decode debugging */
#define DEBUG_INSTR_DECODE

/* enable branch debugging */
#define DEBUG_BRANCH

/* enable load/store debugging */
#define DEBUG_LOADSTORE

/* enable conditional debugging */
#define DEBUG_COND

/* comment this out in combination with disabling the above to disable all logging, even genuine warnings and errors */
#define ALLOW_WARN_ERROR

#endif /* _LIBPPC_CONFIG_H */
