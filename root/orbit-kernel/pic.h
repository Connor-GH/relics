#ifndef _KERNEL_PIC_H
#define _KERNEL_PIC_H
#include <typedefs.h>

#define PIC1		0x20		/* IO base address for primary PIC */
#define PIC2		0xA0		/* IO base address for secondary PIC */
#define PIC1_CMD	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_CMD	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI 0x20 /* End of Interrupt command code */
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28
#define PIC2_END PIC2_OFFSET + 7
#define PIC_ACKNOWLEDGE 0x20


/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4	0x01		        /* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		        /* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		    /* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		        /* Level triggered (edge) mode */
#define ICW1_INIT	0x10		        /* Initialization - required! */

#define ICW4_8086	0x01		        /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		        /* Auto (normal) EOI */
#define ICW4_BUF_SECONDARY	0x08		/* Buffered mode/secondary */
#define ICW4_BUF_PRIMARY	0x0C		/* Buffered mode/primary */
#define ICW4_SFNM	0x10		        /* Special fully nested (not) */

#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

void PIC_enable(void);
void PIC_disable(void);
void PIC_send_eoi(uint8_t irq);
void PIC_remap(int32_t offset);
void PIC_acknowledge(uint32_t interrupt);
uint16_t PIC_get_irr(void);
uint16_t PIC_get_isr(void);
void IRQ_set_mask(uint8_t irq_line);
void IRQ_clear_mask(uint8_t irq_line);


#endif /* _KERNEL_PIC_H */
