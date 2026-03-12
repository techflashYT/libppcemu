/*
 * libppcemu - State handling
 *
 * Copyright (C) 2026 Techflash
 */


#include <unistd.h>
#ifdef _POSIX_TIMERS
#  ifdef _POSIX_MONOTONIC_CLOCK
#    include <time.h>
#    define USE_TIMESPEC
#  else
#    include <sys/time.h>
#  endif
#else
#  include <sys/time.h>
#endif

#include <ppcemu/state.h>
#include "state.h"
#include "spr.h"
/* FIXME: put into state */
enum ppcemu_loglevel virt2phys_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel ifetch_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel decode_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel branch_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel loadstore_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel cond_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel cache_loglevel = PPCEMU_LOGLEVEL_INFO;
enum ppcemu_loglevel misc_loglevel = PPCEMU_LOGLEVEL_INFO;

#define REAL_STATE struct _ppcemu_state *s = (struct _ppcemu_state *)state

uint32_t ppcemu_get_pc(struct ppcemu_state *state) {
	REAL_STATE;
	return s->pc;
}

void ppcemu_set_pc(struct ppcemu_state *state, uint32_t pc) {
	REAL_STATE;
	s->pc = pc;
}

uint32_t ppcemu_get_gpr(struct ppcemu_state *state, unsigned int idx) {
	REAL_STATE;
	if (idx > 31)
		return -1;

	return s->gpr[idx];
}

void ppcemu_set_gpr(struct ppcemu_state *state, unsigned int idx, uint32_t val) {
	REAL_STATE;
	if (idx > 31)
		return;

	s->gpr[idx] = val;
}

uint32_t ppcemu_get_spr(struct ppcemu_state *state, unsigned int sprn) {
	int idx = ppcemu_sprn_to_idx_raw(sprn);
	REAL_STATE;

	if (idx == -1)
		return 0xffffffff;

	return s->sprs[idx];
}

void ppcemu_set_spr(struct ppcemu_state *state, unsigned int sprn, uint32_t val) {
	int idx = ppcemu_sprn_to_idx_raw(sprn);
	REAL_STATE;

	if (idx == -1)
		return;

	s->sprs[idx] = val;
}

uint32_t ppcemu_get_cr(struct ppcemu_state *state) {
	REAL_STATE;
	return s->cr;
}

void ppcemu_set_cr(struct ppcemu_state *state, uint32_t val) {
	REAL_STATE;
	s->cr = val;
}

uint32_t ppcemu_get_msr(struct ppcemu_state *state) {
	REAL_STATE;
	return s->msr;
}

void ppcemu_set_msr(struct ppcemu_state *state, uint32_t val) {
	REAL_STATE;
	s->msr = val;
}

uint64_t ppcemu_get_tb(struct ppcemu_state *state) {
	REAL_STATE;
	return s->tb;
}

void ppcemu_set_loglevel(enum ppcemu_log_source source, enum ppcemu_loglevel level) {
	switch (source) {
	case PPCEMU_LOG_SOURCE_ADDR_TRANSLATION: {
		virt2phys_loglevel = level;
		break;
	}
	case PPCEMU_LOG_SOURCE_IFETCH: {
		ifetch_loglevel = level;
		break;
	}
	case PPCEMU_LOG_SOURCE_INSTR_DECODE: {
		decode_loglevel = level;
		break;
	}
	case PPCEMU_LOG_SOURCE_BRANCH: {
		branch_loglevel = level;
		break;
	}
	case PPCEMU_LOG_SOURCE_LOADSTORE: {
		loadstore_loglevel = level;
		break;
	}
	case PPCEMU_LOG_SOURCE_CACHE: {
		cache_loglevel = level;
		break;
	}
	case PPCEMU_LOG_SOURCE_MISC: {
		misc_loglevel = level;
		break;
	}
	default:
		break;
	}
}

uint64_t ppcemu_rt_refresh(struct ppcemu_state *state) {
	uint64_t diff_us, bus_hz, tb_cycles_per_usec;
	#ifdef USE_TIMESPEC
	struct timespec ts;
	#else
	struct timeval tv;
	#endif
	REAL_STATE;

	#ifdef USE_TIMESPEC
	clock_gettime(CLOCK_MONOTONIC, &ts);
	diff_us = (s->rt_last_sync_sec - ts.tv_sec) * 1000000;
	diff_us += (s->rt_last_sync_usec - (ts.tv_nsec / 1000));
	s->rt_last_sync_sec = ts.tv_sec;
	s->rt_last_sync_usec = ts.tv_nsec / 1000;
	#else
	gettimeofday(&tv, NULL);
	diff_us = (s->rt_last_sync_sec - tv.tv_sec) * 1000000;
	diff_us += (s->rt_last_sync_usec - tv.tv_usec);
	s->rt_last_sync_sec = tv.tv_sec;
	s->rt_last_sync_usec = tv.tv_usec;
	#endif

	/*
	 * Real nath
	 */

	/* get speed in Hz for easier calculations below */
	bus_hz = s->bus_speed_khz * 1000;
	/* TB ticks at 1/4th bus clock; further divide by 1000000 to get number per usec */
	tb_cycles_per_usec = (bus_hz / 4) / 1000000;

	/*
	 * number of TB cycles passed is the number
	 * of TB cycles per usec, multiplied by how many
	 * usecs have passed since the last check
	 */
	return tb_cycles_per_usec * diff_us;
}

void ppcemu_set_timing_mode(struct ppcemu_state *state, enum ppcemu_timing_mode mode) {
	REAL_STATE;

	switch (mode) {
	case PPCEMU_TIMING_MODE_RT: {
		s->sync_rt = true;
		break;
	}
	case PPCEMU_TIMING_MODE_SYNTH: {
		s->sync_rt = false;
		break;
	}
	default:
		return;
	}

	ppcemu_rt_refresh(state);
}
