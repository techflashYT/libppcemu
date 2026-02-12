/*
 * libppcemu - Internal Headers - Types
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_TYPES_H
#define _LIBPPCEMU_INTERNAL_TYPES_H

#include <ppcemu/types.h>
#include <stdbool.h>

/* define this if your host already defines u32, u16, u8, etc */
#ifndef PPCEMU_EXTERNAL_TYPES
	/* define this if your host doesn't have stdint.h */
	#ifndef PPCEMU_NO_STDINT_H
		#include <stdint.h>
		typedef uint32_t u32;
		typedef uint16_t u16;
		typedef uint8_t  u8;

		typedef int32_t  i32;
		typedef int16_t  i16;
		typedef int8_t   i8;

		typedef uintptr_t uptr;
		typedef unsigned int uint;
	#else
		#error "Don't know how to handle this"
	#endif /* PPCEMU_NO_STDINT_H */
#endif /* PPCEMU_EXTERNAL_TYPES */

#define HIDDEN __attribute__((visibility("hidden")))

#endif /* _LIBPPCEMU_INTERNAL_TYPES_H */
