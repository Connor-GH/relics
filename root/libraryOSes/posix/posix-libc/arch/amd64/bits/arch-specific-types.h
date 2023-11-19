#ifndef ARCH_SPECIFIC_TYPES_H
#define ARCH_SPECIFIC_TYPES_H



typedef char            int8_t;
typedef short           int16_t;
typedef int             int32_t;

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;


/* The C standard defines that `long long'
 * always be at least 64 bits. Since this
 * is x86, it can be inferred that `long' is
 * 64 bits here. */
typedef unsigned long   uint64_t;
typedef long            int64_t;
typedef uint64_t        size_t;
typedef int64_t         ssize_t;
#  ifndef __intptr_t_defined
     typedef long int intptr_t;
#    define __intptr_t_defined
#  endif
typedef unsigned long int uintptr_t;


typedef int wchar_t;
#if defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 2
typedef long double float_t;
typedef long double double_t;
#else
typedef float float_t;
typedef double double_t;
#endif

typedef struct { long long __ll; long double __ld; } max_align_t;

#endif /* ARCH_SPECIFIC_TYPES_H */
