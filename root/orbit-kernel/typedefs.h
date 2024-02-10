#ifndef TYPEDEF_H
#define TYPEDEF_H

/* Rationale:
 * type widths are different on
 * different architectures. In order
 * to make this kernel *slightly*
 * more portable, there will be an
 * ifdef guard on these basic types.
 * This way, if someone on ARM decides
 * to try and compile this, it will not
 * only fail because of the massive
 * amount of assembly and machine-specific
 * code, but there also won't be stdint
 * types :) */
#if defined(__X86_64__) || defined(__i386__)
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

#include <wordsize.h>

/* The C standard defines that `long long'
 * always be at least 64 bits. Since this
 * is x86, it can be inferred that `long' is
 * 64 bits here. */
#if __WORDSIZE == 64
typedef unsigned long uint64_t;
typedef long int64_t;
typedef uint64_t size_t;
typedef int64_t ssize_t;
#ifndef __intptr_t_defined
typedef long int intptr_t;
#define __intptr_t_defined
#endif
typedef unsigned long int uintptr_t;
#else
typedef uint32_t size_t;
typedef int32_t ssize_t;
#ifndef __intptr_t_defined
typedef int intptr_t;
#define __intptr_t_defined
#endif
typedef unsigned int uintptr_t;
#endif

#endif /* TYPEDEFS_H */
