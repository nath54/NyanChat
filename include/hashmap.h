#pragma once

#include <stdint.h>

typedef unsigned int uint;

// Default value given to each Item in the Hashmap.
#define EMPTY 0
// Value given to each removed Item of the Hashmap.
#define TOMBSTONE "$"
#define KEY_MAXLEN 256

/**
 * @brief Standard structure for an Item of a Hashmap.
 */
typedef struct Item {
    /*@{*/
    char* key; /**< Value hashed to find the index of the Item. */
    uint value; /**< Stored value. */
    /*@}*/
} Item;

/**
 * @brief Standard structure for a Hashmap.
 */
typedef struct Hashmap {
    Item* items;
    uint size;
    uint count; // Number of filled indexes in Hashmap::items.
} Hashmap;

/**
 * @brief Create a Hashmap with an initial \p size.
 * 
 * @param size Initial size of the Hashmap (Hashmap::size).
 * @return `Hashmap*`: A pointer to the created Hashmap.
 */
Hashmap* hashmap_create(uint size);

/**
 * @brief Free the given Hashmap \p h.
 * 
 * @param h The Hashmap to free.
 */
void hashmap_free(Hashmap* h);

/**
 * @brief Resize the given Hashmap \p h by \p new_size.
 * 
 * @param h The Hashmap to resize.
 * @param new_size New size for the Hashmap.
 */
void hashmap_resize(Hashmap* h, uint new_size);

/**
 * @brief Find the index of a given \p key in the Hashmap \p h.
 * 
 * @param h The given Hashmap.
 * @param key The Item::key to find.
 * @return `int`: -1 if the \p key doesn't exist, its index otherwise.
 */
int hashmap_find(Hashmap* h, char* key);

/**
 * @brief Find the value associated to the given \p key in the Hashmap \p h.
 * 
 * @param h  The given Hashmap.
 * @param key The Item::key we are looking for.
 * @return `uint`: The Item::value associated to the Item::key.
 */
uint hashmap_get(Hashmap* h, char* key);

/**
 * @brief Insert a new element in the Hashmap \p h, if it doesn't already exist.
 * 
 * @param h The given Hashmap.
 * @param key The Item::key to add.
 * @param value The associated Item::value.
 * @return `int`: 1 if the \p key was added, 0 if it already exists or
 * -1 if the key has invalid length.
 */
int hashmap_insert(Hashmap* h, char* key, uint value);

/**
 * @brief Increase the value of \p key in the Hashmap `h` by \p incr.
 * @note If the \p key doesn't exist, use @ref hashmap_insert to create it.
 * 
 * @param h The given Hashmap.
 * @param key The Item::key to find.
 * @param incr The desired increase of the tem::value.
 */
void hashmap_increase(Hashmap* h, char* key, uint incr);

/**
 * @brief Remove the item with the corresponding \p key from the hashmap \p h.
 * 
 * @param h The given Hashmap.
 * @param key The Item::key to remove.
 * @return `uint`: The Item::value corresponding to the removed Item::key.
 */
uint hashmap_remove(Hashmap* h, char* key);

/**
 * @brief Write the given hashmap \p h to the file \p file.
 * 
 * @param h The given Hashmap.
 * @param file The file where writte the Hashmap.
*/
void write_hashmap_to_file(Hashmap *h, FILE *file);

/**
 * @brief Read a Hashmap from the file \p file
 * 
 * @param file The file to extract the Hashmap.
*/
Hashmap* read_hashmap_from_file(FILE *file);