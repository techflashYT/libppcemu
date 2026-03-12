/*
 * libppcemu - State handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_STATE_H
#define _LIBPPCEMU_STATE_H

#include <stdint.h>

/*
 * Opaque version of the ppcemu state struct.
 * Library users should not attempt to access this structure
 * directly.  Use the provided functions to safely modify and
 * inspect the state of this instance.
 */
struct ppcemu_state;

/*
 * Get the current PC (the address of the next instruction
 * that will run if you run ppcemu_{run,step}).
 */
uint32_t ppcemu_get_pc(struct ppcemu_state *state);

/*
 * Set the current PC (the address of the next instruction
 * that will run if you run ppcemu_{run,step}).
 */
void ppcemu_set_pc(struct ppcemu_state *state, uint32_t pc);

/*
 * Get the specified GPR
 */
uint32_t ppcemu_get_gpr(struct ppcemu_state *state, unsigned int idx);

/*
 * Set the specified GPR
 */
void ppcemu_set_gpr(struct ppcemu_state *state, unsigned int idx, uint32_t val);

/*
 * Get the specified SPR.
 * Returns 0xffffffff (-1) if the specified SPR is unimplemented/nonexistent.
 */
uint32_t ppcemu_get_spr(struct ppcemu_state *state, unsigned int sprn);

/*
 * Set the specified SPR.
 */
void ppcemu_set_spr(struct ppcemu_state *state, unsigned int sprn, uint32_t val);

/*
 * Get the CR
 */
uint32_t ppcemu_get_cr(struct ppcemu_state *state);

/*
 * Set the CR
 */
void ppcemu_set_cr(struct ppcemu_state *state, uint32_t val);

/*
 * Get the MSR
 */
uint32_t ppcemu_get_msr(struct ppcemu_state *state);

/*
 * Set the MSR
 */
void ppcemu_set_msr(struct ppcemu_state *state, uint32_t val);

/*
 * Get the Timebase
 */
uint64_t ppcemu_get_tb(struct ppcemu_state *state);

/*
 * Log levels
 */
enum ppcemu_loglevel {
	PPCEMU_LOGLEVEL_ERROR,
	PPCEMU_LOGLEVEL_WARN,
	PPCEMU_LOGLEVEL_INFO,
	PPCEMU_LOGLEVEL_VERBOSE,
	PPCEMU_LOGLEVEL_DEBUG
};

enum ppcemu_log_source {
	PPCEMU_LOG_SOURCE_ADDR_TRANSLATION,
	PPCEMU_LOG_SOURCE_IFETCH,
	PPCEMU_LOG_SOURCE_INSTR_DECODE,
	PPCEMU_LOG_SOURCE_BRANCH,
	PPCEMU_LOG_SOURCE_LOADSTORE,
	PPCEMU_LOG_SOURCE_CACHE,
	PPCEMU_LOG_SOURCE_MISC
};

/*
 * Set the desired loglevel for a given source
 */
void ppcemu_set_loglevel(enum ppcemu_log_source source, enum ppcemu_loglevel level);

/*
 * If in real-time mode, refresh the timer, and
 * return the number of TB ticks that would have passed.
 *
 * Beware that ppcemu_step also calls this function, every
 * 128th instruction.
 */
uint64_t ppcemu_rt_refresh(struct ppcemu_state *state);

enum ppcemu_timing_mode {
	PPCEMU_TIMING_MODE_SYNTH,
	PPCEMU_TIMING_MODE_RT
};

/*
 * Set the timing mode.
 */
void ppcemu_set_timing_mode(struct ppcemu_state *state, enum ppcemu_timing_mode mode);

#endif /* _LIBPPCEMU_STATE_H */
