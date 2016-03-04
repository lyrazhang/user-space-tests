/* pidns_init_sleep.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   A simple demonstration of PID namespaces.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#if 0
static void read_ctxval()
{
	FILE * fp;
	unsigned long x = 0;
	fp=fopen("/sys/bus/coresight/devices/10540000.etm/ctxid_val", "r+");
	if(!fp) return;
	fscanf(fp, "%lu", &x);

	fclose(fp);
	printf("cat /sys/bus/coresight/devices/10540000.etm/ctxid_val %lu\n", x);
}

static void write_ctxval(unsigned long pid)
{
	FILE * fp;
	unsigned long x = 0;
	fp=fopen("/sys/bus/coresight/devices/10540000.etm/ctxid_val", "w+");
	if(!fp) return;
	fprintf(fp, "%lu", pid);

	fclose(fp);
	printf("echo %lu > /sys/bus/coresight/devices/10540000.etm/ctxid_val\n", pid);
}
#endif

static int childFunc(void *arg)
{
	int pid = getpid();
	int ppid = getppid();
	char s[256] = {0};

	printf("childFunc(): PID  = %ld\n", (long) pid);
	printf("childFunc(): PPID = %ld\n", (long) ppid);

	char *mount_point = arg;

	if (mount_point != NULL) {
		mkdir(mount_point, 0555);       /* Create directory for mount point */
	if (mount("proc", mount_point, "proc", 0, NULL) == -1)
		errExit("mount");
	printf("Mounting procfs at %s\n", mount_point);
    }

	sprintf(s, "echo %d > /sys/bus/coresight/devices/10540000.etm/ctxid_val", pid);
	system(s);

	sleep(1);
	system("cat /sys/bus/coresight/devices/10540000.etm/ctxid_val");
}

#define STACK_SIZE (1024 * 1024)

/*static char child_stack[STACK_SIZE];*/    /* Space for child's stack */

int create_proccess(void* mount_point)
{
	pid_t child_pid;
	static char *child_stack;

	child_stack = malloc(STACK_SIZE);
	if (!child_stack) {
		printf("malloc(STACK_SIZE) failed: %m\n");
		abort();
	}

	child_pid = clone(childFunc,
                    child_stack + STACK_SIZE,   /* Points to start of downwardly growing stack */
                    CLONE_NEWPID | SIGCHLD, mount_point);

	if (child_pid == -1)
		errExit("clone");

	printf("PID returned by clone(): %ld\n", (long) child_pid);

	if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
		errExit("waitpid");

	return 0;
}

int main(int argc, char *argv[])
{
	create_proccess(argv[1]);
	create_proccess(argv[2]);
	return 0;
}


