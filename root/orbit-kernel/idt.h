#ifndef IDT_H
#define IDT_H
#include "typedefs.h"

typedef struct __attribute__((packed)) {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rbp;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t irq;
	uint64_t error_code;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
} regs_t;

//__attribute__((no_caller_saved_registers))
void
exception_handler(regs_t *r);

//void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags, uint8_t ist);
void
idt_init(void);

typedef enum {
	IDT_FLAG_GATE_TASK = 0x5,
	IDT_FLAG_GATE_INT = 0xe,
	IDT_FLAG_GATE_TRAP = 0xf,
	IDT_FLAG_GATE_CALL = 0xc,

	IDT_FLAG_RING0 = 0 << 5,
	IDT_FLAG_RING1 = 1 << 5,
	IDT_FLAG_RING2 = 2 << 5,
	IDT_FLAG_RING3 = 3 << 5,

	IDT_FLAG_PRESENT = 0x80,
} IDT_FLAGS;
#define IDT_DESCRIPTOR_EXCEPTION (IDT_FLAG_GATE_INT | IDT_FLAG_PRESENT)
typedef void (*interrupt_handler_t)(void);
void
enable_interrupts(void);
void
disable_interrupts(void);
uint64_t
get_pit_ticks(void);
#endif /* IDT_H */
