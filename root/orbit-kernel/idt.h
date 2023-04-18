#ifndef IDT_H
#define IDT_H
#include "typedefs.h"

__attribute__((noreturn))
void exception_handler(void);
void set_gdt_offset(long offset);

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void idt_init(void);

#endif /* IDT_H */
