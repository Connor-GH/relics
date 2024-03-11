#include <pit.h>
#include <idt.h>
#include <irq.h>
#include <kio.h>
#include <orbit.h>

// returns seconds and milliseconds since boot

const char *
PIT_IRQ_timer_get_current_time_since_boot(char *__inout buf)
{
	if (!PIT_enabled()) {
		return "0.000";
	} else {
		uint64_t ticks = get_pit_ticks();
		return milliseconds_as_seconds(ticks, buf, 64);
	}
}
