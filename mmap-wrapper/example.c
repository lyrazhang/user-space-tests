#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include "stm_user_api.h"

extern struct stm_dev g_stm_dev;

void main()
{
	int i;
	unsigned int chan_start = 32768;
	unsigned width = PAGE_SIZE / BYTES_PER_CHANNEL;
	unsigned int flags = STM_FLAG_TIMESTAMPED;
	unsigned int dsize;
	char *offset, *data;
	unsigned int wrbytes = sizeof(unsigned int) * TEST_DATA_SIZE;
	unsigned int real_wrbytes;
	unsigned int trace_data[TEST_DATA_SIZE] = {0x5555aaaa, 0xaaaa5555, 0x66666666, 0x99999999};

	if (request_stm_resource(&g_stm_dev, chan_start, width))
		return;

	/* 
	 * You can use any channel between [g_stm_dev.policy->channel ...
	 * (g_stm_dev.policy->channel + g_stm_dev.policy->width)]
	 * and width must <= (PAGE_SIZE / BYTES_PER_CHANNEL) 
         * http://lxr.free-electrons.com/source/drivers/hwtracing/stm/core.c?v=4.6#L542
	 */
	real_wrbytes = stm_trace_data(&g_stm_dev, chan_start, flags,
				      wrbytes, trace_data);
	if (real_wrbytes != wrbytes)
		printf("write %d bytes and left % bytes data\n",
		       real_wrbytes, wrbytes - real_wrbytes);

	printf("Success to write %d bytes\n", real_wrbytes);

	release_stm_resource(&g_stm_dev);
}
