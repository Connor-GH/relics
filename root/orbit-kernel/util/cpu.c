/* everything to do with early CPU management */
#include <cpu.h>
#include <kio.h>
#include <orbit-kernel/orbit.h>
#include <stdbool.h>
// bitops, cpuflags
#define _UL(x) ((unsigned long)x)
#define _BITUL(x) (_UL(1) << (x))

static u32 cpu_vendor[3];
u8 cpu_vendor_name[13] = { 0 };

cpu_features_struct cpu_features = {
	.fpu = 0,
	.cpuid = 0,
	.mmx = 0,
	.sse = 0,
	.sse2 = 0,
	.ssse3 = 0,
	.sse4_1 = 0,
	.sse4_2 = 0,
	.avx = 0,
	.avx2 = 0,
	.avx512 = 0,
};

static bool
has_fpu(void)
{
	u16 fcw = 0, fsw = 0;
	unsigned long cr0 = 0;

	__asm__ __volatile__("mov %%cr0,%0\t\n" : "=r"(cr0));
	if (cr0 & (_BITUL(2) | _BITUL(3))) {
		cr0 &= ~(_BITUL(2) | _BITUL(3));
		__asm__ __volatile__("mov %0,%%cr0\t\n" : : "r"(cr0));
	}

	__asm__ __volatile__("fninit\t\n"
						 "fnstsw %0\t\n"
						 "fnstcw %1\t\n"
						 : "+m"(fsw), "+m"(fcw));

	return fsw == 0 && (fcw & 0x103f) == 0x003f;
}

#ifdef __x86_64__
#define PUSHF "pushfq"
#define POPF "popfq"
#else
#define PUSHF "pushfl"
#define POPF "popfl"
#endif /* __x86_64 */

static bool
has_eflag(unsigned long mask)
{
	unsigned long f0 = 0, f1 = 0;

	__asm__ __volatile__(PUSHF "    \n\t" PUSHF "    \n\t"
							   "pop %0    \n\t"
							   "mov %0,%1 \n\t"
							   "xor %2,%1 \n\t"
							   "push %1   \n\t" POPF " \n\t" PUSHF "    \n\t"
							   "pop %1    \n\t" POPF
						 : "=&r"(f0), "=&r"(f1)
						 : "ri"(mask));

	return !!((f0 ^ f1) & mask);
}
void
cpuid_call(u32 id, u32 cnt, u32 *__owned a, u32 *__owned b, u32 *__owned c,
           u32 *__owned d)
{
	__asm__ __volatile__("movl %0, %%eax\t\n"
						 "cpuid\t\n"
						 : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
						 : "0"(id), "2"(cnt));
}

static bool
has_sse(void)
{
	u32 a, b, c, d;
	a = b = c = d = 0;

	cpuid_call(0x1, 0, &a, &b, &c, &d);
	if ((d & CPUID_FEAT_EDX_SSE) && (d & CPUID_FEAT_EDX_FXSR)) {
		/* storage for FPU to return to and such, aligned for performance */
		char fxsave_region[512] __attribute__((aligned(16)));
		__asm__ __volatile__("fxsave %0" : : "m"(fxsave_region));
		/* activate SSE */
		__asm__ __volatile__("mov %%cr0, %%rax\t\n"
							 "and $0xFFFB, %%ax\t\n"
							 "or $0x2, %%ax\t\n"
							 "mov %%rax, %%cr0\t\n"
							 "mov %%cr4, %%rax\t\n"
							 "or $1536, %%ax\t\n"
							 "mov %%rax, %%cr4\t\n"
							 :
							 :
							 :);

		return true;
	} else {
		log_printk("Something went wrong with the SSE process\n");
		return false;
	}
}
/*void
test_fpu(volatile int a, volatile int b)
{
	volatile float k = (volatile float)a / (volatile float)b;
	printk("%d\n", (int)k);
}*/

