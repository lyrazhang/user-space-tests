#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define STM_SET_GUARANTEED	0
#define STM_SET_INVARIANT	1
#define STP_SET_OPTIONS _IOW('%', 2, unsigned long long)
#define STP_GET_OPTIONS _IOR('%', 3, unsigned long long)

void main()
{
	int fd;
	unsigned long long val = STM_SET_GUARANTEED;

	fd = open("/dev/10006000.stm", O_RDWR);
	if (fd < 0)
		printf("Failed to open /dev/10006000.stm.. %s\n", strerror(errno));

	if (ioctl(fd, STP_GET_OPTIONS, &val) == -1)
		printf("STP_GET_OPTIONS failed!!! %s %d\n", strerror(errno), errno);

	printf("get_options = 0x%x\n", val);

	val = STM_SET_INVARIANT;
	if (ioctl(fd, STP_SET_OPTIONS, &val) == -1)
		printf("STP_SET_OPTIONS failed!!! %s %d\n", strerror(errno), errno);

	close(fd);
}

