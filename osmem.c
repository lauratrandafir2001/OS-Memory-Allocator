// SPDX-License-Identifier: BSD-3-Clause
#include "osmem.h"
#include "helpers.h"
#define MMAP_THRESHOLD (128 * 1024)
#define PAGE_SIZE 4096
#define META_SIZE sizeof(struct block_meta)
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
// the head for our linked list
void *global_base = NULL;
struct block_meta *get_block_ptr(void *ptr) {
  	return (struct block_meta*)ptr - 1;
}
void free(void *ptr) {
  if (!ptr) {
    return;
  }
}
struct block_meta *request_space(struct block_meta *last, size_t size)
{
	struct block_meta *block;
	block = sbrk(0);
	void *request = sbrk(ALIGN(size) + sizeof(struct block_meta));
	DIE(request == NULL, "REQUEST IT'S NOT WORKING"); // Not thread safe.
	if (request == (void *)-1) 
	{	
		// sbrk failed.
		return NULL; 
	}

	if (last)
	{ // NULL on first request.
		last->next = block;
	}
	block->size = ALIGN(size);
	block->next = NULL;
	block->status = STATUS_FREE;
	return block;
}

struct block_meta *find_free_block(struct block_meta **last, size_t size)
{

	struct block_meta *current = global_base;
	while (current && !(current->size >= size && current->status == STATUS_FREE))
	{
		*last = current;
		current = current->next;
	}
	return current;
}

