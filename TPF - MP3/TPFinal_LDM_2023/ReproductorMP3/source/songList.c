/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "songList.h"

static song_node_t head = {.data = NULL, .next = NULL, .prev = NULL};
static song_node_t tail = {.data = NULL, .next = NULL, .prev = NULL};
static uint32_t listLenght = 0;

void initList(){
	tail.prev = &head;
	head.next = &tail;
}

song_node_t *getListHead()
{
    return &head;
}
song_node_t *getListTail()
{
    return &tail;
}

song_node_t *pushtoFront(song_t *song)
{
    song_node_t *aux = head.next;
    song_node_t *new = malloc(sizeof(song_node_t));

    new->data = song;
    new->prev = &head;
    new->next = aux;

    head.next = new;
    aux->prev = new;

    listLenght++;

    return new;
}
song_node_t *pushtoBack(song_t *song)
{
    song_node_t *aux = tail.prev;
    song_node_t *new = malloc(sizeof(song_node_t));

    new->data = song;
    new->next = &tail;
    new->prev = aux;

    tail.prev = new;
    aux->next = new;

    listLenght++;

    return new;
}

song_node_t *insertBefore(song_node_t *node, song_t *song)
{
    song_node_t *new = malloc(sizeof(song_node_t));

    new->data = song;

    new->next = node;
    new->prev = node->prev;
    node->prev->next = new;
    node->prev = new;

    listLenght++;

    return new;
}
song_node_t *insertAfter(song_node_t *node, song_t *song)
{
    song_node_t *new = malloc(sizeof(song_node_t));

    new->data = song;

    new->next = node->next;
    new->prev = node;
    node->next->prev = new;
    node->next = new;

    listLenght++;

    return new;
}

void delfromBack()
{
    song_node_t *aux = tail.prev;

    aux->prev->next = &tail;
    tail.prev = aux->prev;

    destroySong(aux->data);
    free(aux);
}
void delfromFront()
{
    song_node_t *aux = head.next;

    aux->next->prev = &head;
    head.next = aux->next;

    destroySong(aux->data);
    free(aux);
}
void deletenode(song_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;

    destroySong(node->data);
    free(node);
}

song_node_t *getNextSong(song_node_t *node)
{
    return node->next;
}
song_node_t *getPrevSong(song_node_t *node)
{
    return node->prev;
}

uint32_t getListLenght()
{
    return listLenght;
}
