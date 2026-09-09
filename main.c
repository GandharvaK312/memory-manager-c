#include "./heap.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define JIM_IMPLEMENTATION
#include "./jim2.h"

typedef struct Node Node;

struct Node{ // binary tree node 
	char x; Node *left, *right;
};

Node *generate_tree(size_t level_curr, size_t level_max){
	if(level_curr < level_max){
		Node* root = heap_alloc(sizeof(*root)); // Made the root using the custom made allocator not even malloc, How cool is that!!!!
		assert((char) level_curr + 'a' <= 'z');
		root -> x = level_curr + 'a';
		root -> left = generate_tree(level_curr + 1, level_max);
		root -> right = generate_tree(level_curr + 1, level_max);
		return root;
	} else return NULL;
}

void print_tree(Node *root, Jim *jim){
	if(root){
		jim_object_begin(jim);
		jim_member_key(jim, "value");
		jim_string_sized(jim, &root -> x, 1);

		jim_member_key(jim, "left");
		print_tree(root -> left, jim);

		jim_member_key(jim, "right");
		print_tree(root -> right, jim);
		
		jim_object_end(jim);
	} else {
		jim_null(jim);
	}
}

# define N 10

void *ptrs[N] = {0};

int main(){


	stack_base = (const uintptr_t*) __builtin_frame_address(0);
	Node *root = generate_tree(0, 2);

	for(size_t i = 0; i < 3; ++ i){
		heap_alloc(i);
	}
	Jim jim = {
		.sink = stdout,
		.write = (Jim_Write) fwrite,
	};

	printf("\n-----------------------------------------\n");
	printf("representation of tree in json format:\n");
	print_tree(root, &jim);
	printf("\n-----------------------------------------\n");
	
	heap_collect(); // reachable
	printf("collected everything except for the tree because root != NULL:\n\n");
	chunk_list_dump(&alloced_chunks, "Alloced");
	chunk_list_dump(&freed_chunks, "Freed");
	
	root = NULL;
	heap_collect(); // not reachable
	printf("\n-----------------------------------------\n");
	printf("collected everything after root was set to NULL\n\n");
	chunk_list_dump(&alloced_chunks, "Alloced");
	chunk_list_dump(&freed_chunks, "Freed");
	
	return 0;
}
