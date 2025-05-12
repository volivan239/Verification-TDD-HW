#pragma once
#include <gtest/gtest.h>
#include "storage.h"

template <typename KeyT, typename ValueT>
struct LoadWithCheckTask {
    Storage<KeyT, ValueT> *storage;
    KeyT key;
    ValueT expectedValue;
};

template <typename KeyT, typename ValueT>
void *loadWithCheck(void *data) {
    LoadWithCheckTask<KeyT, ValueT> *task = (LoadWithCheckTask<KeyT, ValueT> *) (data);
    EXPECT_EQ(task->storage->load(task->key), task->expectedValue);
    return nullptr;
}


template <typename KeyT, typename ValueT>
struct StoreNoExceptTask {
    Storage<KeyT, ValueT> *storage;
    KeyT key;
    ValueT value;
};

template <typename KeyT, typename ValueT>
void *storeNoExcept(void *data) {
    StoreNoExceptTask<KeyT, ValueT> *task = (StoreNoExceptTask<KeyT, ValueT> *) (data);
    EXPECT_NO_THROW(task->storage->store(task->key, task->value));
    return nullptr;
}