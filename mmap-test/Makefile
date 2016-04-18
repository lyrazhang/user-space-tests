#~/tools/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/bin/aarch64-linux-gnu-gcc ioctl-test.c --static -o ioctl-test

CROSS_COMPILE	:= /home/zcy/tools/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/bin/aarch64-linux-gnu-
CC				:= $(CROSS_COMPILE)gcc
CFLAGS			:= --static

MMAP_TEST	= mmap_test
STM_USER	= stm_user_example

OBJS_MMAP_TEST= mmap_test.o
OBJS_STM_USER_API= example.o stm_user_api.o

all: $(STM_USER) $(MMAP_TEST)

$(STM_USER):$(OBJS_STM_USER_API)
	$(CC) $(CFLAGS) -o $@ $^

#$(OBJS_STM_USER_API):%.o:%.c
#	$(CC) -c $< -o $@

$(MMAP_TEST):$(OBJS_MMAP_TEST)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o *~ $(MMAP_TEST) $(STM_USER)

###----------------------------------------###
#stm_user_example: example.o stm_user_api.o
#	$(CC) $(CFLAGS) -o $@ $^
##	$(CC) -o stm_user_example example.o stm_user_api.o
#
#example.o: example.c stm_user_api.h
#	$(CC) -c $<
#
#stm_user_api.o: stm_user_api.c stm_user_api.h
#	$(CC) -c stm_user_api.c
#
#clean:
#	rm *.o stm_user_example
