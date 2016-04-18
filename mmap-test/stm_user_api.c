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

void enable_sink(const char *dev_name)
{
	char buf[256] = {0};
	sprintf(buf, "echo 1 > /sys/bus/coresight/devices/%s/enable_sink", dev_name);
	system(buf);
}

void disable_sink(const char *dev_name)
{
	char buf[256] = {0};
	sprintf(buf, "echo 0 > /sys/bus/coresight/devices/%s/enable_sink", dev_name);
	system(buf);
}

void disable_source(const char *dev_name)
{
	char buf[256] = {0};
	sprintf(buf, "echo 0 > /sys/bus/coresight/devices/%s/enable_source", dev_name);
	system(buf);
}

int set_policy(int fd, struct stp_policy_id *policy)
{
	policy->channel = 0;
	policy->__reserved_0 = 0;
	policy->__reserved_1 = 0;
	policy->width = PAGE_SIZE / BYTES_PER_CHANNEL;
	policy->size = sizeof(struct stp_policy_id) + POLICY_NAME_LEN;
	memcpy(policy->id, STP_POLICY_NAME, POLICY_NAME_LEN);

	if (ioctl(fd, STP_POLICY_ID_SET, policy) == -1) {
		printf("STP_POLICY_ID_SET failed %s %d\n",
		       strerror(errno), errno);
		return -1;
	}

	return 0;
}

int request_stm_resource(struct stm_dev *dev)
{
	int fd;
	char *map;
	struct stp_policy_id *policy;
	unsigned int length = STM_MAP_SIZE;
	unsigned long offset = STM_MAP_OFFSET;

	fd = open(STM_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s %s\n", STM_DEVICE_NAME,
			strerror(errno));
		return -1;
	}
	dev->fd = fd;

	/*
	 * Before allocating a policy for STM, the sink connected with STM must
	 * be enabled.
	 */
	enable_sink(TMC_DEVICE_NAME);

	/* set a master/channel policy for this STM device, this
	 * is because that kernel have to know how many channels
	 * would be mapped, and the size of mapped memory must be
	 * a multiple of page size.
	 */
	dev->policy = malloc(sizeof(struct stp_policy_id) + POLICY_NAME_LEN);
	if (!dev->policy) 
		goto out;
	if (set_policy(fd, dev->policy))
		goto out;

	map = (char *)mmap(0, length, PROT_READ|PROT_WRITE,
			   MAP_PRIVATE, fd, offset);
	if (map == MAP_FAILED) {
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
out:
	release_stm_reaource(dev);
	return -1;
}

void release_stm_reaource(struct stm_dev *dev)
{
	disable_source(STM_DEVICE_NAME);
	disable_sink(TMC_DEVICE_NAME);

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


