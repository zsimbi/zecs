#ifndef ZECS_H
#define ZECS_H

#include "./zstack/zstack.h"
#include "./ztable/ztable.h"

typedef unsigned zecs_entity;
typedef unsigned zecs_component_key;

typedef struct zecs {
    size_t max_entity_count;
    size_t entity_count;
    zstack* entity_pool;

    size_t max_component_count;
    size_t component_count;
    ztable* component_stores;

    ztable component_masks;

    size_t search_count;
    zecs_entity* search_result;
} zecs;

zecs zecs_create(size_t max_entity_count, size_t max_component_count);
int zecs_component_register(zecs* ecs, size_t component_size);
zecs_entity* zecs_component_get_entities(zecs* ecs, zecs_component_key component_key);
size_t zecs_component_get_entity_count(zecs* ecs, zecs_component_key component_key);
zecs_entity zecs_entity_create(zecs* ecs);
int zecs_entity_add_data(zecs* ecs, zecs_entity entity, zecs_component_key component_key, void* component_data);
int zecs_entity_remove_component(zecs* ecs, zecs_entity entity, zecs_component_key component_key);
void* zecs_entity_get_data(zecs* ecs, zecs_entity entity, zecs_component_key component_key);
int zecs_entity_has_component(zecs* ecs, zecs_entity entity, zecs_component_key component_key);
zecs_entity* zecs_search_entities(zecs* ecs, zecs_component_key* component_keys, size_t component_key_count);
int zecs_entity_destroy(zecs* ecs, zecs_entity entity);
void zecs_destroy(zecs* ecs);

#endif

#ifdef ZECS_IMPL

#include <stdio.h>
#include <inttypes.h>

/*Creates the ecs*/
zecs zecs_create(size_t max_entity_count, size_t max_component_count) {
    zecs ecs;

    ecs.max_entity_count = max_entity_count;
    ecs.entity_count = 0;
    ecs.entity_pool = zstack_create(max_entity_count, sizeof(zecs_entity));

    zecs_entity val;
    for(size_t i = max_entity_count; i > 0; i--) {
	val = i - 1;
	zstack_push(ecs.entity_pool, &val);
    }
    
    ecs.max_component_count = max_component_count;
    ecs.component_count = 0;
    ecs.component_stores = (ztable*)malloc(0);
    if(ecs.component_stores == NULL) {
	fprintf(stderr, "Failed to create ecs. Not enough memory.\n");
    }
    
    ecs.component_masks = ztable_create(sizeof(uint32_t));

    ecs.search_count = 0;
    ecs.search_result = (zecs_entity*)malloc(0);
    if(ecs.search_result == NULL) {
	fprintf(stderr, "Failed to create ecs. Not enough memory.\n");
    }
    
    return ecs;
}

/*Registers the component for usage in the ecs*/
int zecs_component_register(zecs* ecs, size_t component_size) {
    if(ecs->component_count >= 32) {
	fprintf(stderr, "Cant register component. Component cap reached, the max number of components is 32.\n");
	return -1;
    }

    ecs->component_count += 1;
    ecs->component_stores = reallocarray(ecs->component_stores, ecs->component_count, sizeof(ztable));

    ecs->component_stores[ecs->component_count - 1] = ztable_create(component_size);

    return 0;
}

/*Returns an entity from the entity pool. The entity is basically just an zecs_component_key id.*/
zecs_entity zecs_entity_create(zecs* ecs) {
    if(ecs->entity_count == ecs->max_entity_count) {
	return ecs->max_entity_count + 1;
    }
    zecs_entity entity = *((zecs_entity*)(zstack_pop(ecs->entity_pool)));
    uint32_t empty_mask = 0;
    
    ecs->entity_count += 1;
    
    ztable_add(&(ecs->component_masks), entity, &empty_mask);
    
    return entity;
}

size_t zecs_component_get_entity_count(zecs* ecs, zecs_component_key component_key) {
    return ecs->component_stores[component_key].count;
}

/*Adds a component with data to an entity*/
int zecs_entity_add_data(zecs* ecs, zecs_entity entity, zecs_component_key component_key, void* component_data) {
    ztable* component_store = &(ecs->component_stores[component_key]);
    ztable_add(component_store, entity, component_data);
    
    uint32_t old_mask = *((uint32_t*)(ztable_get_value(ecs->component_masks, entity)));
    uint32_t new_mask = old_mask | (1 << component_key);

    ztable_modify(&(ecs->component_masks), entity, &new_mask);
    
    return 0;
}

