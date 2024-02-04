#include <pic.h>
#include <port_functions.h>
#include <typedefs.h>
#include <kio.h>

/* This file describes the actions
 * required to initialize the
 * Programmable Interrupt Controller
 * required for hardware IRQs. */

static void
io_wait(void)
{
	outb(0x80, 0);
}

void
PIC_enable(void)
{
	PIC_remap(0x20);
	for (uint8_t irq = 0; irq < 16; irq++)
		IRQ_clear_mask(irq);
}
void
PIC_disable(void)
{
	//PIC_remap(0x20);
	//for (uint8_t irq = 0; irq < 16; irq++)
	//   IRQ_set_mask(irq);
	outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);
}

/* offset1 - vector offset for primary PIC */
/* offset2 - same for secondary PIC: offset2..offset2+7 */
void
PIC_remap(int32_t offset)
{
	uint8_t a1, a2;
	a1 = inb(PIC1_DATA);
	io_wait();
	a2 = inb(PIC2_DATA);

	outb(PIC1_CMD,
		 ICW1_INIT |
			 ICW1_ICW4); // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset); // ICW2: primary PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset + 0x08); // ICW2: secondary PIC vector offset
	io_wait();
	outb(
		PIC1_DATA,
		4); // ICW3: tell primary PIC that there is a secondary PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA,
		 2); // ICW3: tell secondary PIC its cascade identity (0000 0010)
	io_wait();

	outb(PIC1_DATA,
		 ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, a1); // restore saved masks.
	io_wait();
	outb(PIC2_DATA, a2);
	log_printk("PIC remapped to %x\n", offset);
}
void
PIC_acknowledge(uint8_t interrupt)
{
	//if ((interrupt < PIC1_OFFSET) || (interrupt > PIC2_END)) {
	//	return;
	//}

	if (interrupt >= 40) {
		outb(PIC2_CMD, PIC_ACKNOWLEDGE);
	}
	outb(PIC1_CMD, PIC_ACKNOWLEDGE);
}

void
IRQ_set_mask(uint8_t irq_line)
{
	uint16_t port;
	uint8_t value;

	if (irq_line < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq_line -= 8;
	}
	value = inb(port) | (1 << irq_line);
	outb(port, value);
}

void
IRQ_clear_mask(uint8_t irq_line)
{
	uint16_t port;
	uint8_t value;

	if (irq_line < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq_line -= 8;
	}
	value = inb(port) & ~(1 << irq_line);
	outb(port, value);
}

static uint16_t
__pic_get_irq_reg(int ocw3)
{
	/* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
	outb(PIC1_CMD, ocw3);
	outb(PIC2_CMD, ocw3);
	return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t
PIC_get_irr(void)
{
	return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t
PIC_get_isr(void)
{
	return __pic_get_irq_reg(PIC_READ_ISR);
}
