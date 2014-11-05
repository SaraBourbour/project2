// Implementation of the heap

#include "kheap.h"

#include "common.h"
#include "sorted_array.h"

// headers for local functions
void *align(void *p);
ssize_t find_smallest_hole(size_t size,
                           u8int page_align,
                           struct heap *heap)
                           WARN_UNUSED;
s8int header_less_than(void *a, void *b);
s8int heap_resize(size_t new_size, struct heap *heap) WARN_UNUSED;
void add_hole(void *start, void *end, struct heap *heap);

// returns an aligned pointer
// if the address is not aligned, then the aligned address prior to the given
// address is returned
void *align(void *p)
{
   union
   {
      void *pointer;
      size_t integer;
   } u;

   u.pointer = p;
   u.integer &= PAGE_MASK;

   return u.pointer;
}

// less than function for comparing the sizes of memory chunks, using the
// headers; a < b iff a's size is less than b's size
// a and b should both be pointers to header structs
s8int header_less_than(void *a, void *b)
{
   return (((struct header*)a)->size < ((struct header*)b)->size) ? 1 : 0;
}

// creates a heap at the given start address, end address, and maximum growth
// size
// start, end, and max should all be page-aligned (but if they aren't, we just
// waste some space)
struct heap *heap_create(void *start,
                         void *end,
                         void *max)
{
   // in the real kernel, we would kmalloc here, where kmalloc is just a
   // placement implementation, since the heap does not exist yet!
   // for the userspace projet, we can assume that the memory has already been
   // allocated
   //
   // the memory layout from start to end is as follows:
   // | heap struct | free list | actual data |
   //struct heap *heap = (struct heap*)kmalloc(sizeof(heap_t));
   struct heap *heap = (struct heap*)start;

   // create the free list
   heap->free_list = sorted_array_place((void *)start +
                                           sizeof(struct sorted_array),
                                     HEAP_FREE_LIST_SIZE,
                                     &header_less_than);

   // move the start address of the heap, to reflect where data can be place,
   // now that the free list is in the initial portion of the heap's memory
   // address space
   start += sizeof(struct sorted_array) + sizeof(void *) * HEAP_FREE_LIST_SIZE;

   // make sure the start address is page-aligned
   if(align(start) != start) {
      start = align(start) + PAGE_SIZE;
   }

   // write the avariables into the heap structure
   heap->start_address = start;
   heap->end_address = end;
   heap->max_address = max;

   // start with a large hole the size of the memory region
   add_hole(start, end, heap);

   return heap;
}

// expands or contracts the heap to the new_size
// returns a negative value on error, 0 on success
s8int heap_resize(size_t new_size, struct heap *heap)
{
   // make sure the new end address is page-aligned
   // since the heap starts on a page boundary, we need only align the size
   if(align((void *)new_size) != (void *)new_size) {
      new_size = (size_t)(align((void *)new_size)) + PAGE_SIZE;
   }

   if(new_size < heap->end_address - heap->start_address)
   {
      // contracting the heap

      // we are going to naively assume that the heap is not being resized to
      // a value that is too small

      // paging code would go here to free pages
      // for now, just assume that in our flat memory space, memory is
      // available and does not need to be allocated or freed
   }
   else if(new_size > heap->end_address - heap->start_address)
   {
      // expanding the heap

      // make sure the new size is within the bounds
      if(heap->start_address + new_size > heap->max_address) {
         // not within bounds
         return -1;
      }

      // paging code would be here to allocate pages
      // for now, just assume that in our flat memory space, memory is
      // available and does not need to be allocated or freed
   }
   else
   {
      // same size - do nothing
   }

   // set the size of the heap
   heap->end_address = heap->start_address + new_size;

   return 0;
}

// find the smallest hole that will fit the requested size
// if a hole is found, the index in the heap free list is returned
// if a hole is not found, then -1 is returned
// size must include the size of the header and footer, in addition to the
// size that the actual users wishes to request
ssize_t find_smallest_hole(size_t size,
                           u8int page_align,
                           struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 1: iterate over free list
   // 2: see if the chunk is large enough for the requested size - remember to
   //    include page alignment!
   // 3: if large enough, return
   // 4: if not large enough, continue to the next chunk in the iteration
   // 5: if the end is reached before a chunk is found, return -1
	

	// Find the smallest hole that will fit.
   size_t iterator = 0;
   while (iterator < heap->free_list.size)
   {
       struct header *header = (struct header *)sorted_array_lookup(iterator, &heap->free_list);
       // If the user has requested the memory be page-aligned
       if (page_align > 0)
       {
           // Page-align the starting point of this header.
           size_t location = (size_t)header;
           ssize_t offset = 0;
           if ((location+sizeof(struct header)) & (0xFFFFF000 != 0))
               offset = 0x1000 /* page size */  - (location+sizeof(struct header))%0x1000;
           ssize_t hole_size = (ssize_t)header->size - offset;
           // Can we fit now?
           if (hole_size >= (ssize_t)size)
               break;
       }
       else if (header->size >= size)
           break;
       iterator++;
   }
   // Why did the loop exit?
   if (iterator == heap->free_list.size)
       return -1; // We got to the end and didn't find anything.
   else
       return iterator;


   return 0;
}

