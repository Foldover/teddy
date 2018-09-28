//
// Created by jacob on 2018-09-26.
//

#ifndef TEDDY_MAP_H
#define TEDDY_MAP_H

#include <stdlib.h>
#include <math.h>
#include <string.h>

#define N_TO_CELLS_RATIO (10. / 7.)
#define INITIAL_MAP_SIZE 64
#define INITIAL_MAP_CELL_SIZE ((int)floor(INITIAL_MAP_SIZE * N_TO_CELLS_RATIO))
#define PRIME1 137
#define PRIME2 139

typedef void (*TeddyRoutine)(void);

typedef struct teddyMapItem {
    char *key;
    TeddyRoutine routine;
} TeddyMapItem;

typedef struct teddyMap {
    size_t length;
    size_t capacity;
    TeddyMapItem **items;
} TeddyMap;

void insert(TeddyMap *map, const char *key, TeddyRoutine value);
void delete(TeddyMap *map, const char *key);

static TeddyMapItem DELETED_ITEM = {NULL, NULL};

int isPrime(const int x) {
    if (x < 2) return -1;
    if (x < 4) return 1;
    if((x % 2) == 0) return 0;
    for (int i = 3; i <= floor(sqrt((double) x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

int nextPrime(int x) {
    while (isPrime(x) != 1) {
        x++;
    }
    return x;
}

TeddyMap *createSizedMap(const int baseSize) {
    TeddyMap *map = malloc(sizeof(TeddyMap));
    map->length = 0;
    map->capacity = (size_t) nextPrime(baseSize);
    map->items = malloc(sizeof(TeddyMapItem*) * map->capacity);
    return map;
}

TeddyMap *newMap() {
    return createSizedMap(INITIAL_MAP_CELL_SIZE);
}

void destroyMap(TeddyMap *map) {
    for(int i = 0; i < map->capacity; i++) {
        TeddyMapItem *item = map->items[i];
        if (item != NULL) {
            free(item);
        }
    }
    free(map->items);
    free(map);
}

TeddyMapItem* createMapItem(const char *key, TeddyRoutine value) {
    TeddyMapItem *item = malloc(sizeof(TeddyMapItem));
    item->key = strdup(key);
    item->routine = value;
    return item;
}

void destroyMapItem(TeddyMapItem* item) {
    free(item->key);
    free(item);
}

uint hash(const char* key, const int prime, const int bucketsCount) {
    long hash = 0;
    const size_t len_s = strlen(key);
    for (int i = 0; i < len_s; i++) {
        hash += (long)pow(prime, len_s - (i+1)) * key[i];
        hash = hash % bucketsCount;
    }
    return (uint)hash;
}

uint getHash(
        const char* key, const int bucketsCount, const int attempt
) {
    const int firstHash = hash(key, PRIME1, bucketsCount);
    const int secondHash = hash(key, PRIME2, bucketsCount);
    return (uint) ((firstHash + (attempt * (secondHash + 1))) % bucketsCount);
}

uint nToCells(uint n) {
    return (uint)floor(n * N_TO_CELLS_RATIO);
}

void resize(TeddyMap *map, const int baseSize) {
    if (baseSize < INITIAL_MAP_CELL_SIZE) {
        return;
    }
    TeddyMap *newMap = createSizedMap(baseSize);
    for (int i = 0; i < map->capacity; i++) {
        TeddyMapItem *item = map->items[i];
        if (item != NULL && item != &DELETED_ITEM) {
            insert(newMap, item->key, item->routine);
        }
    }

    map->length = newMap->length;

    const int tempCapacity = map->capacity;
    map->capacity = newMap->capacity;
    newMap->capacity = tempCapacity;

    TeddyMapItem **items = map->items;
    map->items = newMap->items;
    newMap->items = items;
    destroyMap(newMap);
}

static void resizeUp(TeddyMap *map) {
    const int newCapacity = map->capacity * 2;
    resize(map, newCapacity);
}


static void resizeDown(TeddyMap* map) {
    const int newCapacity = map->capacity / 2;
    resize(map, newCapacity);
}

void insert(TeddyMap *map, const char *key, TeddyRoutine value) {
    const int load = map->length * 100 / map->capacity;
    if (load > 70) {
        resizeUp(map);
    }
    TeddyMapItem *item = createMapItem(key, value);
    int index = getHash(item->key, map->capacity, 0);
    TeddyMapItem *currentItem = map->items[index];
    int attempt = 1;
    while(currentItem != NULL) {
        if (currentItem != &DELETED_ITEM) {
            if (strcmp(currentItem->key, key) == 0) {
                destroyMapItem(currentItem);
                map->items[index] = item;
                return;
            }
        }
        index = getHash(item->key, map->capacity, attempt);
        currentItem = map->items[index];
        attempt++;
    }
    map->items[index] = item;
    map->length++;
}

TeddyRoutine search(TeddyMap *map, const char *key) {
    int index = getHash(key, map->capacity, 0);
    TeddyMapItem *item = map->items[index];
    int attempt = 1;
    while(item != NULL) {
        if (item != &DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                return item->routine;
            }
        }
        index = getHash(key, map->capacity, attempt);
        item = map->items[index];
        attempt++;
    }
    return NULL;
}

void delete(TeddyMap *map, const char *key) {
    const int load = map->length * 100 / map->capacity;
    if (load < 10) {
        resizeDown(map);
    }
    int index = getHash(key, map->capacity, 0);
    TeddyMapItem *item = map->items[index];
    int attempt = 1;
    while(item != NULL) {
        if (item != &DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                destroyMapItem(item);
                map->items[index] = &DELETED_ITEM;
            }
        }
        index = getHash(key, map->capacity, attempt);
        item = map->items[index];
        attempt++;
    }
    map->length--;
}

#endif //TEDDY_MAP_H
