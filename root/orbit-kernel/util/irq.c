#include <pit.h>
#include <idt.h>
#include <irq.h>
#include <kio.h>

// returns seconds and milliseconds since boot

const char *
PIT_IRQ_timer_get_current_time_since_boot(void)
{
	if (!PIT_enabled()) {
		return "0.000";
	} else {
		uint64_t ticks =  get_pit_ticks();
		char num[64];
		return milliseconds_as_seconds(ticks, num);
	}
}
