/*
 * libppcemu - SPR index table
 *
 * Copyright (C) 2026 Techflash
 */

#include "spr.h"

i8 ppcemu_sprn_to_idx_table[PPCEMU_SPRN_TABLE_SIZE];

static void __attribute__((constructor, used)) init_sprn_to_idx_table(void) {
	int sprn;

	for (sprn = 0; sprn < PPCEMU_SPRN_TABLE_SIZE; sprn++)
		ppcemu_sprn_to_idx_table[sprn] = ppcemu_sprn_to_idx_switch(sprn);
}
