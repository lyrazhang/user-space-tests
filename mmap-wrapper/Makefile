CROSS_COMPILE	:= /home/zcy/tools/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/bin/aarch64-linux-gnu-
CC				:= $(CROSS_COMPILE)gcc
CFLAGS			:= --static

STM_USER	= stm_user_example

OBJS_STM_USER_API= example.o stm_user_api.o

all: $(STM_USER)

$(STM_USER):$(OBJS_STM_USER_API)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o $(STM_USER)
