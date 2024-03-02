#include <kio.h>
#include <pit.h>
#include <inttypes.h>
#include <asm/wrappers.h>
#include <stdbool.h>
#define PIT_FREQUENCY 1193182
#define PIT_CMD_REG 0x43
#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42

extern volatile size_t countdown;
static bool enabled = false;

bool
PIT_enabled(void)
{
	return enabled;
}

void
millisleep(uint64_t millis)
{
	countdown = millis;
	while (countdown > 0) {
		ASM("hlt");
	}
}

void
reprogram_timer(uint16_t hz)
{
	if (hz == 0) {
		log_printk("Hz cannot be zero, not reprogramming.\n");
		return;
	}
	uint16_t divisor = (uint16_t)(PIT_FREQUENCY / hz);
	uint8_t command_byte = 0;
	command_byte |= 0 << 0; // BCD; bit 0
	command_byte |= 3 << 1; // Square Wave mode; bits 1-3
	command_byte |= 3 << 4; // RW mode LSB then MSB; bits 4-5
	command_byte |= 0 << 6; // Counter #0; bits 6-7

	outb(PIT_CMD_REG, command_byte);
	outb(PIT_CHANNEL0, divisor & 0xFF);
	outb(PIT_CHANNEL0, divisor >> 8);
	enabled = true;
	log_printk("PIT enabled, set to %ldHz\n", hz);
}
