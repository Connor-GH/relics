module pmm;

import std.safe_asm;

alias uintptr = ulong;

@nogc nothrow:
@trusted extern(C) __gshared void printk(const char *fmt, ...);
@trusted extern(C) __gshared void log_printk(const char *fmt, ...);
@trusted extern(C) __gshared void panic(int issue);
@trusted extern(C) __gshared void *strcpy(char *dst, const char *src);
@trusted extern(C) __gshared void *memcpy(void *dst, const void *src, size_t n);

__gshared enum MEMORY_ISSUE = 1;

// from C
enum E820_MAX_ENTRIES = 100;

enum E820_TYPE_USABLE = 1;

struct e820_map_64 {
  align(1):
    ulong baseaddr;
    ulong length;
    uint entry_type;
}


static void *to_address(size_t x) {
  return (cast(void *)x);
}

static size_t to_size_t(void *addr) {
  return cast(uintptr)addr;
}


struct MemStub {
  void *ptr = null;
  size_t length;
  bool freed;
}

struct MemAllocEntryNode {
  MemStub data;
  MemAllocEntryNode *next = null;
}

__gshared size_t total_length = 0;

__gshared e820_map_64[E820_MAX_ENTRIES] usable_map;
__gshared size_t usable_map_len = 0;


extern(C) void *kmalloc(size_t size);
extern(C) void kfree(void *ptr);
extern(C) void kmalloc_init(e820_map_64 *usable_map, size_t usable_map_len);

extern(C) void
pmem_map(immutable e820_map_64 *e820_map, uint e820_count)
{
	foreach (i; 0..e820_count) {
		if (e820_map[i].entry_type == E820_TYPE_USABLE) {
			usable_map[usable_map_len] = e820_map[i];
			usable_map_len++;
		}
	}
	printk("Usable map\n");
	for (size_t i = 0; i < usable_map_len; i++) {
		printk("%0lx-%0lx\n", usable_map[i].baseaddr,
			   usable_map[i].baseaddr + usable_map[i].length - 1);
		total_length += usable_map[i].length;
	}
	printk("%llu bytes available (%lluMiB)\n", total_length,
	total_length / 1024 / 1024);
  printk("Bootstrapping malloc...\n");
  kmalloc_init(usable_map.ptr, usable_map_len);
	char *s = cast(char *)kmalloc(600);
	if (s == null) {
	  panic(MEMORY_ISSUE);
	}
  cast(void)strcpy(s, "Hello");
	printk("%s (%p)\n", s, s);
	char *s2 = cast(char *)kmalloc(6);
	if (s2 == null) {
	  panic(MEMORY_ISSUE);
	}
	cast(void)strcpy(s2, "Hello");
	printk("%s (%p)\n", s2, s2);
	kfree(s2);
	kfree(s);
  // try to destroy the heap
  for (int i = 0; i < 1024; i++) {
    void *m = kmalloc(24);
    if (m == null) {
      panic(MEMORY_ISSUE);
    }
    printk("%llu\n", s.to_size_t);
    // explicitly don't free
  }
	mixin(safe_asm!("cli; hlt"));
}
