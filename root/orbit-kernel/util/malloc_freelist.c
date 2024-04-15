/*
 * Copyright © 2017 Embedded Artistry LLC.
 * License: MIT. See LICENSE file for details.
 */

/*
 * This file was taken from the libmemory project
 * and somewhat modified for our needs:
 * https://github.com/embeddedartistry/libmemory
 */

#include "panic.h"
#include <e820.h>
#include <stdint.h>
#include <linked_list.h>
#include <kio.h>
#define __malloc_init
#include <malloc_freelist.h>
#define NULL ((void *)0)

/// By default, the freelist is declared as static so that it cannot be accessed
/// outside of the library. Users who wish to override this default declaration
/// can define `FREELIST_DECL_SPECIFIERS` to use an alternative.
/// One option is to make it an empty definition to make it publicly visible,
/// which may be useful for capturing state or performing metadata analysis.
///
/// Unless you have a specific use case, we recommend sticking with the default.
#ifndef FREELIST_DECL_SPECIFIERS
#define FREELIST_DECL_SPECIFIERS static
#endif

#pragma mark - Definitions -

/**
 * Simple macro for making sure memory addresses are aligned
 * to the nearest power of two
 */
#ifndef align_up
#define align_up(num, align) (((num) + ((align)-1)) & ~((align)-1))
#endif

/*
 * This is the container for our free-list.
 * Note the usage of the linked list here: the library uses offsetof
 * and container_of to manage the list and get back to the parent struct.
 */

typedef struct
{
	ll_t node;
	size_t size;
	char* block;
} alloc_node_t;

/**
 * We vend a memory address to the user.  This lets us translate back and forth
 * between the vended pointer and the container we use for managing the data.
 */
#define ALLOC_HEADER_SZ offsetof(alloc_node_t, block)

// We are enforcing a minimum allocation size of 32B.
#define MIN_ALLOC_SZ ALLOC_HEADER_SZ + 32

#pragma mark - Prototypes -

static void defrag_free_list(void);

#pragma mark - Declarations -

// This macro simply declares and initializes our linked list
FREELIST_DECL_SPECIFIERS LIST_INIT(free_list);

#pragma mark - Private Functions -

/**
 * When we free, we can take our node and check to see if any memory blocks
 * can be combined into larger blocks.  This will help us fight against
 * memory fragmentation in a simple way.
 */
void defrag_free_list(void)
{
	alloc_node_t* block = NULL;
	alloc_node_t* last_block = NULL;
	alloc_node_t* temp = NULL;

	list_for_each_entry_safe(block, temp, &free_list, node)
	{
		if(last_block)
		{
			if((((uintptr_t)&last_block->block) + last_block->size) == (uintptr_t)block)
			{
				last_block->size += ALLOC_HEADER_SZ + block->size;
				list_del(&block->node);
				continue;
			}
		}
		last_block = block;
	}
}

#pragma mark - APIs -

void kmalloc_init(struct e820_map_64 *usable, size_t count)
{
  // TODO will work fine when paging works above 1MiB
  for (size_t i = 0; i < count; i++) {
    malloc_addblock((void *)usable[i].baseaddr, usable[i].length);
  }
}

static void malloc_lock(void)
{
	// Intentional no-op
}

static void malloc_unlock(void)
{
	// Intentional no-op
}

void* kmalloc(size_t size)
{
	void* ptr = NULL;
	alloc_node_t* found_block = NULL;

	if(size > 0)
	{
		// Align the pointer
		size = align_up(size, sizeof(void*));

		malloc_lock();

		// try to find a big enough block to alloc
		list_for_each_entry(found_block, &free_list, node)
		{
			if(found_block->size >= size)
			{
				ptr = &found_block->block;
				break;
			}
		}

		// we found something
		if(ptr)
		{
			// Can we split the block?
			if((found_block->size - size) >= MIN_ALLOC_SZ)
			{
				alloc_node_t* new_block = (alloc_node_t*)((uintptr_t)(&found_block->block) + size);
				new_block->size = found_block->size - size - ALLOC_HEADER_SZ;
				found_block->size = size;
				list_insert(&new_block->node, &found_block->node, found_block->node.next);
			}

			list_del(&found_block->node);
		}

		malloc_unlock();

	} // else NULL

	return ptr;
}

void *kmalloc_infallible(size_t size) {
  void *p = kmalloc(size);
  if (p == NULL) {
    panic2(MEMORY_ISSUE, "kmalloc() failed to allocate.");
  }
  return p;
}

void kfree(void* ptr)
{
	// Don't free a NULL pointer..
	if(ptr)
	{
		// we take the pointer and use container_of to get the corresponding alloc block
		alloc_node_t* current_block = container_of(ptr, alloc_node_t, block);
		alloc_node_t* free_block = NULL;

		malloc_lock();

		// Let's put it back in the proper spot
		list_for_each_entry(free_block, &free_list, node)
		{
			if(free_block > current_block)
			{
				list_insert(&current_block->node, free_block->node.prev, &free_block->node);
				goto blockadded;
			}
		}
		list_add_tail(&current_block->node, &free_list);

	blockadded:
		// Let's see if we can combine any memory
		defrag_free_list();

		malloc_unlock();
	}
}

void malloc_addblock(void* addr, size_t size)
{
	// let's align the start address of our block to the next pointer aligned number
	alloc_node_t* new_memory_block = (void*)align_up((uintptr_t)addr, sizeof(void*));

	// calculate actual size - remove our alignment and our header space from the availability
	new_memory_block->size = (uintptr_t)addr + size - (uintptr_t)new_memory_block - ALLOC_HEADER_SZ;

	// and now our giant block of memory is added to the list!
	malloc_lock();
	list_add(&new_memory_block->node, &free_list);
	malloc_unlock();
}
