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

public:
    Storage(int N, int K): N(N), K(K) {
        storage = new StorageCell[N];
    }

    void store(KeyT key, ValueT value) {
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

    ValueT load(KeyT key) {
        int num = std::hash<KeyT>{}(key) % N;
        int i = num;
        do {
            std::shared_lock lock(storage[i].m);
            if (!storage[i].used) {
                throw NoSuchElementException();
            }
            if (storage[i].key == key) {
                return storage[i].value;
            }
            i = (i + 1) % N;
        } while (i != num);
        
        throw NoSuchElementException();
    }

    bool updateIfEquals(KeyT key, ValueT oldValue, ValueT newValue) {
        int num = std::hash<KeyT>{}(key) % N;
        int i = num;
        do {
            std::shared_lock lock(storage[i].m);
            if (!storage[i].used) {
                return false; // key not found
            }
            if (storage[i].key == key) { // Check that i-th cell is suitable
                if (storage[i].value != oldValue) {
                    return false; // value differs from oldValue
                }
                lock.unlock();
                std::unique_lock ulock(storage[i].m);
                if (storage[i].value == oldValue) { // Re-check after acquiring unique lock
                    storage[i].value = newValue;
                    return true;
                } else {
                    return false; // value differs from oldValue
                }
            }
            i = (i + 1) % N;
        } while (i != num);
        return false;
    }

    ~Storage() {
        delete[] storage;
    }
};