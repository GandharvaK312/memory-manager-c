#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define HEAP_CAP 640000
#define CHUNK_LIST_CAP 1024

#define UNIMPLEMENTED \
	do { \
		fprintf(stderr, "%s:%d: %s is not implemented yet\n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while (0)

typedef struct {
	char *start;
	size_t size;
} Chunk;

typedef struct {
	size_t count;
	Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

char heap[HEAP_CAP] = {0};

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {
	.count = 1,
	.chunks = {
		[0] = {.start = heap, .size = sizeof(heap)}
	},
}; // alternative for heap_freed

void chunk_list_insert(Chunk_List *list, void *start, size_t size){
	assert(list -> count < CHUNK_LIST_CAP);
	list -> chunks[list ->count].start = start;// appending the chunk metadata
	list -> chunks[list ->count].size = size; // holding the size for it

	for(size_t i = list -> count; i > 0 && list -> chunks[i].start < list -> chunks[i - 1].start; -- i){
		// starts sorting in ascending order from end
		const Chunk t = list -> chunks[i];
		list -> chunks[i] = list -> chunks[i - 1];
		list -> chunks[i - 1] = t;
	}
	list -> count += 1;
}

void chunk_list_merge( Chunk_List *dst, const Chunk_List *src){
	dst -> count = 0;
	for(size_t i = 0; i < src -> count; ++ i){
		const Chunk chunk = src -> chunks[i];
		
		if(dst -> count > 0){
			Chunk *top_chunk = &dst -> chunks[dst -> count - 1];
			if(top_chunk->start + top_chunk->size == chunk.start){
				top_chunk->size += chunk.size;
			} else {
				chunk_list_insert(dst, chunk.start, chunk.size);
			}
		} else {
			chunk_list_insert(dst, chunk.start, chunk.size);
		}
	}
}

void chunk_list_dump(const Chunk_List *list){
	printf("Chunks (%zu):\n", list -> count);
	for(size_t i = 0; i < list -> count; ++ i){
		printf("    start: %p, size: %zu\n", list -> chunks[i].start, list -> chunks[i].size);
	}
}

int chunk_start_compar(const void *a, const void *b){
	const Chunk *a_chunk = a;
	const Chunk *b_chunk = b;
	if (a_chunk->start < b_chunk->start) return -1;
	if (a_chunk->start > b_chunk->start) return 1;
	return 0;
}

int chunk_list_find(const Chunk_List *list, void *ptr){
	Chunk key = {
		.start = ptr
	};

	Chunk *result = bsearch(&key, list -> chunks, list -> count, sizeof(list -> chunks[0]), chunk_start_compar);

	if(result != 0) {
		assert(list -> chunks <= result);
		return (int) (result - list -> chunks);
	} else return -1;
}



void chunk_list_remove(Chunk_List *list, size_t index){
	assert(index < list -> count);
	for(size_t i = index; i < list -> count - 1; ++ i){
		list -> chunks[i] = list -> chunks[i + 1];
	}
	list -> count -= 1;
}

Chunk_List tmp_chunks = {0};

void *heap_alloc(size_t size){

	if(size > 0){
		chunk_list_merge(&tmp_chunks, &freed_chunks);
		freed_chunks = tmp_chunks;

		for(size_t i = 0; i < freed_chunks.count; ++ i){
			const Chunk chunk = freed_chunks.chunks[i];
			if(chunk.size >= size){
				chunk_list_remove(&freed_chunks, i);


				const size_t tail_size = chunk.size - size;
				chunk_list_insert(&alloced_chunks, chunk.start, size);

				if(tail_size > 0) {
					chunk_list_insert(&freed_chunks, chunk.start + size, tail_size);
				}
				return chunk.start;;
			}
		}
	}
	return NULL;
}

void heap_free(void *ptr){
	if(ptr){
		const int index = chunk_list_find(&alloced_chunks, ptr);
		assert(index >= 0);
		assert(ptr == alloced_chunks.chunks[index].start);
		chunk_list_insert(&freed_chunks, alloced_chunks.chunks[index].start, alloced_chunks.chunks[index].size);
		chunk_list_remove(&alloced_chunks, (size_t) index);
	}
}

void heap_collect(){ 

	UNIMPLEMENTED;
}

# define N 10
void *ptrs[N] = {0};

int main(){

	for(int i = 0; i < N; ++ i){
		ptrs[i] = heap_alloc(i);
	}

	heap_alloc(10);
	heap_alloc(20);
	
	printf("Alloced:\n");
	chunk_list_dump(&alloced_chunks);
	printf("Freed:\n");
	chunk_list_dump(&freed_chunks);
	return 0;
}
