

#ifndef SONGLIST_H
#define SONGLIST_H

#include <stdlib.h>
#include <string.h>
#include "song.h"

typedef struct _node_t
{
    song_t *data;
    struct _node_t *prev;
    struct _node_t *next;
} song_node_t;

void initList();

song_node_t *getListHead();
song_node_t *getListTail();

song_node_t *pushtoFront(song_t *song);
song_node_t *pushtoBack(song_t *song);

song_node_t *insertBefore(song_node_t *node, song_t *song);
song_node_t *insertAfter(song_node_t *node, song_t *song);

void delfromBack();
void delfromFront();
void deletenode(song_node_t *node);

song_node_t *getNextSong(song_node_t *node);
song_node_t *getPrevSong(song_node_t *node);

uint32_t getListLenght();

#endif