/*Removes a component from the entity*/
int zecs_entity_remove_component(zecs* ecs, zecs_entity entity, zecs_component_key component_key) {
    ztable* component_store = &(ecs->component_stores[component_key]);
    ztable_remove(component_store, entity);

    uint32_t old_mask = *((uint32_t*)(ztable_get_value(ecs->component_masks, entity)));
    uint32_t new_mask = old_mask & ~(1 << component_key);

    ztable_modify(&(ecs->component_masks), entity, &new_mask);

    return 0;
}

/*Returns the entities that all have a specific component.*/
zecs_entity* zecs_component_get_entities(zecs* ecs, zecs_component_key component_key) {
    return ecs->component_stores[component_key].keys;
}

/*Returns a pointer to the entity's data.*/
void* zecs_entity_get_data(zecs* ecs, zecs_entity entity, zecs_component_key component_key) {
    ztable component_store = ecs->component_stores[component_key];

    return ztable_get_value(component_store, entity);
}
/*
void zecs_entity_modify_data(zecs* ecs, zecs_entity entity, zecs_component_key component_key, void* data) {
    ztable_modify(&(ecs->component_stores[component_key]), entity, data);
}
*/
int zecs_entity_has_component(zecs* ecs, zecs_entity entity, zecs_component_key component_key) {
    return *(uint32_t*)ztable_get_value(ecs->component_masks, entity) & (1 << component_key);
}

/*
int zecs_entity_get_component_count(zecs* ecs, zecs_entity entity) {
    
}
*/

/*Returns the entities that have all the components specified in component_keys*/
zecs_entity* zecs_search_entities(zecs* ecs, zecs_component_key* component_keys, size_t component_key_count) {
    //Get the component that has the lowest number of entities
    size_t min = 0;
    size_t tmp = 0;
    size_t min_key = 0;
    for(size_t i = 0; i < component_key_count; ++i) {
	tmp = zecs_component_get_entity_count(ecs, component_keys[i]);
	if(tmp == 0) {
	    fprintf(stderr, "No entity has component with id %d\n", component_keys[i]);
	    return NULL;
	}
	if(tmp < min) {
	    min = tmp;
	    min_key = component_keys[i];
	}
    }
    
    zecs_entity* es = zecs_component_get_entities(ecs, min_key);
    size_t min_count = zecs_component_get_entity_count(ecs, min_key);
    int should_include = 1;

    //Reset search_count and search_result
    ecs->search_count = 0;
    ecs->search_result = reallocarray(ecs->search_result, min_count, sizeof(zecs_entity));

    //Look for the entities that have all the specified components
    for(size_t i = 0; i < min_count; ++i) {
	should_include = 1;
	for(size_t j = 0; (j < component_key_count) && should_include; ++j) {
	    if(component_keys[j] == min_key) {
		continue;
	    }
	    if(!zecs_entity_has_component(ecs, es[i], component_keys[j])) {
		should_include = 0;
	    }
	}
	if(should_include) {
	    ecs->search_result[ecs->search_count] = es[i];
	    ecs->search_count += 1;
	}
    }
    return ecs->search_result;
}

/*Delets entity. Returns the entity to the entity pool and delets all the data associated with the entity.*/
int zecs_entity_destroy(zecs* ecs, zecs_entity entity) {
    zecs_entity e = entity;
    zstack_push(ecs->entity_pool, &e);

    ztable_remove(&(ecs->component_masks), entity);

    /*!!TODO: Change component count to the number of components the entity has!!*/
    ztable* component_store;
    for(size_t i = 0; i < ecs->component_count; ++i) {
	component_store = &(ecs->component_stores[i]);
	ztable_remove(component_store, entity);
    }
    
    return 0;
}

/*Frees the allocated memory*/
void zecs_destroy(zecs* ecs) {
    zstack_destroy(ecs->entity_pool);

    for(size_t i = 0; i < ecs->component_count; ++i) {
	ztable_destroy(ecs->component_stores[i]);
    }
    free(ecs->component_stores);

    ztable_destroy(ecs->component_masks);

    free(ecs->search_result);
    
    return;
}

#endif
