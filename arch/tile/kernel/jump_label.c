/*
 * Copyright 2015 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 *
 * jump label TILE-Gx support
 */

#include <linux/jump_label.h>
#include <linux/memory.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/cpu.h>

#include <asm/cacheflush.h>
#include <asm/insn.h>

static void __jump_label_transform(struct jump_entry *e,
				   enum jump_label_type type)
{
	tilegx_bundle_bits opcode;
	/* Operate on writable kernel text mapping. */
	unsigned long pc_wr = ktext_writable_addr(jump_entry_code(e));

	if (type == JUMP_LABEL_JMP)
		opcode = tilegx_gen_branch(jump_entry_code(e),
					   jump_entry_target(e), false);
	else
		opcode = NOP();

	*(tilegx_bundle_bits *)pc_wr = opcode;
	/* Make sure that above mem writes were issued towards the memory. */
	smp_wmb();
}

void arch_jump_label_transform(struct jump_entry *e,
				enum jump_label_type type)
{
	mutex_lock(&text_mutex);

	__jump_label_transform(e, type);
	flush_icache_range(jump_entry_code(e),
			   jump_entry_code(e) + sizeof(tilegx_bundle_bits));

	mutex_unlock(&text_mutex);
}
