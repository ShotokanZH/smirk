#include "smirk.h"

int matches_reg(const char *, const char *);
int is_magicfile(const char *);
int is_net_file(const char *);
int fake_netstat(FILE *(*f)(const char *, const char *), char *, char *);

