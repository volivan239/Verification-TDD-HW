#include <gtest/gtest.h>
#include "storage.h"

TEST(SingleThreadStorageTest, SimpleLoadStoreTest) {
  Storage<std::string, int> storage(1000, 10);

  storage.store("hello", 0);
  storage.store("world", 1);

  EXPECT_EQ(0, storage.load("hello"));
  EXPECT_EQ(1, storage.load("world"));
}

TEST(SingleThreadStorageTest, OverwriteTest) {
  Storage<std::string, int> storage(1000, 10);

  storage.store("hello", 2);
  storage.store("hello", 3);
  storage.store("hello", 9);

  EXPECT_EQ(9, storage.load("hello"));
}

TEST(SingleThreadStorageTest, NoSuchElementTest) {
  Storage<std::string, int> storage(1000, 10);

  storage.store("hello", 2);

  EXPECT_THROW(storage.load("world"), NoSuchElementException);
}

TEST(SingleThreadStorageTest, StorageOverflowTest) {
  Storage<std::string, int> storage(2, 2);

  storage.store("hello", 2);
  storage.store("world", 3);

  EXPECT_THROW(storage.store("!", 4), StorageOverflowException);
  EXPECT_THROW(storage.load("!"), NoSuchElementException);
}

TEST(SingleThreadStorageTest, DefaultValueKeyTest) {
  Storage<std::string, int> storage(2, 2);

  EXPECT_THROW(storage.load(""), NoSuchElementException);
}
