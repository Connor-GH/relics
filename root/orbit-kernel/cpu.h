#ifndef CPU_H
#define CPU_H
#include <inttypes.h>
#include <orbit.h>
typedef struct cpu_features_struct {
	_Bool fpu;
	_Bool cpuid;
	_Bool mmx;
	_Bool sse;
	_Bool sse2;
	_Bool ssse3;
	_Bool sse4_1;
	_Bool sse4_2;
	_Bool avx;
	_Bool avx2;
	_Bool avx512;
} cpu_features_struct;
extern cpu_features_struct cpu_features;
void __attribute__((noreturn)) halt(void);
void
cpuid_call(u32 id, u32 cnt, u32 *__owned a, u32 *__owned b, u32 *__owned c,
		   u32 *__owned d);
void
cpuflags(void);
void
set_cpu_vendor_name(void);
void
test_fpu(int a, int b);
extern u8 cpu_vendor_name[13];
#endif /* CPU_H */
