#ifndef ORBIT_H
#define ORBIT_H
#include "asm/wrappers.h"

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
#define UNREACHABLE() __builtin_unreachable()
#else
ATTR(noreturn) inline void unreachable_func(void) {}
#define UNREACHABLE() (unreachable_func())
#endif

#ifdef __clang__
#define NOOPT ATTR(optnone)
#else 
#define NOOPT ATTR(optimize("O0"))
#endif

#define CODE16 ASM(".code16\t\n")
#define CODE32 ASM(".code32\t\n")
#define CODE64 ASM(".code64\t\n")

/* type annotations */ 
// instructs that a type will be changed.
#define __owned
// instructs that a type will *not* be changed.
#define __borrowed const
// instructs that a type will be changed and returned.
#define __inout
#endif /* ORBIT_H */
