#ifndef __STM_USER_API_H
#define __STM_USER_API_H

#define BYTES_PER_CHANNEL	256
#define PAGE_SIZE		sysconf(_SC_PAGE_SIZE)
#define STM_MAP_OFFSET		0x0
#define STM_MAP_SIZE		PAGE_SIZE
#define STM_DEVICE_NAME		"/dev/10006000.stm"
#define TMC_SYS_NAME		"10003000.etf"
#define STM_SYS_NAME		"10006000.stm"
#define STP_POLICY_NAME		"test"
#define TEST_DATA_SIZE		4
#define POLICY_NAME_LEN		8

#define STP_POLICY_ID_SET	_IOWR('%', 0, struct stp_policy_id)

enum stm_flags {
	STM_FLAG_TIMESTAMPED	= 0x08,
	STM_FLAG_MARKED		= 0x10,
	STM_FLAG_GUARANTEED	= 0x80,
};

enum stm_pkt_type {
	STM_PKT_TYPE_DATA	= 0x98,
	STM_PKT_TYPE_FLAG	= 0xE8,
	STM_PKT_TYPE_TRIG	= 0xF8,
};

enum error_no {
	E_COMMON = -1,
};

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
	char *map;
};

struct stm_dev {
	int fd;
	struct stp_policy_id *policy;
	struct mem_map mmap;
} g_stm_dev;

unsigned int stm_wrbytes(const char *dev_name);
int request_stm_resource(struct stm_dev *dev, unsigned int chan, unsigned int width);
void release_stm_reaource(struct stm_dev *dev);
int stm_trace_data(struct stm_dev *dev, unsigned int chan, int flags, unsigned int size, void *data);

#endif /* __STM_USER_API_H */
