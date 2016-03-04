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

struct stp_policy_id {
	unsigned int	size;
	unsigned short	master;
	unsigned short	channel;
	unsigned short	width;
	/* padding */
	unsigned short	__reserved_0;
	unsigned int	__reserved_1;
	/* policy name */
	char		id[0];
};

#define BYTES_PER_CHANNEL	256
#define PAGE_SIZE		sysconf(_SC_PAGE_SIZE)
#define STM_MAP_OFFSET		0x0
#define STM_MAP_SIZE		PAGE_SIZE
#define STM_DEVICE_NAME		"/dev/10006000.stm"
#define TMC_DEVICE_NAME		"10003000.etf"
#define STP_POLICY_NAME		"test"
#define TEST_DATA_SIZE		4
#define POLICY_NAME_LEN		8

#define STP_POLICY_ID_SET	_IOWR('%', 0, struct stp_policy_id)

void enable_sink()
{
	char buf[256] = {0};
	sprintf(buf, "echo 1 > /sys/bus/coresight/devices/%s/enable_sink", TMC_DEVICE_NAME);
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

void main()
{
	int fd;
	char *map;
	unsigned int length = STM_MAP_SIZE;
	unsigned long offset = STM_MAP_OFFSET;
	struct stp_policy_id *policy;
	unsigned int trace_data[TEST_DATA_SIZE] = {0x55555555, 0xaaaaaaaa, 0x66666666, 0x99999999};

	fd = open(STM_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s %s\n", STM_DEVICE_NAME,
			strerror(errno));
		return;
	}

	/*
	 * Before allocating a policy for STM, the sink connected with STM must
	 * be enabled.
	 */

	enable_sink();

	/* set a master/channel policy for this STM devicem, this
	 * is because that kernel have to know how many channels
	 * would be mapped, and the size of mapped memory must be
	 * a multiple of page size.
	 */
	policy = malloc(sizeof(struct stp_policy_id) + POLICY_NAME_LEN);
	if (!policy) {
		close(fd);
		return;
	}

	if (set_policy(fd, policy))
		goto out;

	map = (char *)mmap(0, length, PROT_READ|PROT_WRITE,
			   MAP_PRIVATE, fd, offset);
	if (map == MAP_FAILED) {
		printf("Failed to map %s\n", strerror(errno));
		goto out;
	}
	printf("Success to map channel(%u~%u) to 0x%lx\n",
		policy->channel, (policy->width + policy->channel - 1),
		(unsigned long)map);

	/* read from map space */
	printf("map = %u\n", map[0]);
	/* write data to map space */
	memcpy(map, (char*)trace_data, sizeof(unsigned int) * TEST_DATA_SIZE);

	/* unmap the area & error checking */
	if (munmap(map, length) == -1)
		perror("user: Error un-mmapping the file");

out:
	free(policy);
	close(fd);
}

