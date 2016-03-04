#~/tools/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/bin/aarch64-linux-gnu-gcc ioctl-test.c --static -o ioctl-test

CROSS_COMPILE	:= /home/zcy/tools/gcc-linaro-aarch64-linux-gnu-4.9-2014.09_linux/bin/aarch64-linux-gnu-
CC				:= $(CROSS_COMPILE)gcc
OBJ				:= ioctl-test
SOURCE			:= $(OBJ).c
CFLAGS			:= --static

all:$(OBJ)

%.o:%.c
	$(CC) -c $(SOURCE) $(CFLAGS) -o $(OBJ)

clean:
	rm $(OBJ)
