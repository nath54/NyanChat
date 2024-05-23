#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

const double LOAD = 0.75;

static uint jenkins_one_at_a_time_hash(char* key, size_t length)
{
    uint hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash += key[i];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

static uint hash(char* key, uint size)
    { return jenkins_one_at_a_time_hash(key, strlen(key)) % size; }

static uint jump(uint i)
    { return (i * (i+1)) / 2; }

Hashmap* hashmap_create(uint size)
{
    Hashmap* h = (Hashmap*)malloc(sizeof(Hashmap));
    h->size = 1 << (int)ceil(log2(size));
    h->count = 0;
    h->items = (Item*)calloc(h->size, sizeof(Item));
    return h;
}

void hashmap_free(Hashmap* h)
{
    if (h != NULL)
        free(h->items);
    free(h);
}

void hashmap_resize(Hashmap* h, uint new_size)
{
    uint old_size = h->size;
    Item* old_items = h->items;
    h->size = new_size;
    h->count = 0;
    h->items = (Item*)calloc(h->size, sizeof(Item));

    for (uint i = 0; i < old_size; i++) {
        char* k = old_items[i].key;
        if (k != NULL && strncmp(k, TOMBSTONE, KEY_MAXLEN) != 0)
            { hashmap_insert(h, k, old_items[i].value); }
    }
    free(old_items);
}

int hashmap_find(Hashmap* h, char *key)
{
    uint index = hash(key, h->size);
    uint current_pos = index;
    char* current_key = h->items[index].key;
    uint i = 0;

    while (i < h->size && current_key != NULL) {
        if (strncmp(current_key, key, KEY_MAXLEN) == 0)
            return current_pos;
        current_pos = (index + jump(++i)) % h->size;
        current_key = h->items[current_pos].key;
    }
    return -1; // Key not found
}

uint hashmap_get(Hashmap* h, char* key)
{
    int pos = hashmap_find(h, key);
    if (pos == -1)
        return EMPTY; // Key not found
    return h->items[pos].value;
}

void hashmap_increase(Hashmap* h, char* key, uint incr)
{
    int i = hashmap_find(h, key);
    if (i < 0)
        hashmap_insert(h, key, incr);
    else
        h->items[i].value += incr;
}

int hashmap_insert(Hashmap* h, char* key, uint value)
{
    if (strlen(key) > KEY_MAXLEN)
        return -1;
    uint index = hash(key, h->size);
    uint current_pos = index;
    char* current_key = h->items[index].key;
    uint i = 0;
    int id_tombstone = -1;

    if ((double)h->count / (double)h->size >= LOAD) // the load is too high
        { hashmap_resize(h, 2 * h->size); }

    while (current_key != NULL && strncmp(current_key, key, KEY_MAXLEN) != 0) {
        if (strncmp(current_key, TOMBSTONE, KEY_MAXLEN) == 0 && id_tombstone < 0)
            { id_tombstone = current_pos; }
        current_pos = (index + jump(++i)) % h->size;
        current_key = h->items[current_pos].key;
    }

    if (current_key == EMPTY) { // The key doesn't exist.
        if (id_tombstone >= 0) { // Replace the tombstone.
            h->items[id_tombstone].key = key;
            h->items[id_tombstone].value = value;
        }
        else { // Full the empty case.
            h->items[current_pos].key = key;
            h->items[current_pos].value = value;
        }
        h->count++;
        return 1;
    }
    // Otherwise, the hashmap already contains the key.
    return 0;
}

uint hashmap_remove(Hashmap* h, char* key)
{
    int pos = hashmap_find(h, key);
    if (pos < 0)
        return EMPTY;

    uint value = h->items[pos].value;
    h->items[pos].key = TOMBSTONE;
    h->items[pos].value = 0;
    h->count--;

    if (h->size > 2 && (double)h->count / (double)h->size < 1 - LOAD)
        hashmap_resize(h, h->size / 2);  // the load is too low

    return value;
}

void write_hashmap_to_file(Hashmap *h, FILE *file)
{
    fwrite(&h->size, sizeof(h->size), 1, file);
    fwrite(&h->count, sizeof(h->count), 1, file);
    fwrite(h->items, sizeof(Item), h->size, file);
}

Hashmap *read_hashmap_from_file(FILE *file)
{
    Hashmap *h = (Hashmap*) malloc(sizeof(Hashmap));
    if (!fread(&h->size, sizeof(h->size), 1, file)
        || !fread(&h->count, sizeof(h->count), 1, file))
        goto read_error;
        
    h->items = (Item*)calloc(h->size, sizeof(Item));
    if (!fread(h->items, sizeof(Item), h->size, file)) {
        free(h->items);
        goto read_error;
    }
    return h;

read_error:
    free(h);
    return NULL;
}
