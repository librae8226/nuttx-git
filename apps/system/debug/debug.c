/****************************************************************************
 * apps/system/debug/debug.c
 *
 *   Copyright (C) 2013 Librae. All rights reserved.
 *   Modified by: Librae <librae8226@gmail.com>
 *
 *   Copyright (C) 2011 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/lcd/lcd.h>
#include <nuttx/arch.h>

/****************************************************************************
 * Private Data
 ****************************************************************************/
pthread_t custom_thread;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
void up_ledon(int led);
void up_ledoff(int led);

FAR void *custom_thread_func(FAR void *arg)
{
	up_ledon(0);
	printf("custom thread go forth!\n");
	for(;;) {
		sleep(5);
		up_ledon(0);
		usleep(20000);
		up_ledoff(0);
	}
	return NULL;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int debug_main(int argc, char **argv)
{
	pthread_attr_t attr_custom;
	int ret = 0;
	struct lcd_dev_s *dev;

	up_ledoff(0);
	printf("Hello system debug! @%s\n", argv[0]);

#if 1
	/* start a custom thread */
	ret = pthread_attr_init(&attr_custom);
	if (ret != OK)
		goto out;

	ret = pthread_create(&custom_thread, &attr_custom,
			     custom_thread_func, (pthread_addr_t)0);
	if (ret != 0)
		goto out;
#else
	/* Initialize the LCD device */

	printf("debug: Initializing LCD\n");
	ret = up_lcdinitialize();
	if (ret < 0)
	{
		printf("debug: up_lcdinitialize failed: %d\n", -ret);
		goto out;
	}

	/* Get the device instance */

	dev = up_lcdgetdev(CONFIG_EXAMPLES_NX_DEVNO);
	if (!dev)
	{
		printf("debug: up_lcdgetdev failed, devno=%d\n", CONFIG_EXAMPLES_NX_DEVNO);
		goto out;
	}

	/* Turn the LCD on at 75% power */

	(void)dev->setpower(dev, ((3*CONFIG_LCD_MAXPOWER + 3)/4));
#endif

	return OK;
out:
	return ERROR;
}