// creates and writes a hole that spans [start,end)
void add_hole(void *start, void *end, struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 0. determine if coalesing is possible; if so, remove the appropriate
   //    holes on either side of start and end, and then call add_hole
   //    recursively on the larger region (can skip if no coalesing)
   // 1. write header and footer to memory
   // 2. add chunk to free list
	
    	struct header *myheader = (struct header*) start;
	myheader->size = end-start;
	myheader->magic = HEAP_MAGIC;
	myheader->allocated = 0; //*
	sorted_array_insert(myheader, &heap->free_list);
	
/*	myheader->size = end - start;
	struct footer *myfooter = (struct footer) malloc(1);
	myfooter->header = myheader;
	heap->free_list.sorted_array_insert(address, &heap->free_list);
*/
}

void *kalloc_heap(size_t size, u8int page_align, struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 1. figure out size of needed free list entry (add size of header and
   //    footer with sizeof(struct header) / sizeof(struct footer))
   // 2. find a hole to allocate using find_smallest_hole
   // 3. if no hole found, resize the heap, and then start again at 2
   // 4. remove the found hole from the free list to use for allocation
   // 5. page-align, if necessary
   // 5.1. determine if page-alignment makes a good hole before our
   //      allocation; if so, add that hole
   // 6. mark the chunk as allocated, write the header/footer (if necessary)
   // 7. return pointer to allocated portion of memory
	

	// Make sure we take the size of header/footer into account.

   size_t new_size = size + sizeof(struct header) + sizeof(struct footer);
   // Find the smallest hole that will fit.
   ssize_t iterator = find_smallest_hole(new_size, page_align, heap);
   
 if (iterator == -1) // If we didn't find a suitable hole
   {
       // Save some previous data.
       size_t old_length = heap->end_address - heap->start_address;
       size_t old_end_address = heap->end_address;

       // We need to allocate some more space.
       heap_resize(old_length+new_size, heap);
       size_t new_length = heap->end_address-heap->start_address;

       // Find the endmost header. (Not endmost in size, but in location).
       iterator = 0;
       // Vars to hold the index of, and value of, the endmost header found so far.
       size_t idx = -1; size_t value = 0x0;
       while (iterator < heap->free_list.size)
       {
           size_t tmp = (size_t)sorted_array_lookup(iterator, &heap->free_list);
           if (tmp > value)
           {
               value = tmp;
               idx = iterator;
           }
           iterator++;
       }

       // If we didn't find ANY headers, we need to add one.
       if (idx == -1)
       {
           struct header *header = (struct header *)old_end_address;
           header->magic = HEAP_MAGIC;
           header->size = new_length - old_length;
           header->allocated  = 0;//*
           struct footer *footer = (struct footer *) (old_end_address + header->size - sizeof(footer));
           footer->magic = HEAP_MAGIC;
           footer->header = header;
           sorted_array_insert((void*)header, &heap->free_list);
       }
       else
       {
           // The last header needs adjusting.
           struct header *header = sorted_array_lookup(idx, &heap->free_list);
           header->size += new_length - old_length;
           // Rewrite the footer.
           struct footer *footer = (struct footer *) ( (size_t)header + header->size - sizeof(footer) );
           footer->header = header;
           footer->magic = HEAP_MAGIC;
       }
       // We now have enough space. Recurse, and call the function again.
       return kalloc_heap(size, page_align, heap);
   } 


	struct header *orig_hole_header = (struct header *)sorted_array_lookup(iterator, &heap->free_list);
   size_t orig_hole_pos = (size_t)orig_hole_header;
   size_t orig_hole_size = orig_hole_header->size;

   // Here we work out if we should split the hole we found into two parts.
   // Is the original hole size - requested hole size less than the overhead for adding a new hole?
   if (orig_hole_size-new_size < sizeof(struct header)+sizeof(struct footer))
   {
       // Then just increase the requested size to the size of the hole we found.
       size += orig_hole_size-new_size;
       new_size = orig_hole_size;
   } 


	// If we need to page-align the data, do it now and make a new hole in front of our block.
   if (page_align && orig_hole_pos&0xFFFFF000)
   {
       size_t new_location   = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(struct header);
       struct header *hole_header = (struct header *)orig_hole_pos;
       hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos&0xFFF) - sizeof(struct header);
       hole_header->magic    = HEAP_MAGIC;
       hole_header->allocated  = 0; //*
       struct footer *hole_footer = (struct footer *) ( (size_t)new_location - sizeof(struct footer) );
       hole_footer->magic    = HEAP_MAGIC;
       hole_footer->header   = hole_header;
       orig_hole_pos         = new_location;
       orig_hole_size        = orig_hole_size - hole_header->size;
   }
   else
   {
       // Else we don't need this hole any more, delete it from the index.
       sorted_array_remove(iterator, &heap->free_list);
   } 

	// Overwrite the original header...
   struct header *block_header  = (struct header *)orig_hole_pos;
   block_header->magic     = HEAP_MAGIC;
    block_header->allocated    = 1; //*
   block_header->size      = new_size;
   // ...And the footer
   struct footer *block_footer  = (struct footer *) (orig_hole_pos + sizeof(struct header) + size);
   block_footer->magic     = HEAP_MAGIC;
   block_footer->header    = block_header; 


	   // We may need to write a new hole after the allocated block.
   // We do this only if the new hole would have positive size...
   if (orig_hole_size - new_size > 0)
   {
       struct header *hole_header = (struct header *) (orig_hole_pos + sizeof(struct header) + size + sizeof(struct footer));
       hole_header->magic    = HEAP_MAGIC;
       hole_header->allocated   = 0; //*
       hole_header->size     = orig_hole_size - new_size;
       struct footer *hole_footer = (struct footer *) ( (size_t)hole_header + orig_hole_size - new_size - sizeof(struct footer) );
       if ((size_t)hole_footer < heap->end_address)
       {
           hole_footer->magic = HEAP_MAGIC;
           hole_footer->header = hole_header;
       }
       // Put the new hole in the index;
       sorted_array_insert((void*)hole_header, &heap->free_list);
   } 
    // ...And we're done!
   return (void *) ( (size_t)block_header+sizeof(struct header) );
} 

