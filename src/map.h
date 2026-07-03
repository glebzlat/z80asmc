#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Map Map;
typedef struct MapIter MapIter;

/** Destructor function type for the type stored in a Map
 *
 * A destructor is responsible only for cleaning the contents of an object,
 * not freeing object's memory itself, as it is owned by a Map.
 */
typedef void (*Map_value_destructor_fn)(void*);

struct MapIter {
  Map* m;
  char const* key;
  void* value;
  size_t idx;
};

typedef enum {
  MAP_OK,
  MAP_NO_SUCH_KEY,
  MAP_ERROR,
} MapStatus;

/** Create a new Map object
 *
 * @param value_size Size of the value type
 * @param dtor Function used to free up the memory used to store the value
 * @returns Map instance
 */
Map* Map_new(size_t value_size, Map_value_destructor_fn dtor);

/** Destroy a Map object
 *
 * Calls the destructor function for each entry in the map.
 *
 * Note on pointer invalidation: when the map is destroyed, all pointers
 * acquired from `Map_set`, `Map_get`,  become invalid.
 *
 * @param m Map object
 */
void Map_destroy(Map* m);

/** Set a map entry
 *
 * Stores the value by its copy and returns a pointer to it.
 *
 * If the entry with the same key already exists, copies the new value over
 * the old one in place. All pointers to the buffer acquired before value
 * replacement stay valid.
 *
 * Modifies Map object state:
 *
 * - On successful operation sets `MAP_OK`
 * - On memory or other internal errors sets `MAP_ERROR`
 *
 * @param m Map object
 * @param key Entry key
 * @param value Entry value
 * @returns Pointer to the copy of the value
 */
void* Map_set(Map* m, char const* key, void* value);

/** Get a map entry by its key
 *
 * Tries to retrieve the value of an entry with the supplied key. If such
 * entry does not exist, returns `NULL` pointer and sets Map object state
 * to `MAP_NO_SUCH_KEY`. Sets `MAP_OK` upon successful retrieval.
 *
 * @param m Map object
 * @param key Entry key
 * @returns Pointer to the value
 */
void* Map_get(Map* m, char const* key);

/** Delete map entry referenced by the key
 *
 * Deletes an entry with the supplied key if it is present; calls the
 * destructor for stored value. If an entry does not exist, sets Map
 * state to `MAP_NO_SUCH_KEY`, in case of successful deletion sets `MAP_OK`,
 * and in case of other errors, related to STD lib calls or internal logic,
 * sets `MAP_ERROR`.
 *
 * Invalidates pointers to the memory buffer owned by an entry.
 *
 * Returns 0 on success and non-zero value otherwise.
 *
 * @param m Map object
 * @param key Entry key
 * @returns Flag signaling whether the operation was successful.
 */
int Map_del(Map* m, char const* key);

/** Get the length of the map
 *
 * A length of the map is effectively the amount of entries in it.
 *
 * @param m Map object
 * @returns Length
 */
size_t Map_len(Map const* m);

/** Get the status of the map
 *
 * @param m Map object
 * @returns Object status
 */
MapStatus Map_getStatus(Map* m);

/** Create map iterator object
 *
 * @param m Map object
 * @returns Map iterator
 */
MapIter MapIter_init(Map* m);

/** Advance map iterator to the next entry
 *
 * Returns true if an entry exist and false otherwise. If `next` returned
 * false, then `MapIter_getValue` and `MapIter_getKey` return `NULL` pointer.
 *
 * @param it Map iterator
 * @returns Whether the iterator points to an entry
 */
bool MapIter_next(MapIter* it);

/** Get the value of the current entry pointed by iterator
 *
 * @param it Map iterator
 * @returns Pointer to the value
 */
void* MapIter_getValue(MapIter* it);

/** Get the key of the current entry pointed by iterator
 *
 * @param it Map iterator
 * @returns Key string
 */
char const* MapIter_getKey(MapIter* it);

#endif // MAP_H
