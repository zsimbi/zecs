#ifndef ZSTACK_H
#define ZSTACK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct zstack {
    size_t size;
    size_t type_size;
    size_t head;
    void* memory;
} zstack;

zstack* zstack_create(size_t size, size_t type_size);
int zstack_full(zstack* stack);
int zstack_empty(zstack* stack);
int zstack_push(zstack* stack, void* value);
void* zstack_pop(zstack* stack);
void* zstack_peak(zstack* stack);
void zstack_destroy(zstack* stack);

#endif

#ifdef ZSTACK_IMPL

zstack* zstack_create(size_t size, size_t type_size) {
    zstack* stack = (zstack*)malloc(sizeof(zstack));
    if(stack == NULL) {
	fprintf(stderr, "Failed to create stack! ERROR: malloc failed, buy more ram:)");
	exit(-1);
    }
    
    stack->size = size;
    stack->type_size = type_size;
    stack->head = 0;
    
    stack->memory = (void*)malloc(size * type_size);
    if(stack->memory == NULL) {
	fprintf(stderr, "Failed to allocate memory during stack creaton! ERROR: malloc failed, buy more ram:)");
	exit(-1);
    }
    
    return stack;
}

int zstack_full(zstack* stack) {
    return stack->head == stack->size;
}

int zstack_empty(zstack* stack) {
    return stack->head == 0;
}

int zstack_push(zstack* stack, void* value) {
    if(zstack_full(stack)) {
	fprintf(stderr, "Failed to push value to stack! ERROR: stack is full.");
	return 1;
    }
    
    memcpy(stack->memory + stack->head * stack->type_size, value, stack->type_size);
    stack->head += 1;
    return 0;
}

void* zstack_pop(zstack* stack) {
    if(zstack_empty(stack)) {
	fprintf(stderr, "Failed to pop value from stack! ERROR: stack is empty.");
	return NULL;
    }

    stack->head -= 1;
    return stack->memory + (stack->head * stack->type_size);
}

void* zstack_peak(zstack* stack) {
    if(zstack_empty(stack)) {
	fprintf(stderr, "Failed to peak into stack! ERROR: stack is empty.");
	return NULL;
    }
    
    return stack->memory + (stack->head - 1) * stack->type_size;
}

//Error handling
void zstack_destroy(zstack* stack) {
    free(stack->memory);
    free(stack);
}

#endif
