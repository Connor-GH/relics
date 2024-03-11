#ifndef __KERNEL_STRING_H
#define __KERNEL_STRING_H

#include <typedefs.h>
#include <orbit.h>

uint32_t
strlen(const char *s);

int
strcmp(const char *__borrowed s1, const char *__borrowed s2);
int
strncmp(const char *__borrowed s1, const char *__borrowed s2, size_t n);

void *
memcpy(void *__inout dest, void *__borrowed src, size_t n);
void *
memset(void *__inout dest, int c, size_t n);

char *
strcpy(char *__inout dest, const char *__borrowed src);
char *
kernel_strncpy(char *__inout dest, const char *__borrowed src, size_t n);
#endif /* __KERNEL_STRING_H */
