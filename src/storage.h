#pragma once

template <typename KeyT, typename ValueT>
class Storage {
private:
    struct StorageCell {
        KeyT key;
        ValueT value;
        bool used;
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
            if (storage[i].key == key) {
                storage[i].value = value;
                return;
            }
            if (!storage[i].used) {
                storage[i].key = key;
                storage[i].value = value;
                storage[i].used = true;
                return;
            }
            i = (i + 1) % N;
        } while (i != num);
        assert(false);
    }

    ValueT load(KeyT key) {
        int num = std::hash<KeyT>{}(key) % N;
        int i = num;
        do {
            if (storage[i].key == key) {
                return storage[i].value;
            }
            if (!storage[i].used) {
                assert(false);
            }
            i = (i + 1) % N;
        } while (i != num);
        assert(false);
    }

    bool updateIfEquals(KeyT key, ValueT oldValue, ValueT newValue) {
        // TODO
        return false;
    }

    ~Storage() {
        delete[] storage;
    }
};