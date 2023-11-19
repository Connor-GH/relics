#include <stddef.h>
#include <stdbool.h>
#include <memory.h>
#include <kio.h>

#define END_OF_MEM (0x1CE8FFFF)
#define MEMSTART (0xC0000)

enum MEM_PAGE {
	MEM_UNUSABLE = 0,
	MEM_USABLE,
	MEM_RESERVED,
};

static size_t j = END_OF_MEM-MEMSTART; // bytes of memory available

struct mem_page {
	enum MEM_PAGE usable;// = MEM_USABLE;
	void *beginning_ptr;
	void *end_ptr;
};


static struct mem_page *bootstrap_mem;
static struct mem_page *pages[] = {0};
static void bootstrap_malloc(void) {
	struct mem_page *page = (void *)(MEMSTART);
	page->usable = MEM_RESERVED;
	page->beginning_ptr = (void *)MEMSTART;
	page->end_ptr = (void *)(MEMSTART+4096);
	bootstrap_mem = page;
}
uint32_t init_all_memory(void) {
    /*
     * Detect the amount of ram the system has.
     */
    /* Talk to CMOS to get amount of ram */
    uint8_t lowmem, highmem;
    outb(0x70,0x30);
    lowmem = inb(0x71);
    outb(0x70, 0x31);
    highmem = inb(0x71);

    return lowmem | highmem << 8;
}

int all_mem_available_bytes(void) {
    return j;
}

void mem_into_pages(void) {
	bootstrap_malloc();
	pages[0] = bootstrap_mem;
	size_t j = 1;
	size_t i = 4096;
	while ((i < END_OF_MEM) && (j < 4095)) {
		struct mem_page *temp = (void *)(MEMSTART + j * 30);
		temp->usable = MEM_USABLE;
		temp->beginning_ptr = (void *)(MEMSTART+i);
		temp->end_ptr = (void *)(MEMSTART+i+4096);
		pages[j] = temp;
		i+=4096;
		j++;
	}
}

void *grab_good_page(void) {
	for (int i = 0; pages[i]; i++) {
		if (pages[i]->usable == MEM_USABLE) {
			pages[i]->usable = MEM_UNUSABLE;
			return (void *)&pages[i]->beginning_ptr;
		}
	}
	return NULL;
}
static void *malloc_4096_bytes_or_less(size_t size) {
	void *good_page = grab_good_page();
	if (good_page == NULL) {
		return NULL;
	}
	return good_page;

}
void *malloc(size_t size) {
	if (size < 4096)
		return malloc_4096_bytes_or_less(size);
	else
		return NULL;
}
void free(void *ptr) {
	if (ptr == NULL) {
		printk("Tried to free NULL pointer\n");
		return;
	}
	for (int i = 0; pages[i]; i++) {
		if (ptr == &pages[i]->beginning_ptr) {
			pages[i]->usable = MEM_USABLE;
			return;
		}
	}
	printk("Invalid free issue\n");
}
