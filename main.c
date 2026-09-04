#include <stddef.h>
#include <stdio.h>

#define CAPACITY 640000 // size of the heap: 640 Kilo Bytes. initially in C programs 1 Mega Byte is assigned and grows accordingly

char heap[CAPACITY] = {0}; // THE "heap" in this "code environment""

// Only two main functions needed for memory allocation
void *heap_alloc(size_t size){ // Allocates a memory in the heap
	return NULL;
}

void heap_free(void *ptr){ // Frees up the memory in the heap

}
