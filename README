# Project Memory Allocator

# About
Operating Systems course
https://ocw.cs.pub.ro/courses/so

April 2023

Student:  Trandafir Laura;

# Implementation:

# void *os_malloc(size_t size):  Allocates `size` bytes and returns a pointer to the allocated memory.
-base cases: if the size is NULL we return NULL;
-we verify if the size is smaller than MMAP_TRESHOLD
    -if the global_base is NUL:
    and there haven't been allocated any blocks, we allocate the first block(as the head)
    of a linked list, using request_space, a function that is given in the documentation and sets the status of the block on 
    STATUS_ALLOC. We also verify if we can SPLIT the first node of the linked list and return it;
    -if the global_base is not NULL:
    we make the COALESCE operation, go through the entire list and verify if there are free blocks next to each other
    if there are free blocks, from 2 free blocks of memory we make one, by setting the size of the first block equal to the sum
    of the first and second block sizes, and also setting the pointers;
    we try to find a free block using the find_free_block function, also from documentation;
    if we don't find a free block with the size that we need we try to do EXPAND, meaning that we allocate more
    memory to the last free block from the list;
    if we find a free block we consider SPLITTING it, meaning if the block has a bigger size than what we need,
    we split it into 2 separate blocks, one with the size that we need and the next one with the rest of the memory left;
-we verify if the size is bigger or equal to the MMAP_TRESHOLD:
    -we allocate memory using mmap;
    -we we set the status of the block on STATUS_MAPPED and the size on the ALIGN(size);

# void *os_free(void *ptr):  Frees memory previously allocated by `os_malloc()`, `os_calloc()` or `os_realloc()`.
-base cases: if the ptr is NULL we return NULL;
-if the block that the pointers point to has the status STATUS_ALLOC we change the status to free
-if the block has the status FREE, we do nothing
-if the block that the pointers point to has the status STATUS_MAPPED, we use munmap to deallocate the memory


# void *os_calloc(size_t nmemb, size_t size): Allocates memory for an array of `nmemb` elements of `size` bytes each and returns a pointer to the allocated memory.
-base cases: if the size is 0 we return NULL, and if nmemb is 0 we return 0
-the implementation is the same with os_malloc only before we return a block we set
the memory with memset
-we verify if the size is smaller than PAGE_SIZE
    -if the global_base is NUL:
    and there haven't been allocated any blocks, we allocate the first block(as the head)
    of a linked list, using request_space, a function given in the documentation and sets the status of the block on 
    STATUS_ALLOC. We also verify if we can SPLIT the first node of the linked list and return it;
    -if the global_base is not NULL:
    we make the COALESCE operation,  go through the entire list and verify if there are free blocks next to each other
    if there are 2 free blocks, from 2 free blocks of memory we make one, by setting the size of the first block equal to the sum
    of the first and second block sizes, and also setting the pointers;
    we try to find a free block using find_free_block function, also from documentation;
    if we don't find a free block with the size that we need we try to do EXPAND, meaning that we allocate more
    memory to the last free block from the list;
    if we find a free block we consider SPLITTING it, meaning if the block has a bigger size than what we need,
    we split it into 2 separate blocks, one with the size that we need and the next one with the rest of the memory left;
-we verify if the size is bigger or equal to the PAGE_SIZE:
    -we allocate memory using mmap;
    -we we set the status of the block on STATUS_MAPPED and the size on the ALIGN(size);

# void *os_realloc(size_t nmemb, size_t size): Changes the size of the memory block pointed to by `ptr` to `size` bytes.
If the size is smaller than the previously allocated size, the memory block will be truncated.
-base cases: if the size is 0 we return 0 and if the pointer is NULL we malloc memory with os_malloc



References:
- ["Implementing malloc" slides by Michael Saelee](https://moss.cs.iit.edu/cs351/slides/slides-malloc.pdf)
- [Malloc Tutorial](https://danluu.com/malloc-tutorial/)



