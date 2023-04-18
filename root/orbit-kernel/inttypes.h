#ifndef INTTYPE_H
#define INTTYPE_H

#include <typedefs.h>

/* shorthand names for the int types */

/* In my opinion, I feel like
 * these should be part of the actual
 * C standard. In the future, some of my
 * files may not include this macro, and
 * will require a parsing tool that
 * automatically finds what an "i8" and
 * such is, but this tool is incomplete */

#define i8  int8_t
#define i16 int16_t
#define i32 int32_t
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#if __WORDSIZE__ == 64
#define u64 uint64_t
#define i64 int64_t
#endif /* __WORDSIZE__ */

#endif /* INTTYPE_H */
