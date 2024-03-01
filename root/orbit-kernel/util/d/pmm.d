module pmm;

@safe @nogc nothrow:


@system extern (C) extern __gshared struct e820_map_64 {
	align(1):
	ulong baseaddr;
	ulong length;
	uint type;
}

// both of these are initialized from C in "get_mem_map()"
@system extern (C) extern __gshared e820_map_64 *e820_map;
@system extern (C) extern __gshared uint e820_count;

@system extern (C) void printk(const char *fmt, ...);

enum uint E820_MAX_ENTRIES = 100;
enum e820_type {
	Usable = 1,
	Reserved,
	ACPI,
	NVS,
	Unusable,
	Pmem
}
//static mem_alloc_entry *alloc_table = void;

//static mem_alloc_entry *alloc_table = void;
//static size_t total_length = 0;
//void *page_allocator(size_t num_pages) {

//}

//struct mem_alloc_entry {
//	void *ptr;
//	size_t length;
//	mem_alloc_entry *next;
//}

/*@system extern (C) void bootstrap_mem_allocator() {
	alloc_table.ptr = alloc_table;
	if (total_length < usable_map[0].length)
		alloc_table.length = E820_MAX_ENTRIES * total_length;
	else
		printk("FAILED\n");
}*/

//void *mem_allocator(size_t size) {

//}

// initialize map of usable pages
@system extern (C) void pmem_map_d() {
	e820_map_64[E820_MAX_ENTRIES] usable_map;
	size_t usable_map_len = 0;
	size_t total_length = 0;
	printk("Hello from D!: %u\n", e820_count);
	foreach (i; 0 .. e820_count) {
		if (e820_map[i].type == e820_type.Usable) {
			usable_map[usable_map_len] = e820_map[i];
			usable_map_len++;
		}
	}
	printk("Usable map\n");
	foreach (i; 0 .. usable_map_len) {
		printk("%0lx-%0lx\n", usable_map[i].baseaddr,
				usable_map[i].baseaddr+usable_map[i].length);
		total_length += usable_map[i].length;
	}
	printk("%llu bytes available (%lluMiB)\n", total_length,
			total_length / 1024	/ 1024);
	asm pure @safe @nogc nothrow {
		"cli;" ~
		"hlt";
	}
}
