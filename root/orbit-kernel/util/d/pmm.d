/*#include <kio.h>
#include <e820.h>
#include <pmm.h>
#include <asm/wrappers.h>
#include <panic.h>
#include <string.h>
#include <stdbool.h>
#include <orbit.h>*/

module pmm;

import std.safe_asm;


@nogc nothrow:
@trusted extern(C) __gshared void printk(const char *fmt, ...);
@trusted extern(C) __gshared void log_printk(const char *fmt, ...);
@trusted extern(C) __gshared void panic(int issue);
@trusted extern(C) __gshared void *strcpy(char *dst, const char *src);

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

static size_t to_address(size_t x) {
  return (cast(void *)x);
}

static size_t to_size_t(void *addr) {
  return *cast(size_t *)addr;
}

struct MemAllocEntry {
	void *ptr;
	size_t length;
	MemAllocEntry *next;
  ulong freed; // 64 bits due to padding
}

__gshared MemAllocEntry *alloc_table;
__gshared MemAllocEntry *m;
// [ points at self]
// [ .next = points at alloc'd memory, length = length of allocation]
__gshared size_t total_length = 0;

__gshared e820_map_64[E820_MAX_ENTRIES] usable_map;
__gshared size_t usable_map_len = 0;

// needs NOOPT to compile with optimizations
static void
bootstrap_mem_allocator()
{
	// make one alloc entry, with our table in it
  alloc_table = cast(MemAllocEntry *)(cast(void *)usable_map[0].baseaddr);
	alloc_table.ptr = cast(void *)alloc_table; // this line is broken with optimizations
  if ((MemAllocEntry *).sizeof < usable_map[0].length) {
		alloc_table.length = (MemAllocEntry *).sizeof;
    alloc_table.freed = false;
		alloc_table.next = null;
    size_t addr = 0;
    for (MemAllocEntry *entry = alloc_table; addr < usable_map[0].length; addr += (MemAllocEntry *).sizeof) {
      MemAllocEntry *temp;
      temp.length = (MemAllocEntry *).sizeof;
      temp.freed = true;
      entry.next = temp;
      entry = entry.next; 
      addr += temp.length;
    }
		printk("success!\n");
	} else {
		printk("FAILED\n");
		panic(MEMORY_ISSUE);
	}
}


__gshared size_t last_size = 0;
__gshared void *fifo_stack;

static void bootstrap_malloc_fifo() {
  fifo_stack = cast(void *)to_address(usable_map[0].baseaddr + 4096);
}
static void *kmalloc_fifo(size_t size) {
  if (to_size_t(fifo_stack) < (usable_map[0].baseaddr+usable_map[0].length)) {
    fifo_stack = cast(void *)(cast(ubyte *)fifo_stack + size);
    last_size = size;
    return cast(void *)(cast(ubyte *)fifo_stack - size);
  }
  return null;
}
static void kfree_fifo(void *ptr) {
  if (ptr == null)
    return;
  fifo_stack = cast(void *)(cast(ubyte *)fifo_stack + last_size);

}
static void *
kmalloc(size_t size)
{
	for (size_t i = 1; i < usable_map_len; i++) {
		if (size < usable_map[i].length) {
			MemAllocEntry *mem;
			m = alloc_table;
      void *alloc_ptr = m.ptr;
			size_t offset = m.length;
			printk("Current ptr and length: %lx and %llu\n", to_size_t(&alloc_ptr), offset);
      log_printk("Entering loop\n");
      while (m.next != null)  {
			  log_printk("m: %lx, m->next: %lx\n", m, m.next);
        m = m.next;
			}
        // pointers are fun :^)
        //  ptr -> data
        //  addr -> ptr
			log_printk("Deref ptr\n");
      alloc_ptr = cast(ubyte *)(m.ptr);
			log_printk("Get length\n");
			offset = m.length;
			mem.length = size + (MemAllocEntry *).sizeof;
			mem.next = null;
			mem.ptr = alloc_ptr + offset;
			m.next = mem;
	    log_printk("Returning malloc with size %llu and addr %lx\n", size, *cast(size_t *)&mem.ptr);
			return mem.ptr + (MemAllocEntry *).sizeof;
		}
	}
	printk("Returning null\n");
	return null;
}


static void
kfree(void * ptr)
{
	MemAllocEntry *before;
	for (MemAllocEntry *me = alloc_table; me.next != null; before = me) {
		m = m.next;
		if (m.ptr == ptr) {
			before.next = m; // remove member from linked list
		}
	}
}

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
	printk("Bootstrapping memory allocator...");
	//bootstrap_malloc_fifo();
  bootstrap_mem_allocator();
	printk("Testing memory allocator...\n");
	char *s = cast(char *)kmalloc(600);
	if (s == null) {
	  panic(MEMORY_ISSUE);
	}
  cast(void)strcpy(s, "Hello");
	printk("%s (%llu)\n", s, *cast(size_t *)&s);
	kfree(s);
	char *s2 = cast(char *)kmalloc(6);
	cast(void)strcpy(s2, "Hello");
	printk("%s (%llu)\n", s, *cast(size_t *)&s2);
	kfree(s2);
	mixin(safe_asm!("cli; hlt"));
}
