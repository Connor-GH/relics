#include <kio.h>
#include <e820.h>
#include <pmm.h>
#include <asm/wrappers.h>

struct MemAllocEntry {
  void *ptr;
  size_t length;
  struct MemAllocEntry *next;
};

static struct MemAllocEntry *alloc_table;
static size_t total_length = 0;

static void bootstrap_mem_allocator(struct e820_map_64 *usable_map) {
  alloc_table->next = alloc_table;
  if (total_length < usable_map[0].length) {
    alloc_table->length = E820_MAX_ENTRIES * total_length;
  } else {
    printk("FAILED\n");
  }
}
void pmem_map(struct e820_map_64 *e820_map, uint32_t e820_count) {
  struct e820_map_64 usable_map[E820_MAX_ENTRIES];
  size_t usable_map_len = 0;
  for (size_t i = 0; i < e820_count; i++) {
    if (e820_map[i].entry_type == E820_TYPE_USABLE) {
      usable_map[usable_map_len] = e820_map[i];
      usable_map_len++;
    }
  }
  printk("Usable map\n");
  for (size_t i = 0; i < e820_count; i++) {
    printk("%0lx-%0lx\n", usable_map[i].baseaddr, 
           usable_map[i].baseaddr+usable_map[i].length-1);
    total_length += usable_map[i].length;
  }
  printk("%llu bytes available (%lluMiB)\n", total_length, 
         total_length / 1024 / 1024);
  ASM("cli; hlt");
}
