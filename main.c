#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define HEAP_CAP 640000 // size of the heap: 640 Kilo Bytes
#define HEAP_ALLOCED_CAP 1024
#define HEAP_FREED_CAP 1024

typedef struct{
	void* start; // points to the start of the heap chunk
	size_t size; // size of chunk so that it can get the last of the chunk
} Heap_Chunk;

char heap[HEAP_CAP] = {0}; // THE "heap" in this "code environment""
size_t heap_size = 0; // is the boundary between the allocated memory and the free memory in the HEAP

Heap_Chunk heap_alloced[HEAP_ALLOCED_CAP] = {0}; // holds the meta data of for the allocated memory from the heap
size_t heap_alloced_size = 0; // keeps track of the number of such chunks of the meta data "alloced"

Heap_Chunk heap_freed[HEAP_FREED_CAP] = {0}; // holds the meta data of for the free memory in the heap
size_t heap_freed_size = 0; // keeps track of the number of such chunks of the meta data "freed"
/* [ allocated .......... | free .................... ]
	0                heap_size                    HEAP_CAP */

// Only two main functions needed for memory allocation
void *heap_alloc(size_t size){ // Allocates a memory in the heap

	if(size > 0) {

		assert(heap_size + size <= HEAP_CAP); // check if the size we want to allocate is under max capacity, if yes continue
		void *result = heap + heap_size; // points to the location where there is the first free byte (heap + boundary)
		heap_size += size; // moves the heap_size forward indicating "size" bytes are allocated
		
		const Heap_Chunk chunk = { // meta data assignment
			.start = result, // for the particular chunk handled currently, start pointer of the chunk is assigned
			.size = size,  // for the particular chunk handled currently, size of the chunk is assigned
		};
		
		assert(heap_alloced_size < HEAP_ALLOCED_CAP);
		heap_alloced[heap_alloced_size++] = chunk; // the meta data for the current chunk is appended to the array of meta data(s)
		return result; // returns the location of where the current allocation just happened

	} else return NULL;




// while a malloc on the other hand handles zero by returning NULL or a unique pointer that can be successfully passed to free
}

void heap_dump_alloced_chunks(void){
	// prints start address and size of the chunks in the heap of increasing sizes,
	// 0 and 1 have same addresses because 0 is allocated to a chunk and 0 size is added to the heap allocated size and
	// so 1 uses the same address, so 0 does not return a unique pointer
	printf("Allocated chunks (%zu):\n", heap_alloced_size);
	for(size_t i = 0; i < heap_alloced_size; ++ i){

		printf("    start: %p, size: %zu\n", heap_alloced[i].start, heap_alloced[i].size);
	}
}

void heap_free(void *ptr){ // Frees up the memory in the heap
	(void) ptr;
	assert(false && "TODO: heap_free is not implemented");
}

void heap_collect(){ // this somehow implements a rolling window of 8 bytes (a pointer is 8 bytes) checks if any pointer is pointing to something in the heap and is reachable, if not then it is deallocated. also checks the stack and if anything in the heap is pointed to by something in the stack, if no then deallocated
	assert(false && "TODO: heap_collect is not implemented");
}

int main(void){

	for(int i = 0; i < 100; ++ i){
		heap_alloc(i);
	}

	heap_dump_alloced_chunks();

//	heap_free(root);
	return 0;
}
