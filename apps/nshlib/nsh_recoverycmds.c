/****************************************************************************
 * apps/nshlib/dbg_dbgcmds.c
 *
 *   Copyright (C) 2008-2009, 2011-2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *           Librae <librae@linkgo.io>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <nuttx/arch.h>
#include <arch/irq.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "nsh.h"
#include "nsh_console.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: cmd_boot, boot from specific address in recovery mode
 ****************************************************************************/

#ifdef CONFIG_NSH_RECOVERY_BOOT

/*
 * FIXME
 * The check_code() and jump_to() should be arch specific.
 */

static int check_code(uint32_t addr) {
	uint32_t sp = *(volatile uint32_t *) addr;

	if ((sp & 0x2FFE0000) == 0x20000000) {
		return 1;
	} else {
		return 0;
	}
}

static void __msr_msp(uint32_t p)
{
	__asm volatile("msr msp, r0\n" "bx r14\n");
}

static void jump_to(uint32_t addr) {
	typedef void (*funcptr)(void);

	uint32_t jump_addr = *(volatile uint32_t *)(addr + 0x04);
	funcptr entry = (funcptr)jump_addr;
	irqdisable();
	__msr_msp(*(volatile uint32_t *)addr);
	entry();
}

int cmd_boot(FAR struct nsh_vtbl_s *vtbl, int argc, char **argv)
{
	FAR volatile uintptr_t addr;
	FAR char *endptr;

	addr = (uintptr_t)strtol(argv[1], &endptr, 16);
	if (argv[0][0] == '\0' || *endptr != '\0')
	{
		return ERROR;
	}

	if (check_code(addr)) {
		nsh_output(vtbl, "%p: 0x%08x, code valid, jump in 1 sec...\n",
				addr, *(volatile uintptr_t *)addr);
		sleep(1);
		jump_to(addr);
	} else {
		nsh_output(vtbl, "%p: 0x%08x, code invalid, hard reset...\n",
				addr, *(volatile uintptr_t *)addr);
		up_systemreset();
	}

	return OK;
}
#endif
