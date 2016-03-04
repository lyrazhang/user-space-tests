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
	char		id[0];
};

#define BYTES_PER_CHANNEL	256
#define PAGE_SIZE		sysconf(_SC_PAGE_SIZE)
#define STM_MAP_OFFSET		0x0
#define STM_MAP_SIZE		PAGE_SIZE

#define STM_DEVICE_NAME		"/dev/10006000.stm"

#define STP_POLICY_ID_SET	_IOWR('%', 0, struct stp_policy_id)

void main()
{
	int fd;
	char *map;
	unsigned int length = STM_MAP_SIZE;
	unsigned long offset = STM_MAP_OFFSET;
	struct stp_policy_id *policy;
	unsigned int trace_data[4] = {0x55555555, 0xaaaaaaaa, 0x66666666, 0x99999999};
	unsigned int size = sizeof(unsigned int) * 4;

	fd = open(STM_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
		printf("Failed to open %s %s\n", STM_DEVICE_NAME,
			strerror(errno));
		return;
	}

	/* set a master/channel policy for this STM devicem, this
	 * is because that kernel have to know how many channels
	 * would be mapped, and the size of mapped memory must be
	 * a multiple of page size.
	 */
	policy = malloc(sizeof(struct stp_policy_id) + 8);
	if (!policy) {
		close(fd);
		return;
	}
	policy->channel = 0;
	policy->__reserved_0 = 0;
	policy->__reserved_1 = 0;
	policy->width = PAGE_SIZE / BYTES_PER_CHANNEL;
	policy->size = sizeof(struct stp_policy_id) + 8;
	memcpy(policy->id, "test", 8);

	if (ioctl(fd, STP_POLICY_ID_SET, policy) == -1) {
		printf("STP_POLICY_ID_SET failed %s %d\n",
		       strerror(errno), errno);
		free(policy);
		return;
	}

	map = (char *)mmap(0, length, PROT_READ|PROT_WRITE,
			   MAP_SHARED, fd, offset);
	if (map == MAP_FAILED) {
		printf("Failed to map %s\n", strerror(errno));
		goto out;
	}
	printf("Success to map channel(%u~%u) to 0x%x\n",
		policy->channel, (policy->width - policy->channel),
		(unsigned long)map);

	write(fd, trace_data, size);

	/* unmap the area & error checking */
	if (munmap(map, length) == -1)
		perror("user: Error un-mmapping the file");

out:
	free(policy);
	close(fd);
}

