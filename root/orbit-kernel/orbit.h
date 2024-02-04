#ifndef ORBIT_H
#define ORBIT_H
#include <asm/wrappers.h>


#ifndef __GNUC__
#define __attribute__(x)
#error The kernel currently hangs without __attribute__ support.
#endif

#define ATTR(x) __attribute__((x))
#define NOINLINE ATTR(noinline)
#endif /* ORBIT_H */
