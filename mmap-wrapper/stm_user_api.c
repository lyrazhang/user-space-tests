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

static void enable_sink(const char *dev_name)
{
	char buf[256] = {0};
	sprintf(buf, "echo 1 > /sys/bus/coresight/devices/%s/enable_sink",
		dev_name);
	system(buf);
}

static int set_policy(int fd, struct stp_policy_id *policy,
		      unsigned int chan, unsigned int width)
{
	unsigned int chan_perpage = PAGE_SIZE / BYTES_PER_CHANNEL;

	if(width % chan_perpage || chan % chan_perpage) {
		chan = chan / chan_perpage;
		width = (width / chan_perpage + 1) * BYTES_PER_CHANNEL;
	}

	policy->channel = chan;
	policy->__reserved_0 = 0;
	policy->__reserved_1 = 0;
	policy->width = width;
	policy->size = sizeof(struct stp_policy_id) + POLICY_NAME_LEN;
	memcpy(policy->id, STP_POLICY_NAME, POLICY_NAME_LEN);

	if (ioctl(fd, STP_POLICY_ID_SET, policy) == -1) {
		printf("STP_POLICY_ID_SET failed %s %d\n",
		       strerror(errno), errno);
		return -1;
	}

	return 0;
}

/*
 * dev	- storing the information of stimulus resources and STM device
 * chan	- the start index of channels
 * width- the number of channels for request
 */
int request_stm_resource(struct stm_dev *dev, unsigned int chan,
			 unsigned int width)
{
	int fd;
	int ret = 0;
	char *map;
	struct stp_policy_id *policy;
	unsigned int length = STM_MAP_SIZE;
	unsigned long offset = STM_MAP_OFFSET;

	if ((fd = open(STM_DEVICE_NAME, O_RDWR | O_SYNC)) == -1) {
		printf("Failed to open %s %s\n", STM_DEVICE_NAME,
			strerror(errno));
		return -1;
	}
	dev->fd = fd;

	/*
	 * Before allocating a policy for STM, the sink connected with STM must
	 * be enabled.
	 */
	enable_sink(TMC_SYS_NAME);

	/* set a master/channel policy for this STM device, this
	 * is because that kernel have to know how many channels
	 * would be mapped, and the size of mapped memory must be
	 * a multiple of page size.
	 */
	dev->policy = malloc(sizeof(struct stp_policy_id) + POLICY_NAME_LEN);
	if (!dev->policy) {
		ret = -1;
		printf("Failed to malloc policy.\n");
		goto out;
	}

	if (set_policy(fd, dev->policy, chan, width)) {
		ret = -1;
		printf("Failed to set policy.\n");
		goto out;
	}

	map = (char *)mmap(0, length, PROT_READ|PROT_WRITE,
			   MAP_SHARED, fd, offset);
	if (map == MAP_FAILED) {
		ret = -1;
		printf("Failed to map %s\n", strerror(errno));
		goto out;
	}

	dev->mmap.map = map;
	dev->mmap.start = 0;
	dev->mmap.length = length;
	printf("Success to map channel(%u~%u) to 0x%lx\n",
		dev->policy->channel,
		(dev->policy->width + dev->policy->channel - 1),
		(unsigned long)map);

	return ret;

out:
	release_stm_resource(dev);
	return ret;
}

void release_stm_resource(struct stm_dev *dev)
{
	/* unmap the area & error checking */
	if (dev->mmap.map) {
		if (munmap(dev->mmap.map, dev->mmap.length) == -1)
			perror("user: Error un-mmapping the file");
		dev->mmap.map = NULL;
	}
	if (dev->policy) {
		free(dev->policy);
		dev->policy = NULL;
	}
	if (dev->fd) {
		close(dev->fd);
		dev->fd = 0;
	}
}

static char *stm_channel_addr(struct stm_dev *dev, unsigned int chan,
			      unsigned int flags, unsigned int type)
{
	if (chan < dev->policy->channel ||
	    chan >= dev->policy->channel + dev->policy->width) {
		printf("Channel index should be in [%u...%u]\n",
		       dev->policy->channel,
		       dev->policy->channel + dev->policy->width);
		return NULL;
	}

	chan -= dev->policy->channel;

	return (char *)(((unsigned long)dev->mmap.map +
			 chan * BYTES_PER_CHANNEL) |
			((~flags) & type));
}

static unsigned int stm_dsize(const char *dev_name)
{
	FILE *file;
	unsigned int size = 0;
	char buf[128] = {0};
	char spfeat2r[16] = {0};
	int dsize = 0;

	sprintf(buf, "/sys/bus/coresight/devices/%s/mgmt/spfeat2r", dev_name);
	file = fopen(buf, "r");
	/* 
	 * the first two characters in file are '0x', like:
	 * # cat /sys/bus/coresight/devices/10006000.stm/mgmt/spfeat2r 
	 * 0x104f2
	 */
	fseek(file, 2, SEEK_END);
	size = ftell (file);
	fseek(file, 2, SEEK_SET);
	if (fgets(spfeat2r, size, file))
		dsize = strtol(spfeat2r, NULL, 16);
	fclose(file);

	return (dsize >> 12) & 0xf;
}

unsigned int stm_wrbytes(const char *dev_name)
{
	/* 
	 * Fundamental data size:
	 * 0b0001 - 64-bit data.
	 */
	return stm_dsize(dev_name) ? 8: 4;
}

static unsigned int stm_write(char *addr, void *data, unsigned int size)
{
	unsigned int wrbytes = stm_wrbytes(STM_SYS_NAME);
	if (size > wrbytes)
		size = wrbytes;

	memcpy(addr, (char *)data, size);
	return size;
}

int stm_trace_data(struct stm_dev *dev, unsigned int chan, int flags,
		   unsigned int size, void *data)
{
	int i = 0;
	char *addr = stm_channel_addr(dev, chan, flags, STM_PKT_TYPE_DATA);
	unsigned int real_wrbytes, len = size;
	char *pdata = data, nil = 0;

	if (!addr)
		return -1;

	do {
		real_wrbytes = stm_write(addr, pdata, len);
		pdata += real_wrbytes;
		len -= real_wrbytes;
		if (++i == 1)
			addr = stm_channel_addr(dev, chan, 0, STM_PKT_TYPE_DATA);
	} while(len);

	addr = stm_channel_addr(dev, chan, 0, STM_PKT_TYPE_FLAG);
	stm_write(addr, &nil, 1);

	return size;
}
