#include <assert.h>
#include "../include/channel_map.h"

int map_make_space(struct channel_map *map, int slot, int size) {
    if (map->number <= slot) {
        int number = map->number ? map->number : 32;

        void **tmp;

        while (number <= slot) 
            number <<= 1;

        tmp = (void**)realloc(map->entries, number *size);

        if (!tmp)
            return -1;

        memset(&tmp[map->number], 0, (number - map->number) * size);

        map->number = number;
        map->entries = tmp;
    }
    return 0;
}

void map_init(struct channel_map *map) {
    map->entries = NULL;
    map->number = 0;
}

void map_clear(struct channel_map *map) {
    if (map->entries != NULL) {
        for(int i = 0; i < map->number; ++i) {
            if (map->entries[i]) 
                free(map->entries[i]);
        }
        free(map->entries);
        map->entries = NULL;
    }
    map->number = 0;
}