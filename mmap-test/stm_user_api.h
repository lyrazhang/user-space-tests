#ifndef __STM_USER_API_H
#define __STM_USER_API_H

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

struct mem_map {
	unsigned long start;
	unsigned long length;
	char* map;
};

struct stm_dev {
	int fd;
	struct stp_policy_id *policy;
	struct mem_map mmap;
} g_stm_dev;

void enable_sink();
int set_policy(int fd, struct stp_policy_id *policy);
int request_stm_resource(struct stm_dev *dev);
void release_stm_reaource(struct stm_dev *dev);

#endif /* __STM_USER_API_H */
