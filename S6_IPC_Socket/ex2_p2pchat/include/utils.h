#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void safe_printf(const char *fmt, ...);
int get_local_ip(char *buf, size_t buflen);
int is_valid_ipv4(const char *ip);

#endif