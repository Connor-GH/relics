#include <panic.h>
#include <paging.h>
#include <cpu.h>
#include <kio.h>
#include <orbit.h>
#include <stdint.h>

#define PAGE_DIR_BIT_PRESENT (1 << 0) // present flag
#define PAGE_DIR_BIT_RW (1 << 1) // read / write
#define PAGE_DIR_BIT_USER (1 << 2)
#define PAGE_DIR_BIT_SUPERVISOR (0 << 2)
#define PAGE_DIR_BIT_PWT (1 << 3) // write-through
#define PAGE_DIR_BIT_PCD (1 << 4) // cache disable
#define PAGE_DIR_BIT_ACCESS (1 << 5)
#define PAGE_DIR_BIT_DIRTY (1 << 6)
#define PAGE_DIR_BIT_PAGE_SZ (1 << 7)
#define PAGE_DIR_BIT_GLOBAL (1 << 8)
#define PAGE_DIR_BIT_AVL (3 << 9)
#define PAGE_DIR_BIT_PAT (1 << 12)
#define PAGE_DIR_EXECUTE_DISABLE (1UL << 63)

static uint64_t page_directory[512] ATTR(aligned(4096));

static void check_requirements(void) {
  u32 a, b, c, d;
  a = b = c = d = 0;
  cpuid_call(CPUID_EAX_GETFEATURES, 0, &a, &b, &c, &d);

  if ((d & CPUID_FEAT_EDX_PAT) == 0) {
    panic2(CPU_ISSUE, "CPU does not support PAT");
  }
  if ((d & CPUID_FEAT_EDX_PSE) == 0) {
    panic2(CPU_ISSUE, "CPU does not support PSE");
  }
  if ((d & CPUID_FEAT_EDX_PSE36)) {
    log_printk("CPU supports 36 bits of PSE\n");
  }
}

static void blank_page_dir(void) {
  for (int i = 0; i < 512; i++) {
    // Write Enabled: It can be both read from and written to
    // Not Present: The page table is not present
    page_directory[i] = PAGE_DIR_BIT_RW | PAGE_DIR_BIT_PAT | PAGE_DIR_BIT_SUPERVISOR | PAGE_DIR_BIT_PRESENT;
  }
}

// A page is "not present" when it's not (intended to be) used. If the MMU finds one, it will Page Fault. Non-present pages are useful for techniques such as Lazy Loading. It's also used when a page has been swapped to disk, so the Page Fault is not interpreted as an error by the OS. To the OS, it means someone needs a page it swapped to disk, so it is restored. A page fault over a page that was never swapped is a error by which the OS has a reason to kill the process.

//The second step is to create a basic page table. In this example we choose to fill up the whole first page table with addresses for the MMU. Because each page is 4 kilobytes large, and because each page table has exactly 1024 entries, filling up the whole table causes us to map 4 megabytes of memory. Also, the page directory is 1024 entries long, so everything can map up to 4GiB, the full 32-bit address space. Remembered the non-present page trick? Without it, we would use 16MiB per each paging structure. A single page directory needs 4KiB, but it can map some tables as non-present, effectively removing their space needs.


static uint64_t page_directory_ptr[512] ATTR(aligned(4096));

//We now need to fill each index in the table with an address to which the MMU will map that page. Index 0 (zero) holds the address from where the first page will be mapped. Likewise, index 1 (one) holds the address for the second page and index 1023 holds the address of the 1024th page. That's for the first table. So, to get the page at which a certain index is mapped is as simple as (PageDirIndexOfTable * 1024) + PageTabIndexOfPage. If you multiply that by 4, you'll get the address (in KiB) at which the page will be loaded. For example, page index 123 in table index 456 will be mapped to (456 * 1024) + 123 = 467067. 467067 * 4 = 1868268 KiB = 1824.48046875 MiB = 1.781719207763671875 GiB. It's easy, right?

// holds the physical address where we want to start mapping these pages to.
// in this case, we want to map these pages to the very beginning of memory.
//we will fill all 1024 entries in the table, mapping 4 megabytes
static void fill_pdp(void) {
  for (uint64_t i = 0; i < 512; i++) {
    // As the address is page aligned, it will always leave 12 bits zeroed.
    /* TODO why are we setting this 1 << 20 nonsense? */
    page_directory_ptr[i] = (i << 30) | (1 << 20)  | PAGE_DIR_BIT_SUPERVISOR | PAGE_DIR_BIT_RW | PAGE_DIR_BIT_PRESENT;

  }
  // Put the Page Table in the Page Directory
  // The third step is to put the newly created page table into our blank page directory. We do this by setting the first entry in the page directory to the address of our page table.
  // page_directory[0] = ((uintptr_t)page_directory_ptr) | PAGE_DIR_BIT_SUPERVISOR | PAGE_DIR_BIT_RW | PAGE_DIR_BIT_PRESENT;
}

// page map level 4
static uint64_t pml4[512] ATTR(aligned(4096));
static void fill_pml4(void) {
  for (uint64_t i = 0; i < 512; i++) {
    pml4[i] = (i << 12) | (1 << 20) | (1 << 12) | PAGE_DIR_BIT_RW | PAGE_DIR_BIT_PRESENT;
  }
}


//This small assembly function takes one parameter: the address of the page directory. It then loads the address onto the CR3 register, where the MMU will find it. But wait! Paging is not still enabled. That's what we will do next. We must set the 32th bit in the CR0 register, the paging bit. This operation also requires assembly code. Once done, paging will be enabled.



// This should go outside any function..
extern void load_page_dir(uint64_t *);
extern void enable_paging(void);
// And this inside a function
void paging_init(void) {
  check_requirements();
  blank_page_dir();
  fill_pdp();
  fill_pml4();
  load_page_dir(pml4);
  enable_paging();
  log_printk("Paging initialized!\n");
}
