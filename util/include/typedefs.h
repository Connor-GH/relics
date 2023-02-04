#ifndef TYPEDEF_H
#define TYPEDEF_H


// These definitions are not great.
// A long will also be needed for 64-bit

typedef char            int8_t;
typedef short           int16_t;
typedef int             int32_t;

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;



#include "wordsize.h"



#if __WORDSIZE == 64
typedef unsigned long   uint64_t;
typedef long            int64_t;
typedef uint64_t        size_t;
typedef int64_t         ssize_t;
#  ifndef __intptr_t_defined
     typedef long int       intptr_t;
#    define __intptr_t_defined
#  endif
typedef unsigned long int   uintptr_t;
#  else
typedef uint32_t        size_t;
typedef int32_t         ssize_t;
#    ifndef __intptr_t_defined
       typedef int          intptr_t;
#      define __intptr_t_defined
#    endif
typedef unsigned int        uintptr_t;
#endif

#endif
