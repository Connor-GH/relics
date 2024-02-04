#ifndef ASM_WRAPPERS_H
#define ASM_WRAPPERS_H
#include <port_functions.h>
#define ASM(x) __asm__ __volatile__(x); do{} while(0)
#define HALT ASM("hlt\t\n")
#endif /* ASM_WRAPPERS_H */
