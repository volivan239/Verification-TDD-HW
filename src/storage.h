#pragma once
#include <stdexcept>
#include <shared_mutex>

class NoSuchElementException : public std::runtime_error {
public:
    NoSuchElementException(): runtime_error("No such element") {}
};

class StorageOverflowException : public std::runtime_error {
public:
    StorageOverflowException(): runtime_error("No space left in storage") {}
};


template <typename KeyT, typename ValueT>
class Storage {
private:
    struct StorageCell {
        KeyT key;
        ValueT value;
        bool used = false;

        std::shared_mutex m;
    };

    int N, K;
    StorageCell *storage;
    StorageCell *cache;

private:
    void storeToMainStorage(KeyT key, ValueT value) {
        int num = std::hash<KeyT>{}(key) % N;
        int i = num;
        do {
            std::shared_lock lock(storage[i].m);
            if (!storage[i].used || storage[i].key == key) { // Check that i-th cell is suitable
                lock.unlock();
                std::unique_lock ulock(storage[i].m);
                if (!storage[i].used || storage[i].key == key) { // Re-check after acquiring unique lock
                    storage[i].key = key;
                    storage[i].value = value;
                    storage[i].used = true;
                    return;
                }
            }
            i = (i + 1) % N;
        } while (i != num);
        
        throw StorageOverflowException();
    }

    bool loadFromMainStorage(KeyT key, ValueT *value) {
        int num = std::hash<KeyT>{}(key) % N;
        int i = num;
        do {
            std::shared_lock lock(storage[i].m);
            if (!storage[i].used) {
                return false;
            }
            if (storage[i].key == key) {
                *value = storage[i].value;
                return true;
            }
            i = (i + 1) % N;
        } while (i != num);
        
        return false;
    }

public:
    Storage(int N, int K): N(N), K(K) {
        storage = new StorageCell[N];
        cache = new StorageCell[K];
    }

    void store(KeyT key, ValueT value) {
        int i = std::hash<KeyT>{}(key) % K;
        std::unique_lock lock(cache[i].m);
        if (cache[i].used && cache[i].key != key) {
            // Flush old key-value pair to main storage
            storeToMainStorage(cache[i].key, cache[i].value);
        }

        // Updating cache with new key-value pair
        cache[i].used = true;
        cache[i].key = key;
        cache[i].value = value;
    }

    ValueT load(KeyT key) {
        int i = std::hash<KeyT>{}(key) % K;
        std::shared_lock lock(cache[i].m);
        if (cache[i].used && cache[i].key == key) {
            // Easy win, load from cache
            return cache[i].value;
        }
        lock.unlock();
        std::unique_lock ulock(cache[i].m);
        if (cache[i].used) {
            if (cache[i].key == key) {
                // Someone loaded our key to cache, easy win now
                return cache[i].value; 
            }
            // Flush old key-value pair to main storage 
            storeToMainStorage(cache[i].key, cache[i].value);
        }

        ValueT value;
        if (!loadFromMainStorage(key, &value)) {
            throw NoSuchElementException();
        }

        // Saving key-value pair to cache
        cache[i].used = 1;
        cache[i].key = key;
        cache[i].value = value;
        return value;
    }

    bool updateIfEquals(KeyT key, ValueT oldValue, ValueT newValue) {
        int i = std::hash<KeyT>{}(key) % K;
        std::unique_lock lock(cache[i].m);
        if (cache[i].used && cache[i].key == key) {
            // Easy win
            if (cache[i].value == oldValue) {
                // Just update cache
                cache[i].value = newValue;
                return true; 
            }
            return false;
        }

        ValueT realOldValue;
        if (!loadFromMainStorage(key, &realOldValue)) {
            // No such element
            return false;
        }

        if (cache[i].used) {
            // Flush cached key-value pair to memory
            storeToMainStorage(cache[i].key, cache[i].value);
        }

        cache[i].used = true;
        cache[i].key = key;
        cache[i].value = (realOldValue == oldValue) ? newValue : realOldValue;
        return realOldValue == oldValue;
    }

    ~Storage() {
        delete[] storage;
        delete[] cache;
    }
};