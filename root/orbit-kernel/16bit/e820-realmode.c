#include <typedefs.h>
#include <kio-realmode.h>
#include <e820-realmode.h>

// TODO file unused
static SMAP_entry_t *entries_buffer = 0;
static SMAP_entry_t *smap = (SMAP_entry_t *)0x1000;
static const int smap_size = 0x2000;

static void
setMemMap(SMAP_entry_t *buf)
{
	entries_buffer = buf;
}
SMAP_entry_t *
getMemMap(void)
{
	return entries_buffer;
}
// load memory map to buffer - note: regparm(3) avoids stack issues with gcc in real mode
static int __attribute__((noinline)) __attribute__((regparm(3)))
detectMemory(int maxentries)
{
	uint32_t contID = 0;
	int entries = 0, signature = 0, bytes = 0;
	//#if 0
	do {
		__asm__ __volatile__("int  $0x15"
							 : "=a"(signature), "=c"(bytes), "=b"(contID)
							 : "a"(0xE820), /* eax value */
							   "b"(contID), /* continuous address value */
							   "c"(24), /* ACPI size */
							   "d"(0x534D4150), /* "SMAP" */
							   "D"(smap) /* list of all values */
		);
		if (signature != 0x534D4150)
			return -1; // error
		if (bytes > 20 && (smap->ACPI & 0x0001) == 0) {
			// ignore this entry
			realmode_printk("LOG: ACPI Entry ignored!\n");
		} else {
			smap++; /* pointer is not incremented for us */
			entries++;
		}
		realmode_printk("%x-%x ", smap->BaseL, smap->BaseH);
		switch (smap->Type) {
		case '1':
			realmode_printk("(Memory)");
			break;
		case '2':
			realmode_printk("(Reserved)");
			break;
		case '3':
			realmode_printk("(ACPI)");
			break;
		case '4':
			realmode_printk("(NVS)");
			break;
		case '5':
			realmode_printk("(Unusable)");
			break;
		case '6':
			realmode_printk("(Disabled)");
			break;
		default:
			realmode_printk("(Unknown)");
			break;
		}
		realmode_printk("\n");
	} while (contID != 0 && entries < maxentries);
	setMemMap(smap);
	//#endif
	return entries;
}

void
memory_map(void)
{
	realmode_init_vga(WHITE, BLUE);
	realmode_reset_video_memory();

	int entry_count = detectMemory(smap_size / sizeof(SMAP_entry_t));

	if (entry_count == -1) {
		realmode_printk("Memory issue.\n");
		__asm__ __volatile__("hlt\t\n");
	} else {
		realmode_printk("Memory Map has been successfully parsed!\n");
		realmode_printk("Entry count: %d\n", entry_count);
	}
}
