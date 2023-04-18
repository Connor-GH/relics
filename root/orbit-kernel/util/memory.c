#include <stddef.h>
#include <memory.h>
#include <kio.h>

#define END_OF_MEM (0x1E8FFFF)
#define MEMSTART (0xC0000)
static unsigned int j = 0; // bytes of memory available

static size_t mem_current_ptr = MEMSTART;
static size_t last_size_allocated = 0;
static size_t MEMALLOCEND = END_OF_MEM; // end of vga console


__attribute__((optnone))
static unsigned int calculate_end_of_mem(void) {
    //int32_t undefined_mem;
    //void *undefined_mem_ptr = &undefined_mem;
    uint8_t *currentaddr = (uint8_t *)0x1;
    uint32_t counter = 0;
    while (currentaddr != (void *)0x0/*undefined_mem_ptr*/) {
        currentaddr++;
        counter++;
        printk("%u\t%d\n", (uint32_t)*currentaddr, counter);
    }
    printk("Results: %u, %d\n, ",
    (uint32_t) *currentaddr,
    counter);
    return *currentaddr;
}
void init_all_memory(void) {
    /*
     * The OS is currently hardcoded
     * to only initialize 32M (29M) of memory.
     * there will be a system later implemented to
     * detect the amount of ram the system has,
     * and initialize all of it.
     */
    calculate_end_of_mem();
}

int all_mem_available_bytes(void) {
    return j;
}
void *malloc(size_t size) {
    size_t current_alloc_end = mem_current_ptr + size;

    if (current_alloc_end >= END_OF_MEM)
        return NULL; /* TODO return ENOMEM */

    for (size_t i = 0; i < size; i++)
        *(size_t *)(mem_current_ptr + i) = '0';

    *(size_t *)(mem_current_ptr + size) = '\0';

    mem_current_ptr += size;
    last_size_allocated = size;
    return (void *)(mem_current_ptr);
}

void free(void *ptr) {

    for (size_t i = last_size_allocated; i > 0; i--)
        *(char *)((char *)ptr - i) = '\0';
    mem_current_ptr -= last_size_allocated;
}
