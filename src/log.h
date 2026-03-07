/*
 * libppcemu - Internal Headers - Logging
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_LOG_H
#define _LIBPPCEMU_INTERNAL_LOG_H

#include <stdarg.h>
#include <ppcemu/state.h>
#include <stdio.h>
#include "types.h"
#include "../config.h"

extern enum ppcemu_loglevel virt2phys_loglevel;
extern enum ppcemu_loglevel ifetch_loglevel;
extern enum ppcemu_loglevel decode_loglevel;
extern enum ppcemu_loglevel branch_loglevel;
extern enum ppcemu_loglevel loadstore_loglevel;
extern enum ppcemu_loglevel cond_loglevel;
extern enum ppcemu_loglevel misc_loglevel;


#ifndef LOG_LEVEL
#error "Please define LOG_LEVEL"
#endif

#define DO_LOG(level) \
	va_list va; \
	\
	if (LOG_LEVEL <= level) \
		return; \
	\
	va_start(va, fmt); \
	vprintf(fmt, va); \
	va_end(va);

static inline void debug(const char *fmt, ...) {
	DO_LOG(PPCEMU_LOGLEVEL_DEBUG);
}

static inline void verbose(const char *fmt, ...) {
	DO_LOG(PPCEMU_LOGLEVEL_VERBOSE);
}

static inline void info(const char *fmt, ...) {
	DO_LOG(PPCEMU_LOGLEVEL_INFO);
}

static inline void warn(const char *fmt, ...) {
	DO_LOG(PPCEMU_LOGLEVEL_WARN);
}

static inline void error(const char *fmt, ...) {
	DO_LOG(PPCEMU_LOGLEVEL_ERROR);
}

#endif /* _LIBPPCEMU_INTERNAL_LOG_H */