void
cpuflags(void)
{
	u32 intel_level;
	if (has_fpu() > 0) { /* activate fpu */
		cpu_features.fpu = 1;
		log_printk("FPU available\n");
	} else {
		log_printk("No FPU available; halting\n");
		HALT;
	}
	if (has_sse() > 0) {
		cpu_features.sse = 1;
		log_printk("SSE available\n");
	}

	/* Check whether the ID bit in eflags is supported. Support means
     * that use of cpuid is available. */
	if (has_eflag(0x200000)) {
		cpu_features.cpuid = 1;
		/* call cpuid with zero */
		cpuid_call(0x0, 0, &intel_level, &cpu_vendor[0], &cpu_vendor[2],
				   &cpu_vendor[1]);
	} else {
		HALT;
	}
}
static void model_family_stepping(void) {
  u32 a, b, c, d;
  u8 model = 0;
  u16 family = 0;
  u8 processor_stepping = 0;
  cpuid_call(0x1, 0, &a, &b, &c, &d);
  processor_stepping = a & 0xF; // 15
  model = (a >> 4) & 0xF;
  family = (a >> 8) & 0xF;
  if (family == 0xF) {
    family += (a >> 20) & 0xFF; // extended family id
  }
  if (family == 0xF || family == 0x6) {
    model += ((a >> 16) & 0xFF) << 4; // extended model
  }
  log_printk("CPU Model=%x Family=%x Stepping=%x\n", model, family, processor_stepping);

}
static void remaining_features(void) {
  u32 a, b, c, d;
struct cpuid_struct {
    u32 feature;
    const char *const feature_string;
  };
  a = b = c = d = 0;
  cpuid_call(CPUID_EAX_GETFEATURES, 0, &a, &b, &c, &d);
  const struct cpuid_struct cpuidstruct_ecx[31] = {
    {CPUID_FEAT_ECX_SSE3, "sse3"},
    {CPUID_FEAT_ECX_PCLMUL, "pclmul"},
    {CPUID_FEAT_ECX_DTES64, "dtes64"},
    {CPUID_FEAT_ECX_MONITOR, "monitor"},
    {CPUID_FEAT_ECX_DS_CPL, "ds_cpl"},
    {CPUID_FEAT_ECX_VMX, "vmx"},
    {CPUID_FEAT_ECX_SMX, "smx"},
    {CPUID_FEAT_ECX_EST, "est"},
    {CPUID_FEAT_ECX_TM2, "tm2"},
    {CPUID_FEAT_ECX_SSSE3, "ssse3"},
    {CPUID_FEAT_ECX_CID, "cid"},
    {CPUID_FEAT_ECX_SDBG, "sdbg"},
    {CPUID_FEAT_ECX_FMA, "fma"},
    {CPUID_FEAT_ECX_CX16, "cx16"},
    {CPUID_FEAT_ECX_XTPR, "xtpr"},
    {CPUID_FEAT_ECX_PDCM, "pdcm"},
    {CPUID_FEAT_ECX_PCID, "pcid"},
    {CPUID_FEAT_ECX_DCA, "dca"},
    {CPUID_FEAT_ECX_SSE4_1, "sse4_1"},
    {CPUID_FEAT_ECX_SSE4_2, "sse4_2"},
    {CPUID_FEAT_ECX_X2APIC, "x2apic"},
    {CPUID_FEAT_ECX_MOVBE, "movbe"},
    {CPUID_FEAT_ECX_POPCNT, "popcnt"},
    {CPUID_FEAT_ECX_TSC, "tsc"},
    {CPUID_FEAT_ECX_AES, "aes"},
    {CPUID_FEAT_ECX_XSAVE, "xsave"},
    {CPUID_FEAT_ECX_OSXSAVE, "osxsave"},
    {CPUID_FEAT_ECX_AVX, "avx"},
    {CPUID_FEAT_ECX_F16C, "f16c"},
    {CPUID_FEAT_ECX_RDRAND, "rdrand"},
    {CPUID_FEAT_ECX_HYPERVISOR, "hypervisor"},
  };
  struct cpuid_struct cpuidstruct_edx[31] = {
    {CPUID_FEAT_EDX_FPU, "fpu"},
    {CPUID_FEAT_EDX_VME, "vme"},
    {CPUID_FEAT_EDX_DE, "de"},
    {CPUID_FEAT_EDX_PSE, "pse"},
    {CPUID_FEAT_EDX_TSC, "tsc"},
    {CPUID_FEAT_EDX_MSR, "msr"},
    {CPUID_FEAT_EDX_PAE, "pae"},
    {CPUID_FEAT_EDX_MCE, "mce"},
    {CPUID_FEAT_EDX_CX8, "cx8"},
    {CPUID_FEAT_EDX_APIC, "apic"},
    {CPUID_FEAT_EDX_SEP, "sep"},
    {CPUID_FEAT_EDX_MTRR, "mtrr"},
    {CPUID_FEAT_EDX_PGE, "pge"},
    {CPUID_FEAT_EDX_MCA, "mca"},
    {CPUID_FEAT_EDX_CMOV, "cmov"},
    {CPUID_FEAT_EDX_PAT, "pat"},
    {CPUID_FEAT_EDX_PSE36, "pse36"},
    {CPUID_FEAT_EDX_PSN, "psn"},
    {CPUID_FEAT_EDX_CLFLUSH, "clflush"},
    {CPUID_FEAT_EDX_DS, "ds"},
    {CPUID_FEAT_EDX_ACPI, "acpi"},
    {CPUID_FEAT_EDX_MMX, "mmx"},
    {CPUID_FEAT_EDX_FXSR, "fxsr"},
    {CPUID_FEAT_EDX_SSE, "sse"},
    {CPUID_FEAT_EDX_SSE2, "sse2"},
    {CPUID_FEAT_EDX_SS, "ss"},
    {CPUID_FEAT_EDX_HTT, "htt"},
    {CPUID_FEAT_EDX_TM, "tm"},
    {CPUID_FEAT_EDX_IA64, "ia64"},
    {CPUID_FEAT_EDX_PBE, "pbe"},
  };
  log_printk("Cpu features: ");
  for (size_t i = 0; i < 31; i++) {
    if (c & cpuidstruct_ecx[i].feature) {
      printk("%s ", cpuidstruct_ecx[i].feature_string);
    }
  }
  for (size_t i = 0; i < 31; i++) {
    if (c & cpuidstruct_edx[i].feature) {
      printk("%s ", cpuidstruct_edx[i].feature_string);
    }
  }
  printk("\n");
}