struct block_meta *find_last_block(struct block_meta **last)
 {

	struct block_meta *current = global_base;
	while (current->next)
	{
		*last = current;
		current = current->next;
	}
	return current;
}
void *os_malloc(size_t size)
{
	/* TODO: Implement os_malloc */
	struct block_meta *block;
	void *ptr;

	if (size == 0)
	{
		return NULL;
	}

	if (ALIGN(size + sizeof(struct block_meta)) < MMAP_THRESHOLD)
	{
		//the head of the linked_list is NULL
		if (!global_base)
		{ // First call.
			block = request_space(NULL, MMAP_THRESHOLD - sizeof(struct block_meta));
			block->status = STATUS_ALLOC;
			if (!block)
			{
				return NULL;
			}
			global_base = block;
			 if (ALIGN(block->size) - ALIGN(size) >= ALIGN(1) + ALIGN(sizeof( struct block_meta)))
				{
					size_t old_size = block->size;
					// the size of the block will be smaller
					block->size = ALIGN(size);
					struct block_meta *new;
					new =  (char*)block + ALIGN(sizeof(struct block_meta)) +  ALIGN(block->size);
					new->size = 0;
					new->next = NULL;
					if(new == NULL){
						return NULL;
					}
					new->size = old_size - block->size - sizeof(struct block_meta);
					new->status = STATUS_FREE;
					new->next = block->next;
					block->next = new;
				}
				return (block + 1);
		}
		else
		{
			struct block_meta *last = global_base;
			//coalesce
			struct block_meta *cur = last;
			while(cur->next != NULL ){					
			
			//we verify if 2 blocks next to each other are free
				if(cur->status == STATUS_FREE && cur->next->status == STATUS_FREE){	

					size_t first_size = cur->size;
					size_t second_size = cur->next->size;
					struct block_meta *tmp = cur->next;
					cur->next = cur->next->next;
					//we extend the size of the first block
					cur->size += ALIGN(second_size);
					cur->status = STATUS_FREE;
							
				}
				else{
					cur = cur->next;
				}
			}
			
			block = find_free_block(&last, ALIGN(size));
			if (!block)
			{ // Failed to find free block.
			  // verify if size it's divided by 8
			  	while(last->next != NULL){
					last = last->next;
			  	}
				if(last->status == STATUS_FREE){
					void* request = sbrk(ALIGN(size - (size_t)last->size));
					if(request == NULL || request == (void*) -1){
						return NULL;
					}
					last->size = ALIGN(size);
					last->status = STATUS_ALLOC;
					return block;
				}
				
					block = request_space(last, ALIGN(size));
					if (!block)
					{
						return NULL;
					}
					 block->status = STATUS_ALLOC;

					return (block + 1);
			}
			else
			{ // Found free block
				// TODO: consider splitting block here.
				 block->status = STATUS_ALLOC;
				 struct block_meta *last = global_base;
				 //8 is for size of a byte
				 if (ALIGN(block->size) - ALIGN(size) >= ALIGN(1) + ALIGN(sizeof( struct block_meta)))
				{
					size_t old_size = block->size;
					// the size of the block will be smaller
					block->size = ALIGN(size);
					struct block_meta *new;
					new =  (char*)block + ALIGN(sizeof(struct block_meta)) +  ALIGN(block->size);
					if(new == NULL){
						return NULL;
					}
					new->size = old_size - block->size  - sizeof(struct block_meta);
					new->status = STATUS_FREE;
					new->next = block->next;
					block->next = new;
				}
				return (block + 1);	
				
			}
		}

		return (block + 1);
	}
	if(ALIGN(size + sizeof(struct block_meta)) >= MMAP_THRESHOLD)                                                                                                                                                                                                                                                                                                                                                                                      
	{
		ptr = (void *)mmap(NULL, (ALIGN(size) + ALIGN(sizeof(struct block_meta))), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		block = (struct block_meta *)ptr;
		block->status = STATUS_MAPPED;
		block->size = ALIGN(size);
		if (ptr == MAP_FAILED)
		{
			return NULL;
		}
		return (block + 1);
	}
}

void os_free(void *ptr)
{
	/* TODO: Implement os_free */
	if (ptr == NULL)
	{
		return NULL;
	}
	struct block_meta *block = (struct block_meta *)ptr - 1; 
	if (block->status == STATUS_ALLOC)
	{
		block->status = STATUS_FREE;
		
	}
	if (block->status == STATUS_FREE)
	{
		return;
		
	}
	if (block->status == STATUS_MAPPED)
	{
		int ret = munmap(block, (block->size + META_SIZE));
		DIE(ret == -1, "ret ul nu e bun");
	}
}

void *os_calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement os_calloc */
	struct block_meta *block;
	void *ptr;
	if (size == 0)
	{
		return NULL;
	}
	if (nmemb == 0)
	{
		return 0;
	}
	

	if (ALIGN(size*nmemb) + ALIGN(sizeof(struct block_meta)) < PAGE_SIZE)
	{
		//e NULL
		if (!global_base)
		{ // First call.
			block = request_space(NULL, MMAP_THRESHOLD -  sizeof(struct block_meta));
			if (!block)
			{
				return NULL;
			}
			block->status = STATUS_ALLOC;
			global_base = block;
			 if (ALIGN(block->size) - ALIGN(nmemb * size) >= ALIGN(1) + ALIGN(sizeof( struct block_meta)))
				{
					size_t old_size = block->size;
					// the size of the block will be smaller
					block->size = ALIGN(nmemb * size);
					struct block_meta *new;
					new =  (char*)block + ALIGN(sizeof(struct block_meta)) +  ALIGN(block->size);
					new->size = 0;
					new->next = NULL;
					if(new == NULL){
						return NULL;
					}
					new->size = old_size - block->size - sizeof(struct block_meta);
					new->status = STATUS_FREE;
					new->next = block->next;
					block->next = new;
				}
				memset((block + 1), 0, size);
				return (block + 1);
			
		}
		else
		{
			struct block_meta *last = global_base;
			//coalesce
			struct block_meta *cur = last;
			while(cur->next != NULL ){					
			//we verify if 2 blocks next to each other are free
				if(cur->status == STATUS_FREE && cur->next->status == STATUS_FREE){	

					size_t first_size = cur->size;
					size_t second_size = cur->next->size;
					struct block_meta *tmp = cur->next;
					cur->next = cur->next->next;
					//we extend the size of the first block
					cur->size += ALIGN(second_size);
					cur->status = STATUS_FREE;
							
				}
				else{
					cur = cur->next;
				}
			}
			
			block = find_free_block(&last, ALIGN(nmemb* size));
			if (!block)
			{ // Failed to find free block.
			  // verify if size it's divided by 8
			  	while(last->next != NULL){
					last = last->next;
			  	}
				block = request_space(last, ALIGN(nmemb* size));
				if (!block)
				return NULL;
				block->status = STATUS_ALLOC;
				// block->size = ALIGN(size);
				memset((block + 1), 0, size);
				return (block + 1);
			}
			else
			{ // Found free block
				// TODO: consider splitting block here.
				 block->status = STATUS_ALLOC;
				 struct block_meta *last = global_base;
				 //8 is for size of a byte
				 if (ALIGN(block->size) - ALIGN(nmemb * size) >= ALIGN(1) + ALIGN(sizeof( struct block_meta)))
				{
					size_t old_size = block->size;
					// the size of the block will be smaller
					block->size = ALIGN(nmemb* size);
					struct block_meta *new;
					new =  (char*)block + ALIGN(sizeof( struct block_meta)) +  ALIGN(block->size);
					new->size = 0;
					new->next = NULL;
					if(new == NULL){
						return NULL;
					}
					new->size = old_size - block->size  - sizeof(struct block_meta);
					new->status = STATUS_FREE;
					new->next = block->next;
					block->next = new;
				}
				memset((block+1), 0, size);
				return (block + 1);
			}
		}
		memset((block+1), 0, size);
		return (block + 1);
	}
	if(ALIGN(size*nmemb) + ALIGN(sizeof(struct block_meta)) >= PAGE_SIZE)                                                                                                                                                                                                                                                                                                                                                                                      
	{
		ptr = (void *)mmap(NULL, (ALIGN(nmemb* size) + ALIGN(sizeof(struct block_meta))), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		block = (struct block_meta *)ptr;
		block->status = STATUS_MAPPED;
		block->size = ALIGN(nmemb * size);
		if (ptr == MAP_FAILED)
		{
			return NULL;
		}
		memset((block + 1), 0, nmemb* size);
		return (block + 1);
	}
	
}

void *os_realloc(void *ptr, size_t size)
{
	/* TODO: Implement os_realloc */
	 if (size == 0) {
    	return NULL;
  	}
	 if (!ptr ) {
     // NULL ptr. realloc should act like malloc.
     	return malloc(ALIGN(size));
  	 }
	 
	struct block_meta* block_ptr = get_block_ptr(ptr);
	if(block_ptr->status == STATUS_FREE){
		return NULL;
	}

	struct block_meta* block;
	if(block_ptr->status == STATUS_ALLOC){
		if(ALIGN(size) + ALIGN(sizeof(struct block_meta)) >= MMAP_THRESHOLD)                                                                                                                                                                                                                                                                                                                                                                                      
	{	
		struct block_meta* new_block = os_malloc(size);
		memcpy(new_block, block_ptr, block_ptr->size);
		free(ptr);
	}
	}
	if(block_ptr->status == STATUS_MAPPED){
	// Need to really realloc. Malloc new space and free old space.
	// Then copy old data to new space.
			void *new_ptr;
			new_ptr = os_malloc(size);
			memcpy(new_ptr, ptr, block_ptr->size +sizeof(struct block_meta));
			free(ptr);
			block = (struct block_meta *)new_ptr;
			block->status = STATUS_MAPPED;
			block->size = ALIGN(size);
			if (new_ptr == MAP_FAILED)
			{
				return NULL;
			}
			
			return (block + 1);
		
	}
	
}
