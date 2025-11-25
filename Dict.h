#ifndef DICT_H
#define DICT_H

#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Represents a dictionary.
 */
typedef struct Dict_t Dict;

/**
 * @brief Creates a new dictionary.
 *
 * @param m size of the hash table.
 *
 * @return The created dictionary.
 */
Dict *dictCreate(size_t m);

/**
 * @brief Frees a dictionary, including all keys but not the values.
 *
 * @param d Dictionary to free.
 */
void dictFree(Dict *d);

/**
 * @brief Frees a dictionary, including all keys and values, the latter
 *        using the freeData function in argument.
 *
 * @param d Dictionary to free
 * @param freeData a function to be called on all values
 */
void dictFreeValues(Dict *d, void (*freeData)(void *));

/**
 * @brief Apply a function to every (key, value) pairs in the dictionary.
 *
 * @param d Dictionary
 * @param f The function to apply
 */
void dictIterate(Dict *d, void (*f)(const char *key, void *values));

/**
 * @brief Search for a given key in a dictionary and return the associated value.
 *
 * @param d Dictionary to search in.
 * @param key Key to search for.
 *
 * @returns the value if the key was found, or exit the program if the key
 * was not found. dictContains must be use before to avoid exiting.
 */
void *dictSearch(Dict *d, const char *key);

/**
 * @brief Search for a given key in a dictionary and return 1 if the key is found,
 *        0 otherwise.
 *
 * @param d Dictionary to search in.
 * @param key Key to search for.
 *
 * @returns bool if the key was found, false otherwise.
 */
bool dictContains(Dict *d, const char *key);

/**
 * @brief Insert a new key-value pair in a dictionary. The key is copied at insertion.
 *
 * @param d Dictionary to insert in.
 * @param key Key to insert.
 * @param value Value to insert.
 */
void dictInsert(Dict *d, const char *key, void *value);

/**
 * @brief Return the number of distinct keys in the dictionary.
 *
 * @param d Dictionary
 * @return size_t The number of keys
 */
size_t dictGetNbKeys(Dict *d);

#endif
