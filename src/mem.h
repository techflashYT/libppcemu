/*
 * libppcemu - Internal Headers - Memory handling
 *
 * Copyright (C) 2026 Techflash
 */

#ifndef _LIBPPCEMU_INTERNAL_MEM_H
#define _LIBPPCEMU_INTERNAL_MEM_H

#include "exception.h"
#include "types.h"
#include "state.h"
#include "../config.h"

enum virt2phys_err {
	V2P_SUCCESS,
	V2P_NOT_MAPPED,
	V2P_NO_PERMS
};

/* converts a virtual address to a physical one, returning various values if it failed */
enum virt2phys_err ppcemu_virt2phys(struct _ppcemu_state *state, u32 virt, u32 *phys, bool ifetch, bool write);
const char *v2p_strerror(enum virt2phys_err err);

static inline enum virt2phys_err _do_basic_store(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;

	err = ppcemu_virt2phys(state, ea, &phys, false, true);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		warn("_do_basic_store: virt2phys error: %s (%d)\r\n", v2p_strerror(err), err);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	state->bus_hook((struct ppcemu_state *)state, phys, len, val, true);
	return err;
}

static inline enum virt2phys_err _do_basic_load(struct _ppcemu_state *state, uint len, u32 ea, void *val) {
	enum virt2phys_err err;
	u32 phys;

	err = ppcemu_virt2phys(state, ea, &phys, false, false);
	if (err != V2P_SUCCESS) {
		/* TODO: need to set other info? */
		warn("_do_basic_load: virt2phys error: %s (%d)\r\n", v2p_strerror(err), err);
		exception_fire(state, EXCEPTION_DSI);
		return err;
	}

	state->bus_hook((struct ppcemu_state *)state, phys, len, val, false);
	return err;
}



#endif /* _LIBPPCEMU_INTERNAL_MEM_H */
