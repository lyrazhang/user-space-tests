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
	unsigned int trace_data[TEST_DATA_SIZE] = {0x55555555, 0xaaaaaaaa, 0x66666666, 0x99999999};

	if (request_stm_resource(&g_stm_dev))
		return;

	/* write data to map space */
	memcpy(g_stm_dev.mmap.map, (char*)trace_data, sizeof(unsigned int) * TEST_DATA_SIZE);

out:
	release_stm_reaource(&g_stm_dev);
}
