#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define HEAP_CAP 640000 // size of the heap: 640 Kilo Bytes
#define CHUNK_LIST_CAP 1024

#define UNIMPLEMENTED \
	do { \
		fprintf(stderr, "%s:%d: %s is not implemented yet\n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while (0)

typedef struct {
	char *start; // points to the start of the heap chunk
	size_t size; // size of chunk so that it can get the last of the chunk
} Chunk;

typedef struct {
	size_t count;
	Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

char heap[HEAP_CAP] = {0}; // THE "heap" in this "code environment""
// size_t heap_size = 0; // is the boundary between the allocated memory and the free memory in the HEAP commented to encourage memory manager to use the chunks which are now "free"

/* Chunk heap_alloced[HEAP_ALLOCED_CAP] = {0}; // holds the meta data of for the allocated memory from the heap
size_t heap_alloced_size = 0; // keeps track of the number of such chunks of the meta data "alloced"

Chunk heap_freed[HEAP_FREED_CAP] = {0}; // holds the meta data of for the free memory in the heap
size_t heap_freed_size = 0; // keeps track of the number of such chunks of the meta data "freed" */

Chunk_List alloced_chunks = {0}; // alternative for heap_alloced
Chunk_List freed_chunks = {
	.count = 1,
	.chunks = {
		[0] = {.start = heap, .size = sizeof(heap)}
	},
}; // alternative for heap_freed
/* [ allocated .......... | free .................... ]
	0                heap_size                    HEAP_CAP */


void /*heap_dump_alloced_chunks*/chunk_list_dump(const Chunk_List *list){
	// prints start address and size of the chunks in the heap of increasing sizes,
	// 0 and 1 have same addresses because 0 is allocated to a chunk and 0 size is added to the heap allocated size and
	// so 1 uses the same address, so 0 does not return a unique pointer
	printf("Chunks (%zu):\n", list -> count);
	for(size_t i = 0; i < list -> count; ++ i){
		printf("    start: %p, size: %zu\n", list -> chunks[i].start, list -> chunks[i].size);
	}
}

int chunk_start_compar(const void *a, const void *b){
	const Chunk *a_chunk = a;
	const Chunk *b_chunk = b;
	return a_chunk->start - b_chunk->start;
}

int chunk_list_find(const Chunk_List *list, void *ptr){
	Chunk key = {
		.start = ptr
	};

	Chunk *result = bsearch(&key, list -> chunks, list -> count, sizeof(list -> chunks[0]), chunk_start_compar);

	if(result != 0) {
		assert(list -> chunks <= result);
		return (result - list -> chunks);
	} else return -1;
}

void chunk_list_insert(Chunk_List *list, void *start, size_t size){
	assert(list -> count < CHUNK_LIST_CAP);
	list -> chunks[list ->count].start = start;// for the particular chunk handled currently, start pointer of the chunk is assigned
	list -> chunks[list ->count].size = size; // for the particular chunk handled currently, size of the chunk is assigned

	for(size_t i = list -> count; i > 0 && list -> chunks[i].start < list -> chunks[i - 1].start; -- i){
		const Chunk t = list -> chunks[i]; // getting sorted in ascending order from the last
		list -> chunks[i] = list -> chunks[i - 1];
		list -> chunks[i - 1] = t;
	}
	list -> count += 1;
}

void chunk_list_remove(Chunk_List *list, size_t index){
	assert(index < list -> count);
	for(size_t i = index; i < list -> count - 1; ++ i){
		list -> chunks[i] = list -> chunks[i + 1];
	}
	list -> count -= 1;
}

// Only two main functions needed for memory allocation
void *heap_alloc(size_t size){ // Allocates a memory in the heap

	if(size > 0){
		for(size_t i = 0; i < freed_chunks.count; ++ i){
			const Chunk chunk = freed_chunks.chunks[i];
			if(chunk.size >= size){
				chunk_list_remove(&freed_chunks, i);


				const size_t tail_size = chunk.size - size; // will never be negative cz of the >= condition so >= 0
				chunk_list_insert(&alloced_chunks, chunk.start, size);

				if(tail_size > 0) {
					chunk_list_insert(&freed_chunks, chunk.start + size, tail_size);
				}
				return chunk.start;;
			}
		}
	}
	return NULL;

/*	assert(heap_size + size <= HEAP_CAP); // check if the size we want to allocate is under max capacity, if yes continue
	void *ptr = heap + heap_size; // points to the location where there is the first free byte (heap + boundary)
	heap_size += size; // moves the heap_size forward indicating "size" bytes are allocated
	
	chunk_list_insert(&alloced_chunks, ptr, size);
	does the same as what the below part does:
	const Chunk chunk = { // meta data assignment
		.start = ptr, // for the particular chunk handled currently, start pointer of the chunk is assigned
		.size = size,  // for the particular chunk handled currently, size of the chunk is assigned
	};
	
	assert(heap_alloced_size < HEAP_ALLOCED_CAP);
	heap_alloced[heap_alloced_size++] = chunk; // the meta data for the current chunk is appended to the array of meta data(s)
	
	return ptr; // returns the location of where the current allocation just happened */

// while a malloc on the other hand handles zero by returning NULL or a unique pointer that can be successfully passed to free
}

void heap_free(void *ptr){ // Frees up the memory in the heap. will show undefined behaviour if ptr points to NULL (if 0 is passed)
	// exactly why malloc passes a unique value more often than not so it doesnt have to check for "NULL"-ity
	// also the reason why double-free of a malloced pointer shows undefined behaviour
	if(ptr){
		const int index = chunk_list_find(&alloced_chunks, ptr);
		// printf("%d\n", index);
		assert(index >= 0);
		chunk_list_insert(&freed_chunks, alloced_chunks.chunks[index].start, alloced_chunks.chunks[index].size);
		chunk_list_remove(&alloced_chunks, (size_t) index);
	}
}

void heap_collect(){ 
	// this somehow implements a rolling window of 8 bytes (a pointer is 8 bytes) checks
	// if any pointer is pointing to something in the heap and is reachable,
	// if not then it is deallocated. also checks the stack and if anything in the heap is pointed to by something
	// in the stack, if no then deallocated
	UNIMPLEMENTED;
}

int main(){

	for(int i = 0; i < 10; ++ i){
		void *p = heap_alloc(i);
		if(i % 2 == 0){
			heap_free(p);
		}
	}
	heap_alloc(420);
	for(int i = 1; i <= 4; i ++){
		heap_alloc(i);
	}
	chunk_list_dump(&alloced_chunks);
	chunk_list_dump(&freed_chunks);
	return 0;
}
