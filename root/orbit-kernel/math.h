#ifndef __KERNEL_MATH_H
#define __KERNEL_MATH_H
#include <typedefs.h>

/* All of these math functions are nonstandard. */
uint32_t ppowi(uint32_t num, uint32_t power);
int32_t  npowi(int32_t num, uint32_t power);
int32_t  powi(int32_t num,  uint32_t power);

#endif /* __KERNEL_MATH_H */
