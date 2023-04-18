#include <sys/syscall.h>

long syscall(long number, ...) {
#if COMPILED_ARCHITECTURE == "x86-64"
/* amd64 architecture has syscall take in
 * the following registers as arguments:
 * rdi, rsi, rdx, r10, r8, and r9.
 * It returns rax with a secondary
 * return value in rdx. */
    long result = 0;
__asm__ __volatile__
    ("push %rdi\t\n"
    "push %rsi\t\n"
    "push %rdx\t\n"
    "push %r10\t\n"
    "push %r8\t\n"
    "push %r9\t\n"
    "mov %0, %rdi\t\n"
    "mov %1, %rsi\t\n"
    "mov %2, %rdx\t\n"
    "mov %3, %r10\t\n"
    "mov %4, %r8\t\n"
    "mov %5, %r9\t\n"
    "syscall\t\n"
    "pop %rdi\t\n"
    "pop %rsi\t\n"
    "pop %rdx\t\n"
    "pop %r10\t\n"
    "pop %r8\t\n"
    "pop %r9\t\n"
    : "=a" (result)
    :
    :
    );
return result;
#endif /* COMPILED_ARCHITECTURE */
}
