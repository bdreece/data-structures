/*! \file map.c
 *  \brief Map data structure implementation
 *  \author Brian Reece
 *  \version 0.1.0
 */

#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "vla.h"

static unsigned long map_binary_search(map_t * map, void *key, unsigned long low, unsigned long high)
{
	unsigned long mid;
	unsigned long cmp;
	pair_t pair;

	if (low > high)
		return MAP_FAILURE;

	mid = (low + high) / 2;
	if (vla_get(&map->vla, mid, (void *)&pair) < 0)
		return MAP_FAILURE;

	cmp = memcmp(key, pair.key, map->key_size);
	
	if (cmp == 0)
		return mid;
	else if (cmp < 0)
		return map_binary_search(map, key, low, mid - 1);
	else
		return map_binary_search(map, key, mid + 1, high);

	return MAP_FAILURE;
}

static unsigned long map_linear_search(map_t *map, void *key)
{
	unsigned long i;
	pair_t pair;

	for(i = 0; i < map->vla.size; i++)
	{
		if(vla_get(&map->vla, i, (void *)&pair) < 0)
			return MAP_FAILURE;
		if(memcmp(key, pair.key, map->key_size) == 0)
			return i;
	}

	return MAP_FAILURE;
}

static pair_t *map_create_pair(map_t *map, void *key, void *val)
{
	if (!map || !key || !val) return NULL;
	pair_t *p = (struct pair *) malloc(sizeof(struct pair));
	p->key = malloc(map->key_size);
	p->val = malloc(map->val_size);

	return p;
}

static int map_destroy_pair(pair_t *p)
{
	if (!p || !p->key || !p->val)
		return MAP_NULL;

	free(p->key);
	free(p->val);
	free(p);
	return MAP_SUCCESS;
}

int map_init(map_t *map, bool sorted, size_t key_size, size_t val_size, unsigned long initial_capacity)
{
	if (!map)
		return MAP_NULL;

	if (key_size < 1 || val_size < 1 || initial_capacity < 1)
		return MAP_EMPTY;

	if (vla_init(&map->vla, sizeof(pair_t), initial_capacity) == VLA_FAILURE)
		return MAP_FAILURE;

	map->sorted = sorted;
	map->key_size = key_size;
	map->val_size = val_size;

	return MAP_SUCCESS;
}

int map_deinit(map_t *map)
{
	if (!map)
		return MAP_NULL;

	if (vla_deinit(&map->vla) == VLA_FAILURE)
		return MAP_FAILURE;

	map->key_size = map->val_size = 0;

	return MAP_SUCCESS;
}

int map_get(map_t *map, void *key, void *val)
{
	if (!map || !map->vla.elements)
		return MAP_NULL;

	if (map->vla.size < 1)
		return MAP_EMPTY;

	unsigned long i = map->sorted ? map_binary_search(map, key, 0, map->vla.size - 1) : map_linear_search(map, key);
	pair_t p;

	if (i < 0)
		return MAP_UNKNOWN_KEY;

	if (vla_get(&map->vla, i, (void *)&p) < 0)
		return MAP_FAILURE;

	memcpy(val, p.val, map->val_size);

	return MAP_SUCCESS;

}

int map_set(map_t *map, void *key, void *val)
{
	if (!map || !map->vla.elements)
		return MAP_NULL;

	if (map->vla.size < 1)
		return MAP_EMPTY;

	unsigned long i = map->sorted ? map_binary_search(map, key, 0, map->vla.size - 1) : map_linear_search(map, key);

	pair_t *p = map_create_pair(map, key, val);

	if (i < 0)
	{
		if (vla_enq(&map->vla, (void *)p) < 0)
			return MAP_FAILURE;
	}

	if (vla_set(&map->vla, i, (void *)p) < 0)
			return MAP_FAILURE;

	return MAP_SUCCESS;	
}

int map_delete(map_t *map, void *key)
{
	if (!map || !map->vla.elements)
		return MAP_NULL;

	if (map->vla.size < 1)
		return MAP_EMPTY;

	unsigned long i = map->sorted ? map_binary_search(map, key, 0, map->vla.size - 1) : map_linear_search(map, key);

	if (i < 0)
		return MAP_UNKNOWN_KEY;

	pair_t *p;
	if (vla_get(&map->vla, i, (void *)&p) < 0)
		return MAP_FAILURE;

	if (map_destroy_pair(p) < 0)
		return MAP_FAILURE;

	if (vla_delete(&map->vla, i) < 0)
		return MAP_FAILURE;

	return MAP_SUCCESS;
}

int map_clear(map_t *map)
{
	if (!map || !map->vla.elements)
		return MAP_FAILURE;

	if (vla_clear(&map->vla) < 0)
		return MAP_FAILURE;

	return MAP_SUCCESS;
}
