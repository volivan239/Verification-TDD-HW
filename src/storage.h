#pragma once
#include <stdexcept>

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
            if (!storage[i].used) {
                storage[i].key = key;
                storage[i].value = value;
                storage[i].used = true;
                return;
            }
            if (storage[i].key == key) {
                storage[i].value = value;
                return;
            }
            i = (i + 1) % N;
        } while (i != num);
        
        throw StorageOverflowException();
    }

    ValueT load(KeyT key) {
        int num = std::hash<KeyT>{}(key) % N;
        int i = num;
        do {
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
        // TODO
        return false;
    }

    ~Storage() {
        delete[] storage;
    }
};