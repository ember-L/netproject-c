#ifndef __CHANNEL_MAP_H
#define __CHANNEL_MAP_H

#include "channel.h"
#include "require.h"

/*将fd做为键, channel为值*/
struct channel_map {
    void **entries;

    /* The number of entries available in entries */
    int number;
};

int map_make_space(struct channel_map *, int , int);
void map_init(struct channel_map *);
void map_clear(struct channel_map *);

#endif