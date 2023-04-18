#ifndef CPU_H
#define CPU_H
#include <inttypes.h>
void __attribute__((noreturn)) halt(void);
void cpuid_call(u32 id, u32 cnt, u32 *a, u32 *b, u32 *c, u32 *d);
void cpuflags(void);
void set_cpu_vendor_name(void);
void test_fpu(int a, int b);
extern u8 cpu_vendor_name[13];
#endif /* CPU_H */
