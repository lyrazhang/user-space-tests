/* pidns_init_sleep.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   A simple demonstration of PID namespaces.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

/* Child's stack size */
#define STACK_SIZE (1024 * 1024)
/* ETM base address */
#define ETM1_BASE 10440000

#define USE_ECHO
#undef USE_ECHO

#ifndef USE_ECHO
#define RDWR_BUF_SIZE 32
static void read_file(const char* name)
{
	char *buf[RDWR_BUF_SIZE] = {0};
	ssize_t size = 0;
	int fd = open(name, O_RDONLY);
	if(fd == -1) {
		printf("failed to open the file.\n");
		return;
	}
	size = read(fd, buf, RDWR_BUF_SIZE);
	printf("***read file %s\n", buf);
	close(fd);

}

static void write_file(const char* name, unsigned long val)
{
	char buf[RDWR_BUF_SIZE] = {0};
	ssize_t size = 0;
	int fd = open(name, O_WRONLY);
	if(fd == -1) {
		printf("failed to open the file.\n");
		return;
	}

	sprintf(buf, "%x", val);

	size = write(fd, buf, RDWR_BUF_SIZE);
	close(fd);
}
#endif
static inline void set_ctxid(unsigned long addr)
{
	char s[256] = {0};
	int pid = getpid();
	printf("****set_ctxid: %d\n", pid);

#ifdef USE_ECHO
	sprintf(s, "echo %x > /sys/bus/coresight/devices/%lu.etm/ctxid_pid", pid, addr);
	system(s);

	/* disable & enable the etm, then the configuration will work */
	sprintf(s, "echo 0 >  /sys/bus/coresight/devices/%lu.etm/enable_source", addr);
	system(s);
	sprintf(s, "echo 1 >  /sys/bus/coresight/devices/%lu.etm/enable_source", addr);
	system(s);

	/* cat the new value just writen to */
	sprintf(s, "cat /sys/bus/coresight/devices/%lu.etm/ctxid_pid", addr);
	system(s);
#else
	sprintf(s, "/sys/bus/coresight/devices/%lu.etm/ctxid_pid", addr);
	write_file(s, pid);

	/* disable & enable the etm, then the configuration will work */
	sprintf(s, "/sys/bus/coresight/devices/%lu.etm/enable_source", addr);
	write_file(s, 0);
	write_file(s, 1);

	/* read the new value just writen to */
	sprintf(s, "/sys/bus/coresight/devices/%lu.etm/ctxid_pid", addr);
	read_file(s);
#endif
}

static int child_func()
{
	printf("child_func(): PID  = %ld\n", (long) getpid());
	printf("child_func(): PPID = %ld\n", (long) getppid());

	set_ctxid(ETM1_BASE);

	return 0;
}

static int thread_func()
{
	child_func();
	sleep(1);
	create_proccess(SIGCHLD, child_func);

	return 0;
}

int create_proccess(int flags, int (*func)(void *))
{
	pid_t child_pid;
	static char *child_stack;

	child_stack = malloc(STACK_SIZE);
	if (!child_stack) {
		printf("malloc failed: %m\n");
		abort();
	}

	printf("create_proccess: pid =%d flag=0x%x\n", getpid(), flags);

	child_pid = clone(func,
                    child_stack + STACK_SIZE,   /* Points to start of downwardly growing stack */
                    flags, NULL);

	if (child_pid == -1) {
		free(child_stack);
		errExit("clone");
	}

	printf("PID returned by clone(): %ld\n", (long) child_pid);

	if (waitpid(child_pid, NULL, 0) == -1) {      /* Wait for child */
		free(child_stack);
		errExit("waitpid");
	}

	free(child_stack);
	return 0;
}

int main(int argc, char *argv[])
{
	create_proccess(CLONE_NEWPID | SIGCHLD, thread_func);
	//create_proccess(SIGCHLD, thread_func);

	//create_proccess(CLONE_NEWPID | SIGCHLD, thread_func);
	create_proccess(SIGCHLD, thread_func);
	return 0;
}