void kfree_heap(void *p, struct heap *heap)
{
   // TODO: IMPLEMENT THIS FUNCTION

   // pseudocode:
   // 1. check for a null pointer before proceeding
   // 2. get the header and the footer based on the passed pointer
   // 3. mark the chunk as free in the header
   // 4. add a hole in the space that was previously allocated

// Exit gracefully for null pointers.
   if (p == 0)
       return;

   // Get the header and footer associated with this pointer.
   struct header *header = (struct header*) ( (size_t)p - sizeof(struct header) );
   struct footer *footer = (struct footer*) ( (size_t)header + header->size - sizeof(struct footer) );

   // Sanity checks.
   //ASSERT(header->magic == HEAP_MAGIC);
   //ASSERT(footer->magic == HEAP_MAGIC);
 // Make us a hole.
   header->allocated = 0;//*

   // Do we want to add this header into the 'free holes' index?
   char do_add = 1;
  // If the thing immediately to the left of us is a footer...
   struct footer *test_footer = (struct footer*) ( (size_t)header - sizeof(struct footer) );
   if (test_footer->magic == HEAP_MAGIC &&
       test_footer->header->allocated == 0) //*
   {
       size_t cache_size = header->size; // Cache our current size.
       header = test_footer->header;     // Rewrite our header with the new one.
       footer->header = header;          // Rewrite our footer to point to the new header.
       header->size += cache_size;       // Change the size.
       do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
   }
   struct header *test_header = (struct header*) ( (size_t)footer + sizeof(struct footer) );
   if (test_header->magic == HEAP_MAGIC &&
       test_header->allocated == 0) //*
   {
       header->size += test_header->size; // Increase our size.
       test_footer = (struct footer*) ( (size_t)test_header + // Rewrite it's footer to point to our header.
                                   test_header->size - sizeof(struct footer) );
       footer = test_footer;
       // Find and remove this header from the index.
       size_t iterator = 0;
       while ( (iterator < heap->free_list.size) &&
               (sorted_array_lookup(iterator, &heap->free_list) != (void*)test_header) )
           iterator++;

       // Make sure we actually found the item.
       //ASSERT(iterator < heap->free_list.size);
       // Remove it.
       sorted_array_remove(iterator, &heap->free_list);
   }
    // If the footer location is the end address, we can contract.
   if ( (size_t)footer+sizeof(struct footer) == heap->end_address)
   {
       size_t old_length = heap->end_address-heap->start_address;
       size_t new_length = heap_resize( (size_t)header - (size_t)heap->start_address, (void*)heap); //*
       // Check how big we will be after resizing.
       if (header->size - (old_length-new_length) > 0)
       {
           // We will still exist, so resize us.
           header->size -= old_length-new_length;
           footer = (struct footer*) ( (size_t) header + header->size - sizeof(struct footer) );
           footer->magic = HEAP_MAGIC;
           footer->header = header;
       }
       else
       {
           // We will no longer exist :(. Remove us from the index.
           size_t iterator = 0;
           while ( (iterator < heap->free_list.size) &&
                   (sorted_array_lookup(iterator, &heap->free_list) != (void*)test_header) )
               iterator++;
           // If we didn't find ourselves, we have nothing to remove.
           if (iterator < heap->free_list.size)
               sorted_array_remove(iterator, &heap->free_list);
       }
   }
if (do_add == 1)
  sorted_array_insert((void*) header, &heap->free_list); 







}
