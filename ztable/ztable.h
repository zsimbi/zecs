#ifndef ZTABLE_H
#define ZTABLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct ztable {
    size_t type_size;
    size_t count;
    unsigned* keys;
    void* values;
} ztable;

ztable ztable_create(size_t type_size);
int ztable_add(ztable* table, unsigned key, void* value);
int ztable_remove(ztable* table, unsigned key);
int ztable_modify(ztable* table, unsigned key, void* value);
void* ztable_get_value(ztable table, unsigned key);
void ztable_destroy(ztable table);

#endif
#ifdef ZTABLE_IMPL

ztable ztable_create(size_t type_size) {
    ztable table;
    table.type_size = type_size;
    table.count = 0;
    
    table.values = (void*)malloc(0);
    if(table.values == NULL) {
	fprintf(stderr, "Memory allocation failed:(\n");
	exit(-1);
    }

    table.keys = (unsigned*)malloc(0);
    if(table.values == NULL) {
	fprintf(stderr, "Memory allocation failed:(\n");
	exit(-1);
    }
    
    return table;
}

int ztable_add(ztable* table, unsigned key, void* value) {
    for(size_t i = 0; i < table->count; ++i) {
	if(table->keys[i] == key) {
	    fprintf(stderr, "Key %d already taken.\n", key);
	    return -1;
	}
    }
    
    table->count += 1;

    table->keys = reallocarray(table->keys, table->count, sizeof(unsigned));
    table->values = reallocarray(table->values, table->count, table->type_size);

    if(table->values == NULL || table->keys == NULL) {
	fprintf(stderr, "failed realloc\n");
	return -1;
    }
    
    table->keys[table->count - 1] = key;
    memcpy(table->values + (table->count - 1) * table->type_size, value, table->type_size);
    
    return 0;
}

int ztable_remove(ztable* table, unsigned key) {
    if(table->count == 0) {
	fprintf(stderr, "Cant remove from table. Table is empty\n");
	return -1;
    }

    for(size_t i = 0; i < table->count; ++i) {
	if(table->keys[i] == key) {
	    table->keys[i] = table->keys[table->count - 1];
	    memcpy(table->values + i * table->type_size, table->values + (table->count - 1) * table->type_size, table->type_size);
	    
	    table->count -= 1;
	    table->keys = reallocarray(table->keys, table->count, sizeof(unsigned));
	    table->values = reallocarray(table->values, table->count, table->type_size);
	    
	    return 0;
	}
    }

    fprintf(stderr, "Key not found in table\n");
    return -1;
}

int ztable_modify(ztable* table, unsigned key, void* value) {
    for(size_t i = 0; i < table->count; ++i) {
	if(table->keys[i] == key) {
	    memcpy(table->values + (i * table->type_size), value, table->type_size);
	    return 0;
	}
    }

    fprintf(stderr, "Key not found in table\n");
    return -1;
}

void* ztable_get_value(ztable table, unsigned key) {
    for(size_t i = 0; i < table.count; ++i) {
	if(table.keys[i] == key) {
	    return table.values + (i * table.type_size);
	}
    }
    fprintf(stderr, "Key %d not found in table\n", key);
    return NULL;
}

void ztable_destroy(ztable table) {
    free(table.keys);
    free(table.values);
}

#endif
