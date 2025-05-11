#pragma once

template <typename KeyT, typename ValueT>
class Storage {
public:
    Storage(int N, int K) {}

    void store(KeyT key, ValueT value) {
        // TODO
    }

    ValueT load(KeyT key) {
        // TODO
        return 0;
    }

    bool updateIfEquals(KeyT key, ValueT oldValue, ValueT newValue) {
        // TODO
        return false;
    }
};