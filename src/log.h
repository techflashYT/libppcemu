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
extern enum ppcemu_loglevel cache_loglevel;
extern enum ppcemu_loglevel misc_loglevel;


#ifndef LOG_LEVEL
#error "Please define LOG_LEVEL"
#endif

#ifdef ALLOW_WARN_ERROR
static void __attribute__((noinline, cold, unused)) log_emit(const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}

#define DO_LOG(level, ...) \
	do { \
		if (unlikely(LOG_LEVEL >= (level))) \
			log_emit(__VA_ARGS__); \
	} while (0)
#else
#define DO_LOG(level, ...) ((void)0)
#endif

#define debug(...)   DO_LOG(PPCEMU_LOGLEVEL_DEBUG, __VA_ARGS__)
#define verbose(...) DO_LOG(PPCEMU_LOGLEVEL_VERBOSE, __VA_ARGS__)
#define info(...)    DO_LOG(PPCEMU_LOGLEVEL_INFO, __VA_ARGS__)
#define warn(...)    DO_LOG(PPCEMU_LOGLEVEL_WARN, __VA_ARGS__)
#define error(...)   DO_LOG(PPCEMU_LOGLEVEL_ERROR, __VA_ARGS__)

#endif /* _LIBPPCEMU_INTERNAL_LOG_H */