void
set_cpu_vendor_name(void)
{
	u32 bitmask = 255; // 11111111
	cpuflags();
	cpu_vendor_name[0] = (u8)(cpu_vendor[0] & bitmask); // lower 7-0 bits
	cpu_vendor_name[1] =
		(u8)((cpu_vendor[0] & (bitmask << 8)) >> 8); // 15-8 bits
	cpu_vendor_name[2] =
		(u8)((cpu_vendor[0] & (bitmask << 16)) >> 16); // 23-16 bits
	cpu_vendor_name[3] =
		(u8)((cpu_vendor[0] & (bitmask << 24)) >> 24); // 31-24 bits
	cpu_vendor_name[4] = (u8)(cpu_vendor[1] & bitmask);
	cpu_vendor_name[5] = (u8)((cpu_vendor[1] & (bitmask << 8)) >> 8);
	cpu_vendor_name[6] = (u8)((cpu_vendor[1] & (bitmask << 16)) >> 16);
	cpu_vendor_name[7] = (u8)((cpu_vendor[1] & (bitmask << 24)) >> 24);
	cpu_vendor_name[8] = (u8)(cpu_vendor[2] & bitmask);
	cpu_vendor_name[9] = (u8)((cpu_vendor[2] & (bitmask << 8)) >> 8);
	cpu_vendor_name[10] = (u8)((cpu_vendor[2] & (bitmask << 16)) >> 16);
	cpu_vendor_name[11] = (u8)((cpu_vendor[2] & (bitmask << 24)) >> 24);
	cpu_vendor_name[12] = '\0';
	log_printk("CPU Vendor: %s\n", cpu_vendor_name);
  model_family_stepping();
  // TODO space-related issues
  remaining_features();
}
