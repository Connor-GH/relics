#ifndef ORBIT_H
#define ORBIT_H
#include <asm/wrappers.h>

#ifndef __GNUC__
#define __attribute__(x)
#error The kernel currently hangs without __attribute__ support.
#endif

#define ATTR(x) __attribute__((x))
#define NOINLINE ATTR(noinline)

#if defined(__has_builtin) && __has_builtin(__builtin_expect)
#define unlikely(x) __builtin_expect((x), 0)
#define likely(x) __builtin_expect((x), 1)
#else
#define unlikely(x) x
#define likely(x) x
#endif
#if defined(__has_builtin) && __has_builtin(__builtin_unreachable)
#define UNREACHABLE __builtin_unreachable()
#else
#define UNREACHABLE                  \
	volatile char UB = *((char *)0); \
	do {                             \
	} while (0)
#endif

#define CODE16 ASM(".code16\t\n")
#endif /* ORBIT_H */
