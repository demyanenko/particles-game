#pragma once

#include "util.cpp"
#include <stdint.h>

template <typename TKey, typename TValue>
struct HashMap
{
    struct Entry
    {
        uint64_t hash;
        TKey key;
        TValue value;
    };

    int size;
    int capacity;
    Entry *entries;
};

template <typename TKey>
uint64_t _hash(TKey key)
{
    uint64_t z = (uint64_t)(key) + 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    z = z ^ (z >> 31);
    return z | 0x8000000000000000ULL; // Final value is never zero
}

template <typename TKey, typename TValue>
void hashMapInit(HashMap<TKey, TValue> *outHashMap)
{
    outHashMap->size = 0;
    outHashMap->capacity = 4;
    outHashMap->entries = (typename HashMap<TKey, TValue>::Entry *)malloc(
        outHashMap->capacity * sizeof(typename HashMap<TKey, TValue>::Entry));
    assertOrAbort(outHashMap->entries != NULL, "Could not allocate memory for hash map");
    for (int i = 0; i < outHashMap->capacity; i++)
    {
        outHashMap->entries[i].hash = 0;
    }
}

template <typename TKey, typename TValue>
void hashMapFree(HashMap<TKey, TValue> *hashMap)
{
    hashMap->size = 0;
    hashMap->capacity = 0;
    free(hashMap->entries);
    hashMap->entries = NULL;
}

template <typename TKey, typename TValue>
TValue hashMapGet(HashMap<TKey, TValue> *hashMap, TKey key)
{
    uint64_t hash = _hash(key);
    int bucket = hash % hashMap->capacity;
    while (hashMap->entries[bucket].hash != 0)
    {
        if (hashMap->entries[bucket].key == key)
        {
            return hashMap->entries[bucket].value;
        }
        bucket = (bucket + 1) % hashMap->capacity;
    }

    abortWithMessage("Value not found in hash map");
}

template <typename TKey, typename TValue>
bool hashMapContains(HashMap<TKey, TValue> *hashMap, TKey key)
{
    uint64_t hash = _hash(key);
    int bucket = hash % hashMap->capacity;
    while (hashMap->entries[bucket].hash != 0)
    {
        if (hashMap->entries[bucket].key == key)
        {
            return true;
        }
        bucket = (bucket + 1) % hashMap->capacity;
    }

    return false;
}

template <typename TKey, typename TValue>
void hashMapInsert(HashMap<TKey, TValue> *hashMap, TKey key, TValue value)
{
    if (hashMap->size > 0.7 * hashMap->capacity)
    {
        int oldCapacity = hashMap->capacity;
        typename HashMap<TKey, TValue>::Entry *oldEntries = hashMap->entries;

        hashMap->capacity *= 2;
        hashMap->entries = (typename HashMap<TKey, TValue>::Entry *)malloc(
            hashMap->capacity * sizeof(typename HashMap<TKey, TValue>::Entry));
        assertOrAbort(hashMap->entries != NULL, "Could not allocate more memory for hash map");

        for (int i = 0; i < hashMap->capacity; i++)
        {
            hashMap->entries[i].hash = 0;
        }

        for (int i = 0; i < oldCapacity; i++)
        {
            if (oldEntries[i].hash != 0)
            {
                uint64_t hash = _hash(oldEntries[i].key);
                int bucket = hash % hashMap->capacity;
                while (hashMap->entries[bucket].hash != 0)
                {
                    bucket = (bucket + 1) % hashMap->capacity;
                }
                hashMap->entries[bucket].hash = _hash(oldEntries[i].key);
                hashMap->entries[bucket].key = oldEntries[i].key;
                hashMap->entries[bucket].value = oldEntries[i].value;
            }
        }

        free(oldEntries);
    }

    uint64_t hash = _hash(key);
    int bucket = hash % hashMap->capacity;

    while (hashMap->entries[bucket].hash != 0)
    {
        bucket = (bucket + 1) % hashMap->capacity;
    }

    hashMap->entries[bucket].hash = hash;
    hashMap->entries[bucket].key = key;
    hashMap->entries[bucket].value = value;
    hashMap->size++;
}
