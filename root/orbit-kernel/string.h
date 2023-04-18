#ifndef __KERNEL_STRING_H
#define __KERNEL_STRING_H

#include <stddef.h>
#include <typedefs.h>

uint32_t strlen(const char *s);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

void *memcpy(void *dest, void *src, size_t n);
char *strcpy(char *dest, const char *src);
char *kernel_strncpy(char *dest, const char *src, size_t n);
#endif /* __KERNEL_STRING_H */
