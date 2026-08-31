/*
 * libppcemu - State handling
 *
 * Copyright (C) 2026 Techflash
 */


#include "ppcemu/types.h"
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

static void ppcemu_rt_reseed(struct ppcemu_state *state) {
	#ifdef USE_TIMESPEC
	struct timespec ts;
	#else
	struct timeval tv;
	#endif
	REAL_STATE;

	#ifdef USE_TIMESPEC
	clock_gettime(CLOCK_MONOTONIC, &ts);
	s->rt_epoch_sec = ts.tv_sec;
	s->rt_epoch_usec = ts.tv_nsec / 1000;
	#else
	gettimeofday(&tv, NULL);
	s->rt_epoch_sec = tv.tv_sec;
	s->rt_epoch_usec = tv.tv_usec;
	#endif
	s->rt_epoch_tb = s->tb;
}

void ppcemu_rt_throttle(struct ppcemu_state *state) {
	uint64_t emulated_us, real_us, tb_hz, sleep_us;
	#ifdef USE_TIMESPEC
	struct timespec ts;
	#else
	struct timeval tv;
	#endif
	REAL_STATE;

	tb_hz = ((uint64_t)s->bus_speed_khz * 1000) / 4;
	if (tb_hz == 0)
		return;

	emulated_us = ((s->tb - s->rt_epoch_tb) * 1000000) / tb_hz;

	#ifdef USE_TIMESPEC
	clock_gettime(CLOCK_MONOTONIC, &ts);
	real_us = (uint64_t)(ts.tv_sec - s->rt_epoch_sec) * 1000000;
	real_us += (ts.tv_nsec / 1000) - s->rt_epoch_usec;
	#else
	gettimeofday(&tv, NULL);
	real_us = (uint64_t)(tv.tv_sec - s->rt_epoch_sec) * 1000000;
	real_us += tv.tv_usec - s->rt_epoch_usec;
	#endif

	if (emulated_us > real_us) {
		sleep_us = emulated_us - real_us;
		if (sleep_us > 100000) {
			ppcemu_rt_reseed(state);
			return;
		}
		usleep(sleep_us);
	}
	else if (real_us > emulated_us + 250000) {
		/* too far behind */
		ppcemu_rt_reseed(state);
	}
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

	ppcemu_rt_reseed(state);
}

void ppcemu_set_loadstore_hook(struct ppcemu_state *state, ppcemu_loadstore_hook hook) {
	REAL_STATE;

	s->loadstore_hook = hook;
}

void ppcemu_external_interrupt(struct ppcemu_state *state, bool asserted) {
	REAL_STATE;

	atomic_store_explicit(&s->external_interrupt_pending, asserted, memory_order_relaxed);
}

void ppcemu_set_cache_mode(struct ppcemu_state *state, enum ppcemu_cache_mode mode) {
	REAL_STATE;

	s->cache_mode = mode;
}
